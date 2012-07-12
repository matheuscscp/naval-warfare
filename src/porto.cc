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
		gear2d::link<int> hp;
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
			
			// dinheiro do porto
			init<int>("cash.value", sig["cash.value"], 1000);
			hook("cash.value", (component::call)&porto::updateCashText);
			cash = fetch<int>("cash.value");
			updateCashText("",0,0);
			hook("myturn", (component::call)&porto::getTurnCash);
			
			// custo dos barcos
			custo_barco[big] = eval<int>( sig["cost.barcogrande"], 300 );
			custo_barco[medium] = eval<int>( sig["cost.barcomedio"], 200 );
			custo_barco[small] = eval<int>( sig["cost.barcopequeno"], 100 );
			
			// hooks pra spawnar FIXME: input temporario
			hook("key.i",(component::call)&porto::handleBarco );
			hook("key.o",(component::call)&porto::handleBarco );
			hook("key.p",(component::call)&porto::handleBarco );

			// vida do porto
			init<int>("hp.value", sig["hp.value"], 100);
			hook("hp.value", (component::call)&porto::updateHpText);
			hook("hp.value", (component::call)&porto::handleLife);
			hp = fetch<int>("hp.value");
			updateHpText("",0,0);
			
			criarBarco("barcopequeno", small, false);
			
			/* TODO: VERIFICAR  O NUMERO DO PLAYER E POSICIONAR DE
			 ACORDO */
		}
		
		virtual void update(timediff dt) {
		}
		
		virtual void criarBarco(const string& tbarco, barcotype barco_t, bool debitar = true) {
			if (((debitar) && (cash >= custo_barco[barco_t])) || (!debitar)) {
				component::base* barco = spawn(tbarco)->component("spatial");
				barcos.push_back(barco);
				barco->write("porto", this);
				if(debitar)
					cash = (cash - custo_barco[barco_t]);
			}
		}

		virtual void handleLife(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "hp.value"){
				if(read<int>("hp.value") <= 0) {
					cout<<"PORTO MORTO"<<endl;	
				}
			}
		}
		
		virtual void handleBarco(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "key.i" ||  pid == "key.j") {
				if (raw<int>("key.i") == 1 || raw<int>("key.j") == 1) {
					criarBarco("barcopequeno", small);
				}
			}
			else if (pid == "key.o" || pid == "key.k") {
				if (raw<int>("key.o") == 1 || raw<int>("key.k") == 1) {
					criarBarco("barcomedio", medium);
				}
			}
			else if (pid == "key.p" || pid == "key.l") {
				if (raw<int>("key.p") == 1 || raw<int>("key.l") == 1) {
					criarBarco("barcogrande", big);
				}
			}
		}
		
		void updateCashText(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			stringstream ss;
			ss << "Cash: ";
			ss << cash;
			write("cash.text", ss.str());
		}
		
		void getTurnCash(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			cash = cash + (1);//FIXME
		}
		
		void updateHpText(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			stringstream ss;
			ss << "HP: ";
			ss << hp;
			write("hp.text", ss.str());
		}
		

	private:
		static std::list<porto *> portos;
		static bool initialized;
		static int custo_barco[last];
		
	private:
		static void initialize() {
			if (initialized) return;
			initialized = true;
		}
};

std::list<porto *> porto::portos;
bool porto::initialized = false;
int porto::custo_barco[last];

// the build function
extern "C" { component::base * build() { return new porto(); } }
