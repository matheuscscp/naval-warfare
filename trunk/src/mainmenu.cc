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

		virtual std::string depends() { return ""; }

		virtual void setup(object::signature & sig) {
			// inicializa o nome do proximo gamestate para a entrance hookar
			write<object::type>("newstate", "");
			
			spawn("partida");
		}
		
		virtual void update(timediff dt) {
			
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			
		}
		
	private:
		// static vars declaration
		
};

// static vars definition


// the build function
extern "C" { component::base * build() { return new mainmenu(); } }
