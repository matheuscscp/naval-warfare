#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;


class loot : public component::base {
	private:
		gear2d::link<int> cash;
		gear2d::link<float> x, y, w, h;

	public:
		// constructor and destructor
		loot() {
			loots.push_back(this);
		}
		virtual ~loot() {
			loots.remove(this);
		}
		
		gear2d::link<int> player;
		
		virtual gear2d::component::family family() { return "unit"; }

		virtual gear2d::component::type type() { return "loot"; }

		virtual std::string depends() { return "renderer/renderer collider/collider2d keyboard/keyboard spatial/space2d"; }

		virtual void setup(object::signature & sig) {
			initialize();
			init<int>("cash"        , sig["cash"]        , 0);
//			cout << "Barco virou loot" << endl;


		}
		virtual void update(timediff dt) {

		}
		
		
	private:
		static std::list<loot *> loots;
		static bool initialized;

		
	private:
		static void initialize() {
			if (initialized) return;
			initialized = true;
		}
};

std::list<loot *> loot::loots;
bool loot::initialized = false;

// the build function
extern "C" { component::base * build() { return new loot(); } }
