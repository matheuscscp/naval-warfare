#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class porto : public component::base {
	public:
		// constructor and destructor
		porto() { 
			portos.push_back(this);
		}
		virtual ~porto() { 
			portos.remove(this);
		}
		
		gear2d::link<int> player;
		
		virtual gear2d::component::family family() { return "porto"; }
		virtual gear2d::component::type type() { return "porto"; }
		virtual void setup(object::signature & sig) {
			initialize();
			player = fetch<int>("porto.player");
			player = eval<int>(sig["porto.player"], 0);
			write("gamesetup", 0);
			write("gameplay", 0);
			
			cout << "port: " << player << endl;
			
			spawn("barco")->component("spatial")->write("porto", this); //comentado para não travar o programa travando o programa
			
			/* TODO: VERIFICAR  O NUMERO DO PLAYER E POSICIONAR DE
			 ACORDO */
		}			
		
		virtual void update(timediff dt) {
		}
		
	private:
		static bool initialized;
		static std::list<porto *> portos;
		
	private:
		static void initialize() {
			if (initialized) return;
			initialized = true;
		}
};

std::list<porto *> porto::portos;
bool porto::initialized = false;

// the build function
extern "C" { component::base * build() { return new porto(); } }