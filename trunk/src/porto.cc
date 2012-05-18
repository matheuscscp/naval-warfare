#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class porto : public component::base {
	public:
		// constructor and destructor
		porto() { }
		virtual ~porto() { }
		
		virtual gear2d::component::family family() { return "porto"; }
		virtual gear2d::component::type type() { return "porto"; }
		virtual void setup(object::signature & sig) {
			initialize();
			init<int>("porto.player", sig["porto.player"], 0);
			
			/* TODO: VERIFICAR O PLAYER == 1 OU == 2 */
		}			
		
		virtual void update(timediff dt) {
			cout << player << endl;
		}
		
	private:
		static int player;
		static bool initialized;
		
	private:
		static void initialize() {
			if (initialized) return;
			initialized = true;
			player = 1;
		}
};

int porto::player = -1;
bool porto::initialized = false;

// the build function
extern "C" { component::base * build() { return new porto(); } }