#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class barco : public component::base {
	private:
		
		float cx, cy;
		
		float destx, desty;
		
		gear2d::link<float> x, y, w, h;
		gear2d::link<int> mouse1;

		//atributos
		gear2d::link<string> type;		//tipo do barco
		gear2d::link<int>    hp;		//vida
		gear2d::link<int>    range;		//range de ataque
		gear2d::link<int>    moverange;		//range de movimento
		gear2d::link<int>    speed;		//velocidade de movimento
		gear2d::link<int>    dmg;		//dano
		gear2d::link<int>    loot;		//loot dropado

		init<string>("type"            , sig["type"]            , "small");
		init<int>   ("hp.value"        , sig["hp.value"]        , 0);
		init<int>   ("range.value"     , sig["range.value"]     , 0);
		init<int>   ("moverange.value" , sig["moverange.value"] , 0);
		init<int>   ("speed.value"     , sig["speed.value"]     , 0);
		init<int>   ("dmg.value"       , sig["dmg.value"]       , 0);
		init<int>   ("loot.value"      , sig["loot.value"]      , 0);
		
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
				if (c->read<string>("collider.tag") == "barco") 
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
			switch(read<int>"type.value")
			{
				case 0://small
					write("hp.value"        , 100);
					write("range.value"     , 64);
					write("moverange.value" , 128);
					write("speed.value"     , 300);
					write("dmg.value"       , 10);
					write("loot.value"      , 100);	
				break;
				case 1://medium
					write("hp.value"        , 200);
					write("range.value"     , 64);
					write("moverange.value" , 128);
					write("speed.value"     , 200);
					write("dmg.value"       , 10);
					write("loot.value"      , 100);	
				break;
				case 2://big
					write("hp.value"        , 300);
					write("range.value"     , 64);
					write("moverange.value" , 128);
					write("speed.value"     , 100);
					write("dmg.value"       , 10);
					write("loot.value"      , 100);
				break;
			}

			hook("mouse.1", (component::call)&barco::handleclick);

			int attRange = read<int>("range.value");

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
