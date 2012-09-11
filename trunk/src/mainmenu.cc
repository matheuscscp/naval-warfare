#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class mainmenu : public component::base {
	private:
		// private vars
		component::base* screen;
		bool screen_freshspawn;
		
		bool load_partida;
		timediff loading_delay;
		timediff lifetime;
		
	public:
		// constructor and destructor
		mainmenu() :
		screen(0),
		screen_freshspawn(false),
		load_partida(false),
		loading_delay(0.0f),
		lifetime(0.0f)
		{
		}
		
		virtual ~mainmenu() {
			
		}
		
		virtual gear2d::component::family family() { return "gamestate"; }

		virtual gear2d::component::type type() { return "mainmenu"; }

		virtual std::string depends() { return "keyboard/keyboard menumanager/singleselectmanager"; }

		virtual void setup(object::signature & sig) {
			hook("menu.trigger");
			hook("key.return");
		}
		
		virtual void update(timediff dt) {
			lifetime += dt;
			
			if ((load_partida) && (lifetime >= loading_delay)) {
				load("partida");
			}
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			if (load_partida) return;
			if (pid == "menu.trigger") {
				string opt = read<string>("menu.focus");
				if (opt == "newgame") {
					load_partida = true;
					loading_delay = lifetime + 3.0f;
					spawn("loading");
				} else if (opt == "directions") {
					if (!screen) {
						screen_freshspawn = true;
						screen = spawn("instrucoes")->component("spatial");
					}
				} else if (opt == "credits") {
					if (!screen) {
						screen_freshspawn = true;
						screen = spawn("creditos")->component("spatial");
					}
				} else if (opt == "exitgame") {
					exit(0);
				}
			} else if (pid == "key.return") {
				if (screen_freshspawn) {
					screen_freshspawn = false;
				}
				else if ((screen) && (read<int>("key.return") == 1)) {
					screen->destroy();
					screen = 0;
				}
			}
		}
		
	private:
		// static vars declaration
		
};

// static vars definition


// the build function
extern "C" { component::base * build() { return new mainmenu(); } }
