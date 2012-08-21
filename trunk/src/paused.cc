#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class paused : public component::base {
	public:
		static bool ispaused;
		static std::set<paused*> pausedobjects;
		static bool writing;
		
		paused() { pausedobjects.insert(this); }
		virtual ~paused() { pausedobjects.erase(this); }
		
		virtual gear2d::component::family family() { return "pause"; }
		virtual gear2d::component::type type() { return "paused"; }
		virtual void setup(object::signature & sig) {
			write("paused", ispaused);
			hook("paused");
		}
		
		virtual void handle (parameterbase::id pid, base * lastwrite, object::id owner) {
			if (writing) return;
			read("paused", ispaused);
			writing = true;
			for (std::set<paused*>::iterator i = pausedobjects.begin(); i != pausedobjects.end(); i++)
				write((*i)->owner, "paused", ispaused);
		}
		
		virtual void update(timediff dt) { }
};

bool paused::ispaused = false;
bool paused::writing = false;
std::set<paused*> paused::pausedobjects;

// the build function
extern "C" { component::base * build() { return new paused(); } }