#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

enum barcotype {
	big,
	medium,
	small,
	none
};

namespace gear2d {
		template<> barcotype eval<barcotype>(std::string t, barcotype def) {
			if (t == "big") return big;
			if (t == "medium") return medium;
			if (t == "small") return small;
			return def;
	}
}

class porto : public component::base {
	private:
		gear2d::link<int> cash;
		std::list< component::base* > barcos;
	public:
		// constructor and destructor
		porto() { 
			portos.push_back(this);
		}
		virtual ~porto() { 
			portos.remove(this);
		}
		
		gear2d::link<int> player;
		
		virtual gear2d::component::family family() { return "porto"; }
		virtual gear2d::component::type type() { return "porto"; }
		virtual std::string depends() { return "renderer/renderer"; }
		virtual void setup(object::signature & sig) {
			initialize();
			player = fetch<int>("porto.player");
			player = eval<int>(sig["porto.player"], 0);
			write("gamesetup", 0);
			write("gameplay", 0);
			
// 			cout << "port: " << player << endl;
			
			// hookando o teclado
			hook("key.a");
			hook("key.s");
			hook("key.d");
			
			// dinheiro do porto
			init<int>("cash.value", sig["cash.value"], 1000);
			hook("cash.value", (component::call)&porto::updateCashText);
			cash = fetch<int>("cash.value");
			updateCashText("",0,0);
			
			criarBarco("barcopequeno");
			
			/* TODO: VERIFICAR  O NUMERO DO PLAYER E POSICIONAR DE
			 ACORDO */
		}
		
		virtual void update(timediff dt) {
		}
		
		virtual void criarBarco(const string& tbarco) {
			component::base* barco = spawn(tbarco)->component("spatial");
			barcos.push_back(barco);
			barco->write("porto", this);
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "key.a") {
				if (raw<int>("key.a") == 1) {
					criarBarco("barcopequeno");
				}
			}
			else if (pid == "key.s") {
				if (raw<int>("key.s") == 1) {
					criarBarco("barcomedio");
				}
			}
			else if (pid == "key.d") {
				if (raw<int>("key.d") == 1) {
					criarBarco("barcogrande");
				}
			}
		}
		
		void updateCashText(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			stringstream ss;
			ss << "Cash: ";
			ss << cash;
			write("cash.text", ss.str());
		}
		
	private:
		static bool initialized;
		static std::list<porto *> portos;
		
	private:
		static void initialize() {
			if (initialized) return;
			initialized = true;
		}
};

std::list<porto *> porto::portos;
bool porto::initialized = false;

// the build function
extern "C" { component::base * build() { return new porto(); } }
