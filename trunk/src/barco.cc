#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

class barco : public component::base {
	public:
		barco() { }
		virtual ~barco() { }
		
		virtual gear2d::component::family family() { return "unit"; }
		
		virtual gear2d::component::type type() { return "barco"; }
		
		virtual void setup(object::signature & sig) {
		}			
		
		virtual void update(timediff dt) {
			cout << "oi barco!" << endl;
		}
};

// the build function
extern "C" { component::base * build() { return new barco(); } }