#include <iostream>
#include <cmath>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

enum barcotype {
	big = 0,
	medium,
	small,
	lastsize
};

namespace gear2d {
	template<> barcotype eval<barcotype>(std::string t, barcotype def) {
		if (t == "big") return big;
		if (t == "medium") return medium;
		if (t == "small") return small;
		return def;
	}
}

class barco : public component::base {
	private:

		float xspeedbuf, yspeedbuf;
		float cx, cy;
		float targetx, targety;
		bool selected;
		bool gameplay; //flag para o barco saber se está na etapa de movimentação
		bool gamesetup; //flag para o barco saber se está na etapa de setup
		bool done;


		gear2d::link<float> x, y, w, h;
		gear2d::link<int> mouse1;

		gear2d::link<bool> paused;
		
		struct atributos
		{
			gear2d::link<int>	tipo;		//tipo do barco
			gear2d::link<int>	hp;			//vida
			int					hp_max;			//vida maxima
			gear2d::link<float>	range;		//range de ataque
			gear2d::link<float>	moverange;	//range de movimento
			gear2d::link<float>	speed;		//velocidade de movimento
			gear2d::link<int>	dmg;		//dano por segundo
			gear2d::link<int>	loot;		//loot dropado
			gear2d::link<int>	attackTimer;		//loot dropado
			timediff			lifetime;	// tempo de vida
		}atr;

		component::base * alvoPrincipal;
		component::base * porto;
		
		float barcohover_y0;

	public:	
		barco() { }
		virtual ~barco() { 
        }
        
        virtual void destroyed() {
			unhook(porto, "gameplay");
			unhook(porto, "gamesetup");
		}

		virtual gear2d::component::family family() { return "unit"; }

		virtual gear2d::component::type type() { return "barco"; }

		virtual std::string depends() {
			return "renderer/renderer spatial/space2d kinematics/kinematic2d mouse/mouse mouseover/mouseover collider/collider2d pause/paused";
		}

		virtual void setup(object::signature & sig) {
          		modinfo("barco");

		//	spawn("barcoCollisionBox");
			
			/* o done indica que terminamos */
			write<component::base*>("done", 0);
			
			init<int>	("tipo"      , sig["tipo"]      , big);
			init<int>	("hp.value"  , sig["hp.value"]  , 100);
			init<float>	("range"     , sig["range"]     , 64.0f);
			init<float>	("moverange" , sig["moverange"] , 128.0f);
			init<float>	("speed"     , sig["speed"]     , 300.0f);
			init<int>	("dmg"       , sig["dmg"]       , 10);
			init<int>	("loot.value", sig["loot.value"], 100);

			atr.tipo 		= fetch<int>	("tipo");
			atr.hp 			= fetch<int>	("hp.value");
			atr.hp_max		= atr.hp;
			atr.range		= fetch<float>	("range");
			atr.moverange 		= fetch<float>	("moverange");
			atr.speed		= fetch<float>	("speed");
			atr.dmg			= fetch<int>	("dmg");
			atr.loot		= fetch<int>	("loot.value");

			x 		= fetch<float>("x");
			y 		= fetch<float>("y");
			w 		= fetch<float>("w");
			h 		= fetch<float>("h");

			mouse1 	= fetch<int>  ("mouse.1");

			write<component::base *>("porto", NULL);
			porto 			= NULL;
			alvoPrincipal 	= NULL;
			
			paused = fetch<bool>("paused");
			done = fetch<bool>("done");
			
			//alterando range de ataque do barco
			//NOTA: isso vai aumentar a caixa de colisao dos barcos, fazendo com que
			//as colisoes ja comecem a ser testadas a partir de colisoes entre os 
			//ranges de ataque
			write("collider.aabb.x",x+(w/2)-atr.range);
			write("collider.aabb.y",y+(h/2)-atr.range);
			write("collider.aabb.w",atr.range*2);
			write("collider.aabb.h",atr.range*2);
			
			//iniciando todos os valores do range de movimento e de ataque
			float rangeWH = (float)read<int>("range.position.w")/2.0f; 
			float rangeatkWH = (float)read<int>("rangeatk.position.w")/2.0f;
			
			write("range.position.x",(w/2.0f) - rangeWH);
			write("range.position.y", (h/2.0f) - rangeWH);
            trace("rpx",
                  read<float>("range.position.x"),
                  read<int>("range.position.w"),
                  "rpy",
                  read<float>("range.position.y"),
                  read<int>("range.position.h"));
 			write("range.zoom", atr.moverange/rangeWH);
            
            trace("rpx",
                  read<float>("range.position.x"),
                  read<int>("range.position.w"),
                  "rpy",
                  read<float>("range.position.y"),
                  read<int>("range.position.h"));
            
			write("rangeatk.position.x",(w/2.0f) - rangeatkWH);
			write("rangeatk.position.y",(h/2.0f) - rangeatkWH);
			write("rangeatk.zoom", atr.range/rangeatkWH);
			
			
			//iniciando os valores do texto de atributos
			stringstream dmg,spd,hp;
			
			dmg<<"DMG: "<<atr.dmg<<" ";
			spd<<"SPD: "<<(int)atr.speed<<" ";
			
			write("atributoDano.text",dmg.str());
			write("atributoSpeed.text",spd.str());
			
			init<float>("barcosprite.position.z", sig["barcosprite.position.z"], 0.0f);
			
			barcohover_y0 = eval<float>(sig["barcohover.position.y"]);
			
			/**HOOKS**/
			hook("porto");
			hook("mouse.1", (component::call)&barco::handleClick);
			hook("collider.collision",(component::call)&barco::handleCollision);
			hook("hp.value", (component::call)&barco::handleLife);
			hook("mouseover", (component::call)&barco::handleMouseover);
		//	hook("barcoCollisionBox.collider.collision", (component::call)&barco::handleCollisionBox);
			
			selected = false;
		}

		virtual void update(timediff dt) {

			if (paused)
			{
				write<float>("x.speed", 0);
				write<float>("y.speed", 0);
				return;
			}
			modinfo("nw-barco");

			cx = x + w/2;
			cy = y + h/2;
			
			trace("Gameplay:", gameplay, "selected:", selected, "done",  read<component::base*>("done"));

			//verifica se é o turno do barco antes de executar a movimentação
			//selected usado pra não prejudicar testes
			if(gameplay)
			{				
				float xs = targetx - cx;
				float ys = targety - cy;
				write("x.speed", xs);
				write("y.speed", ys);
				bool done = false;
				
				if ((xs > -0.3 && xs < 0.3) && (ys > -0.3 && ys < 0.3))
				{
					write("x.speed", 0.0);
					write("y.speed", 0.0);
					done = true;
				}

				if (done && (read<component::base*>("done") == NULL)) {
// 					trace("Barco done");
					write("done", this); 
				}

				
			}

			//posiciona o target quando o barco esta selecionado
			if (selected)
			{
				int mousex = read<int>("mouse.x");
				int mousey = read<int>("mouse.y");

				float dx = mousex - cx;
				float dy = mousey - cy;

				float distance = sqrt(dx*dx + dy*dy);

				if(distance > atr.moverange) {
					dx = dx * (atr.moverange/distance);
					dy = dy * (atr.moverange/distance);
				}
				
				targetx = cx + dx;
				targety = cy + dy;

				write("target.position.x", targetx - 8);
				write("target.position.y", targety - 8);
			}
			
			if (!gameplay) {
				write("x.speed", 0);
				write("y.speed", 0);
			}
					
			//clip da barra de hp proporcional ao hp
			write("hpbar.clip.w", (atr.hp*read<int>("hpbarback.clip.w"))/atr.hp_max);
			
			// troca de direcao se a velocidade em x nao for nula
			float xspeed_direcao = read<float>("x.speed");
			if (xspeed_direcao < 0) {
				write<bool>("barco.render", false);
				write<bool>("barcoinv.render", true);
			}
			else if (xspeed_direcao > 0) {
				write<bool>("barco.render", true);
				write<bool>("barcoinv.render", false);
			}
			
			// animacao do mouseover
			atr.lifetime += dt;
			write<float>("barcohover.position.y", barcohover_y0 - 10*pow(cos(atr.lifetime), 2));
		}

		virtual void handle(parameterbase::id pid, base* lastwrite, object::id owner) {
			modinfo("nw-barco");
			if (paused) return;
			if (pid == "porto") {
				porto = read<component::base *>("porto");
				hook(porto, "gamesetup");
				hook(porto, "gameplay");
				gameplay = porto->read<bool>("gameplay");
				gamesetup = porto->read<bool>("gamesetup");
				
				write<float>("barco.position.z", read<float>("barcosprite.position.z"));
				write<float>("barcoinv.position.z", read<float>("barcosprite.position.z"));
				
				write<float>("w", read<int>("barco.clip.w"));
				write<float>("h", read<int>("barco.clip.h"));
				
				cx = x + w/2;
				cy = y + h/2;

				targetx = cx;
				targety = cy;
			}
			
			else if (pid == "gamesetup") {
				gamesetup = porto->read<bool>("gamesetup");
				if (!gamesetup) {
					write<component::base*>("done", NULL);
					write<bool>("target.render", false);
					write("barcohover.render", false);
				}
				else {
					write<bool>("target.render", selected);
					write("barcohover.render", read<bool>("mouseover"));
				}
			}
			
			else if (pid == "gameplay") {
				trace("Gameplay being set");
				gameplay = porto->read<bool>("gameplay");
			}
		}

		//Cuida das colisoes gerais
		virtual void handleCollision(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (paused) return;
			bool longe, perto = false;
			float inimX,inimY,inimW = 0.0f;
			
			if ((pid == "collider.collision")&&(atr.hp!=0))
			{//arrumar essa colisao de ultimo segundo com loot
				
				component::base * inimigo = read<component::base*>(pid);
				
				inimX = inimigo->read<float>("x");
				inimY = inimigo->read<float>("y");
				inimW = inimigo->read<float>("w");
				
				if((inimigo->owner == porto->owner)||(inimigo->read<component::base*>("porto") == porto ))
				{
					///TODO: PARAR  BARCO SE ELE COLIDIR COM UM BARCO ALIADO
					return;
				}
				
				//checando as colisoes esfericas dentro da bounding box do collider(que eh uma caixa, duh)
				//com o objeto "inimigo"
				longe = sphereCollision(x+(w/2.0f),y+(h/2.0f),atr.range,
										inimX+(inimW/2.0f),inimY+(inimW/2.0f),inimW/2.0f);
				
				perto = sphereCollision(x+(w/2.0f),y+(h/2.0f),w/2,
										inimX+(inimW/2.0f),inimY+(inimW/2.0f),inimW/2.0f);
				
				//colisao de longe (range do barco vs. barco inimigo, range do barco vs, porto)
				if(longe)
				{
					if((inimigo->read<string>("collider.tag") == "barco")||(inimigo->read<string>("collider.tag") == "porto"))
					{
						//em teoria isso deveria estar funcionando: guarda o primeiro inimigo a entrar no range de ataque,
						//somente esse primeiro inimigo vai ser atacado. Quando ele morrer, o proximo alvo vai ser atacado.
						if(alvoPrincipal==NULL)
							alvoPrincipal = inimigo;
						else
						{
							//if((alvoPrincipal==inimigo)&&(recarregando==false))
							if(alvoPrincipal==inimigo)
							{
								if(removeHP(inimigo,atr.dmg))
									alvoPrincipal=NULL;
								//recarregando=true;
							}
						}
					}	
				}
				
				//colisao de perto (barco vs barco inimigo, barco vs loot)
				if(perto)
				{
					if(inimigo->read<string>("collider.tag") == "barco")
					{
						//se colidirem, os dois se causam dano.
						int danoFisico = inimigo->read<int>("hp.value");
						removeHP(inimigo,atr.hp);
						atr.hp = atr.hp-danoFisico; 
					}
					else
					{
						if(inimigo->read<string>("collider.tag") == "loot")
						{
							porto->add("cash.value", inimigo->read<int>("cash"));
							porto->add("cashganho", inimigo->read<int>("cash"));
							inimigo->destroy();
						}
					}
				}
			}
			else alvoPrincipal=NULL;
		}

		virtual void handleCollisionBox(parameterbase::id pid, base* lastwrite, object::id owner) {
		}

		//Tira vida de um component, pelo amor de deus, use isso em algo que tem hp.value
		//retorna true se o alvo morrer(vida <=0)
		bool removeHP(component::base * inimigo,int dmg) {
			if (paused) return false;
			int newHp = 0;
			if(inimigo->read<int>("hp.value")>0){
				newHp = inimigo->read<int>("hp.value")-dmg;
				inimigo->write("hp.value",newHp);
//				cout<<"ATIREI"<<endl;
				return false;
			}
			return true;
		}
		
		//calcula colisao esferica entre objetos a e b (ar = raio de a)
		//nota: mande o centro x e centro y de a e b
		bool sphereCollision(float acx,float acy,float ar,float bcx, float bcy,float br) {
			if (paused) return false;
			float  dX   = acx - bcx;
			float  dY   = acy - bcy;
			float  dR   = br + ar;
			if((dX*dX)+(dY*dY) <= dR*dR)
				return true;
			return false;
		}

		virtual void handleClick(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (paused || !gamesetup || gameplay) return;
			if (mouse1 == 1) {
				if (selected) {
					selected = false;
					write("range.render"			, false);
					write("rangeatk.render"			, false);
					//write("target.render"			, false);
					write("atributoDano.render"		, false);
					write("atributoSpeed.render"	, false);
					write("atributoHP.render"		, false);
				}
				else {
					if (read<bool>("mouseover")) {
						cout << "clicked over me" << endl;
						selected = true;
						write("range.render"			, true);
						write("target.render"			, true);
						write("rangeatk.render"			, true);
						write("atributoDano.render"		, true);
						write("atributoSpeed.render"	, true);
						write("atributoHP.render"		, true);
						write<bool>("barcohover.render", false);
					}
				}
			}
		}
		
		virtual void handleMouseover(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (paused) return;
			if (gamesetup) {
				if (!selected)
					write("barcohover.render", read<bool>("mouseover"));
				else
					write("barcohover.render", false);
			}
		}
		
		//Na morte do barco, dah spawn num loot aonde o barco morreu
		virtual void handleLife(parameterbase::id pid, component::base * last, object::id owns) {
			if (paused) return;
			if (pid == "hp.value"){
				if(read<int>("hp.value") <= 0) {
					porto->write<component::base*>("barcomorrendo", this);
					
					component::base* loot;
					loot = spawn("loot")->component("spatial");
					loot->write("cash",read<int>("loot.value"));
					loot->write("x",read<float>("x"));
					loot->write("y",read<float>("y"));
					
					destroy();
				}
			}
		}
};

// the build function
extern "C" { component::base * build() { return new barco(); } }
