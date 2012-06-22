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
			barcotype t = eval(sig["barco.type"], small);
			switch(t)
			{
				case small://small
					atr.hp 		=	100;
					atr.range	=	64;
					atr.moverange 	=	128;
					atr.speed	=	300;
					atr.dmg		=	10;
					atr.loot	=	100;	
				break;
				case medium://medium
					atr.hp		=	200;
					atr.range	=	64;
					atr.moverange	=	128;
					atr.speed	=	200;
					atr.dmg		=	10;
					atr.loot	=	100;	
				break;
				case big://big
					atr.hp		=	300;
					atr.range	=	64;
					atr.moverange	=	128;
					atr.speed	=	100;
					atr.dmg		=	10;
					atr.loot	=	100;
				break;
			}

			hook("mouse.1", (component::call)&barco::handleclick);

			int attRange = read<int>("range");

			write<component::base *>("porto", NULL);
			porto = NULL;
			hook("porto");
			
			x = fetch<float>("x");
			y = fetch<float>("y");
			w = fetch<float>("w");
			h = fetch<float>("h");
			
			mouse1 = fetch<int>("mouse.1");

			//setando a caixa de colisao do raio de ataque
			write("collider.aabb.x",((attRange-w)/2)+x);
			write("collider.aabb.y",((attRange-h)/2)+y);
			write("collider.aabb.w",attRange);
			write("collider.aabb.h",attRange);

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
