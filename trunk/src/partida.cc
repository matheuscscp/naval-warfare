#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

enum barcotype {
	big = 0,
	medium,
	small,
	last
};

class partida : public component::base {
	private:
		// private vars
		
	public:
		// constructor and destructor
		partida() {
		}
		virtual ~partida() {
		}
		
		virtual gear2d::component::family family() { return "partida"; }

		virtual gear2d::component::type type() { return "partida"; }

		virtual std::string depends() { return ""; }

		virtual void setup(object::signature & sig) {
			// spawna os portos
			try {
				for (int i = 1; ; ++i) {
					stringstream ss;
					ss << "porto-p";
					ss << i;
					object::id porto = spawn( ss.str() );
					
					// encerra o loop
					if( !porto )
						throw evil();
					
					portos.push_back( porto->component("porto") );
				}
			} catch(evil&) {
			}
		}
		
		virtual void update(timediff dt) {
			
		}
		
		
	private:
		// static vars declaration
		static std::list<component::base*> portos;
};

// static vars definition
std::list<component::base*> partida::portos;

// the build function
extern "C" { component::base * build() { return new partida(); } }
