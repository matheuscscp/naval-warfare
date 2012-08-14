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
			portos.push_back(spawn("porto-p1")->component("porto"));
			portos.push_back(spawn("porto-p2")->component("porto"));
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
