#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class mainmenu : public component::base {
	private:
		// private vars
		
		
	public:
		// constructor and destructor
		mainmenu() {
			
		}
		
		virtual ~mainmenu() {
			
		}
		
		virtual gear2d::component::family family() { return "gamestate"; }

		virtual gear2d::component::type type() { return "mainmenu"; }

		virtual std::string depends() { return "keyboard/keyboard menu/singleselect"; }

		virtual void setup(object::signature & sig) {
			hook("key.up");
			hook("key.down");
			hook("key.return");
			
			hook("menu.focus");
		}
		
		virtual void update(timediff dt) {
			
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "key.up") {
				if (read<int>("key.up") == 1)
					write("menu.prev", true);
			}
			else if (pid == "key.down") {
				if (read<int>("key.down") == 1)
					write("menu.next", true);
			}
			else if (pid == "key.return") {
				if (read<string>("menu.focus") == "newgame")
					load("partida");
			}
			else if (pid == "menu.focus") {
				if (read<string>("menu.focus") == "newgame") {
					write<float>("newgame.alpha", 1.0f);
					write<float>("exitgame.alpha", 0.5f);
				}
				else {
					write<float>("newgame.alpha", 0.5f);
					write<float>("exitgame.alpha", 1.0f);
				}
			}
		}
		
	private:
		// static vars declaration
		
};

// static vars definition


// the build function
extern "C" { component::base * build() { return new mainmenu(); } }
