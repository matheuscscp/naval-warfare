#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class mainmenu : public component::base {
	private:
		// private vars
		component::base* credits;
		bool credits_freshspawn;
		
	public:
		// constructor and destructor
		mainmenu() :
		credits(0),
		credits_freshspawn(false)
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
			
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "menu.trigger") {
				string opt = read<string>("menu.focus");
				if (opt == "newgame") {
					load("partida");
				} else if (opt == "credits") {
					if (!credits) {
						credits_freshspawn = true;
						credits = spawn("creditos")->component("spatial");
					}
				}
			} else if (pid == "key.return") {
				if (credits_freshspawn) {
					credits_freshspawn = false;
				}
				else if ((credits) && (read<int>("key.return") == 1)) {
					credits->destroy();
					credits = 0;
				}
			}
		}
		
	private:
		// static vars declaration
		
};

// static vars definition


// the build function
extern "C" { component::base * build() { return new mainmenu(); } }
