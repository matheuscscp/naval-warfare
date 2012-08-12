#include <iostream>
#include <cmath>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

enum barcotype {
	big,
	medium,
	small,
	none
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
			gear2d::link<string> tipo;		//tipo do barco
			gear2d::link<int>    hp;		//vida
			gear2d::link<float>  range;		//range de ataque
			gear2d::link<float>  moverange;		//range de movimento
			gear2d::link<float>  speed;		//velocidade de movimento
			gear2d::link<int>    dmg;		//dano por segundo
			gear2d::link<int>    loot;		//loot dropado
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
			barcotype t = eval(sig["barcotype"], small);
			init<int>	("hp.value"  , sig["hp.value"]  , 100);
			init<float>	("range"     , sig["range"]     , 64.0f);
			init<float>	("moverange" , sig["moverange"] , 128.0f);
			init<float>	("speed"     , sig["speed"]     , 300.0f);
			init<int>	("dmg"       , sig["dmg"]       , 10);
			init<int>	("loot.value", sig["loot.value"], 100);

			atr.hp 		= fetch<int>  ("hp.value");
			atr.range	= fetch<float>("range");
			atr.moverange 	= fetch<float>("moverange");
			atr.speed	= fetch<float>("speed");
			atr.dmg		= fetch<int>  ("dmg");
			atr.loot	= fetch<int>  ("loot.value");

			x 		= fetch<float>("x");
			y 		= fetch<float>("y");
			w 		= fetch<float>("w");
			h 		= fetch<float>("h");

			mouse1 		= fetch<int>  ("mouse.1");

			write<component::base *>("porto", NULL);
			porto = NULL;
			
			//alterando range de ataque do barco
			write("collider.aabb.x",x - (atr.range-w)/2);
			write("collider.aabb.y",y - (atr.range-h)/2);
			write("collider.aabb.w",atr.range*2);
			write("collider.aabb.h",atr.range*2);

			hook("porto");
			hook("mouse.1", (component::call)&barco::handleClick);
			hook("collider.collision",(component::call)&barco::handleCollision);//hookando a colisao
			hook("hp.value", (component::call)&barco::handleLife);//hookando a life
			hook("hp.value", (component::call)&barco::updateHpText);
			hook("mouseover", (component::call)&barco::handleMouseover);

			write("target.render", false);
			
			//iniciando todos os valores do range de movimento
			write("range.render", false);
			write("range.position.x",-128.0f+(w/2.0f));
			write("range.position.y",-128.0f+(h/2.0f));
			write("range.zoom", atr.moverange/128.0f);
			
			write("barcohover.render", false);
			
			
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
			/*cout<<" X: "<<x<<" Y: "<<y<<" w: "<<w<<" h: "<<h<<endl;
			//Descomentar para testes de collider bounding box
			float auxX,auxY,auxW,auxH;

			auxX = read<float>("collider.aabb.x");
			auxY = read<float>("collider.aabb.y");
			auxW = read<float>("collider.aabb.w");
			auxH = read<float>("collider.aabb.h");

			cout<<" X: "<<auxX<<" Y: "<<auxY<<" iX: "<<auxW<<" iY: "<<auxH<<endl;*/
			
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

		virtual void handleCollision(parameterbase::id pid, base* lastwrite, object::id owner) {
			bool longe, perto = false;
			float dx, dy =0.0f;
			if (pid == "collider.collision"){
				
				cout<<"colisao ";
				component::base * inimigo = read<component::base*>(pid);
				longe = sphereCollision(read<float>("x")+read<float>("collider.aabb.x"),read<float>("y")+read<float>("collider.aabb.y"),atr.range,
										inimigo->read<float>("x"),inimigo->read<float>("y"),inimigo->read<float>("w"));
				
				perto = sphereCollision(x,y,w,
										inimigo->read<float>("x"),inimigo->read<float>("y"),inimigo->read<float>("w"));
				
				//colisao de longe (range do barco vs. barco inimigo)
				if(longe)
				{
					//cout<<"longe ";	
					if((inimigo->read<string>("collider.tag") == "barco")||(inimigo->read<string>("collider.tag") == "porto"))
					{
						//removeHP(inimigo,1);
					}
				}
				
				//colisao de perto (barco vs barco inimigo)
				if(perto)
				{
		//			cout<<"perto ";
					if(inimigo->read<string>("collider.tag") == "barco")
					if(inimigo->read<string>("collider.tag") == "loot")
					{
						porto->write("cash",porto->read<float>("cash") + inimigo->read<float>("cash"));
						inimigo->destroy();
					}
				}
				//cout<<endl;
			}

		}

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
		bool sphereCollision(float ax,float ay,float ar,float bx, float by,float br) {
			int dX   = ax - bx;
			int dY   = ay - by;
			int dR   = br + ar;
			if((dX*dX)+(dY*dY) <= dR*dR)
				return true;
			return false;
		}

		virtual void handleClick(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (mouse1 == 1) {
				if (selected) {
					selected = false;
					write("range.render", false);
					//write("target.render", false);
				}
				else {
					if (read<bool>("mouseover")) {
						cout << "clicked over me" << endl;
						selected = true;
						write("range.render", true);
						write("target.render", true);
					}
				}
			}
		}
		
		virtual void handleMouseover(parameterbase::id pid, base* lastwrite, object::id owner) {
			write("barcohover.render", read<bool>("mouseover"));
		}
		
		virtual void handleLife(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "hp.value"){
				if(read<int>("hp.value") <= 0) {
					int barco = read<int>("barcotype");
					float x = read<float>("x"), y = read<float>("y");
					destroy();

					component::base* loot;
					loot = spawn("loot")->component("spatial");
					loot->write("value",read<float>("loot.value"));
					loot->write("x",x);
					loot->write("y",y);
				 }
			}
		}

		void updateHpText(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			//stringstream ss;
			//ss << "HP: ";
			//ss << hp;
			//write("hp.text", ss.str());
		}
};

// the build function
extern "C" { component::base * build() { return new barco(); } }
