#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

/**
 * @li @c menu.trigger Hook to know when options are selected. @b bool
 * @li @c menu.keyprev Set the key to move to the previous option. @b string
 * @li @c menu.keynext Set the key to move to the next option. @b string
 */

class singleselectmanager : public component::base {
	private:
		// private vars
		std::list<string> options;
		string key_prev;
		string key_next;
		
	public:
		// constructor and destructor
		singleselectmanager() {
			
		}
		
		virtual ~singleselectmanager() {
			
		}
		
		virtual gear2d::component::family family() { return "menumanager"; }

		virtual gear2d::component::type type() { return "singleselectmanager"; }

		virtual std::string depends() { return "keyboard/keyboard menu/singleselect"; }

		virtual void setup(object::signature & sig) {
			// inicializa as teclas para selecionar as opcoes
			key_prev = eval<string>(sig["menu.keyprev"]);
			key_next = eval<string>(sig["menu.keynext"]);
			
			// hookando as acoes
			hook(key_prev);
			hook(key_next);
			hook("key.return");
			hook("menu.focus");
			
			// parametro a ser hookado por componentes que instanciem objetos com singleselectmanager
			write<bool>("menu.trigger", false);
			
			// salva as opcoes numa lista
			string raw_options = read<string>("menu.options");
			string tmp_option = "";
			for (int i = 0; i < raw_options.size(); ++i) {
				// poe uma opcao na lista e limpa o buffer
				if (raw_options[i] == ' ') {
					options.push_back(tmp_option);
					tmp_option = "";
				}
				// poe a ultima opcao na lista
				else if (i == int(raw_options.size()) - 1) {
					tmp_option += raw_options[i];
					options.push_back(tmp_option);
				}
				// concatena a letra da opcao
				else
					tmp_option += raw_options[i];
			}
		}
		
		virtual void update(timediff dt) {
			
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == key_prev) {
				if (read<int>(key_prev) == 1)
					write("menu.prev", true);
			}
			else if (pid == key_next) {
				if (read<int>(key_next) == 1)
					write("menu.next", true);
			}
			else if (pid == "key.return") {
				if (read<int>("key.return") == 1)
					write<bool>("menu.trigger", true);
			}
			else if (pid == "menu.focus") {
				// setta as opacidades das opcoes
				string focus = read<string>("menu.focus");
				for (std::list<string>::iterator it = options.begin(); it != options.end(); ++it) {
					if (focus == *it)
						write<float>((*it) + ".alpha", 1.0f);
					else
						write<float>((*it) + ".alpha", 0.5f);
				}
			}
		}
		
	private:
		// static vars declaration
		
};

// static vars definition


// the build function
extern "C" { component::base * build() { return new singleselectmanager(); } }
