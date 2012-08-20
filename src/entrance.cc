#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class entrance : public component::base {
	private:
		// private vars
		component::base* curstate;
		
	public:
		// constructor and destructor
		entrance() : curstate(NULL) {
			
		}
		
		virtual ~entrance() {
			if (curstate)
				unloadState();
		}
		
		virtual gear2d::component::family family() { return "entrance"; }

		virtual gear2d::component::type type() { return "entrance"; }

		virtual std::string depends() { return ""; }

		virtual void setup(object::signature & sig) {
			loadState("mainmenu");
		}
		
		virtual void update(timediff dt) {
			
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			unloadState();
			loadState( last->read<object::type>("newstate") );
		}
		
		void loadState(object::type type) {
			curstate = spawn(type)->component("gamestate");
			hook(curstate, "newstate");
		}
		
		void unloadState() {
			curstate->destroy();
			curstate = NULL;
		}
		
	private:
		// static vars declaration
		
};

// static vars definition


// the build function
extern "C" { component::base * build() { return new entrance(); } }
