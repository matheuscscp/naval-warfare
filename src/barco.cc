#include <iostream>
#include <cmath>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

enum barcotype {
	big,
	medium,
	small,
	last
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

		float cx, cy;
		float targetx, targety;
		bool selected;
	        bool gameplay; //flag para o barco saber se está na etapa de movimentação
        	bool gamesetup; //flag para o barco saber se está na etapa de setup

		gear2d::link<float> x, y, w, h;
		gear2d::link<int> mouse1;

		
		struct atributos
		{
			gear2d::link<int>	tipo;		//tipo do barco
			gear2d::link<int>	hp;			//vida
			gear2d::link<float>	range;		//range de ataque
			gear2d::link<float>	moverange;	//range de movimento
			gear2d::link<float>	speed;		//velocidade de movimento
			gear2d::link<int>	dmg;		//dano por segundo
			gear2d::link<int>	loot;		//loot dropado
		}atr;

		component::base * alvoPrincipal;
		component::base * porto;

	public:
		barco() { }
		virtual ~barco() { }

		virtual gear2d::component::family family() { return "unit"; }

		virtual gear2d::component::type type() { return "barco"; }

		virtual std::string depends() {
			return "kinematics/kinematic2d mouse/mouse mouseover/mouseover collider/collider2d";
		}

		virtual void setup(object::signature & sig) {
			init<int>	("tipo"      , sig["tipo"]      , big);
			init<int>	("hp.value"  , sig["hp.value"]  , 100);
			init<float>	("range"     , sig["range"]     , 64.0f);
			init<float>	("moverange" , sig["moverange"] , 128.0f);
			init<float>	("speed"     , sig["speed"]     , 300.0f);
			init<int>	("dmg"       , sig["dmg"]       , 10);
			init<int>	("loot.value", sig["loot.value"], 100);

			atr.tipo 		= fetch<int>	("tipo");
			atr.hp 			= fetch<int>	("hp.value");
			atr.range		= fetch<float>	("range");
			atr.moverange 	= fetch<float>	("moverange");
			atr.speed		= fetch<float>	("speed");
			atr.dmg			= fetch<int>	("dmg");
			atr.loot		= fetch<int>	("loot.value");

			x 		= fetch<float>("x");
			y 		= fetch<float>("y");
			w 		= fetch<float>("w");
			h 		= fetch<float>("h");

			mouse1 	= fetch<int>  ("mouse.1");

			write<component::base *>("porto", NULL);
			porto = NULL;
			
			//alterando range de ataque do barco
			//NOTA: isso vai aumentar a caixa de colisao dos barcos, fazendo com que
			//as colisoes ja comecem a ser testadas a partir de colisoes entre os 
			//ranges de ataque
			write("collider.aabb.x",x+(w/2)-atr.range);
			write("collider.aabb.y",y+(h/2)-atr.range);
			write("collider.aabb.w",atr.range*2);
			write("collider.aabb.h",atr.range*2);
			
			write("target.render", false);
			write("rangeatk.render", false);
			write("barcohover.render", false);
			write("range.render", false);
			
			//iniciando todos os valores do range de movimento e de ataque
			float rangeWH = (float)read<int>("range.position.w")/2; 
			float rangeatkWH = (float)read<int>("rangeatk.position.w")/2;
			
			write("range.position.x",(w/2.0f)-rangeWH);
			write("range.position.y",(h/2.0f)-rangeWH);
			write("range.zoom", atr.moverange/rangeWH);
			
			write("rangeatk.position.x",(w/2.0f)-rangeatkWH);
			write("rangeatk.position.y",(h/2.0f)-rangeatkWH);
			write("rangeatk.zoom", atr.range/rangeatkWH);
			
			/**HOOKS**/
			hook("porto");
			hook("mouse.1", (component::call)&barco::handleClick);
			hook("collider.collision",(component::call)&barco::handleCollision);
			hook("hp.value", (component::call)&barco::handleLife);
			hook("mouseover", (component::call)&barco::handleMouseover);
			
			cx = x + w/2;
			cy = y + h/2;

			targetx = cx;
			targety = cy;

			selected = false;
			gameplay = true; //incializando como true para não prejudicar os demais testes
			gamesetup = true;
		}

		virtual void update(timediff dt) 
		{	
			cx = x + w/2;
			cy = y + h/2;

        	//verifica se é o turno do barco antes de executar a movimentação
        	//!selected usado pra não prejudicar testes
        	if( gameplay && !selected )
        	{
            	write("x.speed", targetx - cx);
            	write("y.speed", targety - cy);
        	}

	        if (selected)
	        {
				int mousex = read<int>("mouse.x");
				int mousey = read<int>("mouse.y");

				float dx = mousex - cx;
				float dy = mousey - cy;

				float distance = sqrt(dx*dx + dy*dy);

				if(distance > atr.range) {
					dx = dx * (atr.range/distance);
					dy = dy * (atr.range/distance);
				}
				
				targetx = cx + dx;
				targety = cy + dy;

				write("target.position.x", targetx - 8);
				write("target.position.y", targety - 8);
			}
			
			//clip da barra de hp proporcional ao hp
			write("hpbar.clip.w", (atr.hp*64)/100.0);
		}

		virtual void handle(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (pid == "porto") {
				porto = read<component::base *>("porto");
				hook(porto, "gamesetup");
				hook(porto, "gameplay");
			}
		}

		//Cuida das colisoes gerais
		virtual void handleCollision(parameterbase::id pid, base* lastwrite, object::id owner) {
			bool longe, perto = false;
			float inimX,inimY,inimW = 0.0f;
			
			if ((pid == "collider.collision")&&(atr.hp!=0))
			{//arrumar essa colisao de ultimo segundo com loot
				
				component::base * inimigo = read<component::base*>(pid);
				
				inimX = inimigo->read<float>("x");
				inimY = inimigo->read<float>("y");
				inimW = inimigo->read<float>("w");
				 
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
						removeHP(inimigo,1);
					}	
				}
				
				//colisao de perto (barco vs barco inimigo, barco vs loot)
				if(perto)
				{
					if(inimigo->read<string>("collider.tag") == "barco")
					{
						int danoFisico = inimigo->read<int>("hp.value");
						removeHP(inimigo,atr.hp);
						atr.hp = atr.hp-danoFisico; 
					}
					else
					{
						if(inimigo->read<string>("collider.tag") == "loot")
						{
							porto->write("cash.value",porto->read<int>("cash.value") + inimigo->read<int>("cash"));
							inimigo->destroy();
						}
					}
				}
			}
		}

		//Tira vida de um component, pelo amor de deus, use isso em algo que tem hp.value
		//retorna true se o alvo morrer(vida <=0)
		bool removeHP(component::base * inimigo,int dmg) {
			int newHp = 0;
			if(inimigo->read<int>("hp.value")>0){
				newHp = inimigo->read<int>("hp.value")-dmg;
				inimigo->write("hp.value",newHp);
				return false;
			}
			return true;
		}
		
		//funcao calcula colisao esferica entre objetos a e b (ar = raio de a)
		//nota: mande o centro x e centro y de a e b
		bool sphereCollision(float acx,float acy,float ar,float bcx, float bcy,float br) {
			float  dX   = acx - bcx;
			float  dY   = acy - bcy;
			float  dR   = br + ar;
			if((dX*dX)+(dY*dY) <= dR*dR)
				return true;
			return false;
		}

		virtual void handleClick(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (mouse1 == 1) {
				if (selected) {
					selected = false;
					write("range.render", false);
					write("rangeatk.render", false);
					//write("target.render", false);
				}
				else {
					if (read<bool>("mouseover")) {
						cout << "clicked over me" << endl;
						selected = true;
						write("range.render", true);
						write("target.render", true);
						write("rangeatk.render", true);
					}
				}
			}
		}
		
		virtual void handleMouseover(parameterbase::id pid, base* lastwrite, object::id owner) {
			write("barcohover.render", read<bool>("mouseover"));
		}
		
		//Na morte do barco, dah spawn num loot aonde o barco morreu
		virtual void handleLife(parameterbase::id pid, component::base * last, object::id owns) {
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
