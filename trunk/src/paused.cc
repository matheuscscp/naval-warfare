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
			// retorna o metodo se algum outro objeto ja estiver broadcastando a mudanca no estado de pausa (lockado)
			if (writing) return;
			
			// le o novo valor da flag de pausa
			read("paused", ispaused);
			
			// locka este handle para nao entrar em loop infinito, broadcasta e depois unlocka
			writing = true;
			for (std::set<paused*>::iterator i = pausedobjects.begin(); i != pausedobjects.end(); i++) {
				// este teste impede que o novo estado de pausa seja escrito novamente no proprio objeto que fez a mudanca
				if (*i != this) {
					write((*i)->owner, "paused", ispaused);
				}
			}
			writing = false;
		}
		
		virtual void update(timediff dt) { }
};

bool paused::ispaused = false;
bool paused::writing = false;
std::set<paused*> paused::pausedobjects;

// the build function
extern "C" { component::base * build() { return new paused(); } }