#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

/**
 * @li @c menu.trigger Hook to know when options are selected. @b bool
 * @li @c menu.keyprev Set the key to move to the previous option. @b string
 * @li @c menu.keynext Set the key to move to the next option. @b string
 */

class fade : public component::base {
	private:
		// private vars
		string surface;
		timediff delay;
		timediff timer;
		
	public:
		// constructor and destructor
		fade() {
		}
		virtual ~fade() {
		}
		
		virtual gear2d::component::family family() { return "animation"; }

		virtual gear2d::component::type type() { return "fade"; }

		virtual std::string depends() { return "renderer/renderer"; }

		virtual void setup(object::signature & sig) {
			surface = sig["fade.surface"];
			delay = eval<timediff>(sig["fade.delay"]);
			timer = 0.0f;
		}
		
		virtual void update(timediff dt) {
			timer += dt;
			if (timer < delay) return;
			
			float alpha = read<float>(surface + ".alpha");
			
			// destroi o objeto e retorna o update caso o alpha ja seja zero
			if (alpha == 0.0f) {
				destroy();
				return;
			}
			
			// caso novo valor do alpha seja negativo, setta para zero e retorna
			if (alpha - 0.01f < 0.0f) {
				write<float>(surface + ".alpha", 0.0f);
				return;
			}
			
			// decrementa a opacidade exponencialmente
			add<float>(surface + ".alpha", -0.01f);
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			
		}
		
	private:
		// static vars declaration
		
};

// static vars definition


// the build function
extern "C" { component::base * build() { return new fade(); } }
