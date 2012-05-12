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
		
	public:
		barco() { }
		virtual ~barco() { }
		
		virtual gear2d::component::family family() { return "unit"; }
		
		virtual gear2d::component::type type() { return "barco"; }
		
		virtual std::string depends() { return "kinematics/kinematic2d mouse/mouse"; }
		

		virtual void handleclick(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (mouse1 == 1) {
				destx = read<float>("mouse.x");
				desty = read<float>("mouse.y");
			}
				
			
			/* TODO:
			 * 
			 * checa raio
			 * se tiver no raio, seta destino
			 * se nao, faz o que?
			 * 	-> toca um som
			 */
		}
		
		virtual void setup(object::signature & sig) {
			init<int>("attackrange", sig["attackrange"], 40);
			init<int>("moverange", sig["moverange"], 200);
			hook("mouse.1", (component::call)&barco::handleclick);
			
			x = fetch<float>("x");
			y = fetch<float>("y");
			w = fetch<float>("w");
			h = fetch<float>("h");
			
			mouse1 = fetch<int>("mouse.1");
			
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
			
			cout << destx - cx << " " << desty - cy << endl;
		}
};

// the build function
extern "C" { component::base * build() { return new barco(); } }