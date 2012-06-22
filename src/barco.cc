#include <iostream>
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
		
		float destx, desty;
		
		gear2d::link<float> x, y, w, h;
		gear2d::link<int> mouse1;

		struct atributos
		{
			gear2d::link<string> tipo;		//tipo do barco
			gear2d::link<int>    hp;		//vida
			gear2d::link<int>    range;		//range de ataque
			gear2d::link<int>    moverange;		//range de movimento
			gear2d::link<int>    speed;		//velocidade de movimento
			gear2d::link<int>    dmg;		//dano
			gear2d::link<int>    loot;		//loot dropado
		}atr;
		
		
		component::base * porto;
		
	public:
		barco() { }
		virtual ~barco() { }
		
		virtual gear2d::component::family family() { return "unit"; }
		
		virtual gear2d::component::type type() { return "barco"; }
		
		virtual std::string depends() { 
			return "kinematics/kinematic2d mouse/mouse mouseover/mouseover collider/collider2d";
		}
		
		virtual void handle(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (pid == "porto") {
				porto = read<component::base *>("porto");
				hook(porto, "gamesetup");
				hook(porto, "gameplay");
			}
		}
		
		virtual void handlecollision(parameterbase::id pid, base* lastwrite, object::id owner) {
			int dX,dY,dist=0;
			if (pid == "collider.collision")
			{
				component::base * target = read<component::base*>(pid);
				if (target->read<string>("collider.tag") == "barco") 
				{
					dX   = x - target->read<int>("x");
					dY   = y - target->read<int>("y");
					dist = (dX*dX) + (dY*dY);
					if(dist <= (target->read<int>("w") * target->read<int>("w")))
					{
						//TODO:: ATACAR
					}
				}
			}		
		}

		virtual void handleclick(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (mouse1 == 1) {
				destx = read<int>("mouse.x");
				desty = read<int>("mouse.y");
				if (read<bool>("mouseover")) {
					cout << "clicked over me" << endl;
					if (read<bool>("range.render") == false) {
						write("range.render", true);
					} else {
						write("range.render", false);
					}
						
				}
			}
		}
		
		virtual void setup(object::signature & sig) {
			barcotype t = eval(sig["barcotype"], small);
			init<int>("hp"        , sig["hp"]        , 100);
			init<int>("range"     , sig["range"]     , 64);
			init<int>("moverange" , sig["moverange"] , 128);
			init<int>("speed"     , sig["speed"]     , 300);
			init<int>("dmg"       , sig["dmg"]       , 10);
			init<int>("loot"      , sig["loot"]      , 100);
			
			atr.hp 		= fetch<int>("hp");
			atr.range	= fetch<int>("range");
			atr.moverange 	= fetch<int>("moverange");
			atr.speed	= fetch<int>("speed");
			atr.dmg		= fetch<int>("dmg");
			atr.loot	= fetch<int>("loot");	
			
			hook("mouse.1", (component::call)&barco::handleclick);

			write<component::base *>("porto", NULL);
			porto = NULL;
			hook("porto");
			
			x = fetch<float>("x");
			y = fetch<float>("y");
			w = fetch<float>("w");
			h = fetch<float>("h");
			
			mouse1 = fetch<int>("mouse.1");

			//setando a caixa de colisao do raio de ataque
			write("collider.aabb.x",((atr.range-w)/2)+x);
			write("collider.aabb.y",((atr.range-h)/2)+y);
			write("collider.aabb.w",atr.range);
			write("collider.aabb.h",atr.range);

			hook("collider.collision",(component::call)&barco::handlecollision);//hookando a colisao
			
			write("range.render", true);
			
			cx = x + w/2;
			cy = y + h/2;
			
			destx = cx;
			desty = cy;
		}			
		
		virtual void update(timediff dt) {
			
			cx = x + w/2;
			cy = y + h/2;
			
			write("x.speed", destx - cx);
			write("y.speed", desty - cy);
		}
};

// the build function
extern "C" { component::base * build() { return new barco(); } }
