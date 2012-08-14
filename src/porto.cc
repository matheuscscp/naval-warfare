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
		int qtde_barcos[last];
		component::base* painel;
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
			
			// inicializa um parametro para hookar um barco que acabou de morrer
			write<component::base*>("barcomorrendo", 0);
			hook("barcomorrendo", (component::call)&porto::removeBarco);
			
// 			cout << "port: " << player << endl;
			
			// dinheiro do porto
			init<int>("cash.value", sig["cash.value"], 1000);
			hook("cash.value", (component::call)&porto::updateCashText);
			cash = fetch<int>("cash.value");
			updateCashText("",0,0);
			hook("myturn", (component::call)&porto::getTurnCash);
			
			// minimo e maximo de dinheiro a ganhar por turno
			min_cash_turn = eval<int>( sig["cash.minperturn"], 50 );
			max_cash_turn = eval<int>( sig["cash.maxperturn"], 500 );
			
			// custo dos barcos
			custo_barco[big] = eval<int>( sig["cost.barcogrande"], 300 );
			custo_barco[medium] = eval<int>( sig["cost.barcomedio"], 200 );
			custo_barco[small] = eval<int>( sig["cost.barcopequeno"], 100 );
			
			// quantidade maxima de barcos de cada tipo para o calculo do cash ganho por turno
			cash_max_barcos[big] = eval<int>( sig["cash.maxgrande"], 3 );
			cash_max_barcos[medium] = eval<int>( sig["cash.maxmedio"], 4 );
			cash_max_barcos[small] = eval<int>( sig["cash.maxpequeno"], 5 );
			
			// vida do porto
			init<int>("hp.value", sig["hp.value"], 100);
			hook("hp.value", (component::call)&porto::updateHpText);
			hook("hp.value", (component::call)&porto::handleLife);
			hp = fetch<int>("hp.value");
			updateHpText("",0,0);
			
			// painel de fabricacao de barcos
			write<string>("spawn.tamanho", "");
			write<int>("spawn.tipo", last);
			hook("spawn.tamanho", (component::call)&porto::handlePainel);
			painel = spawn("painel")->component("spatial");
			painel->write("porto", this);
			painel->write<float>("x", eval<float>(sig["painel.x"], 0));
			painel->write<float>("y", eval<float>(sig["painel.y"], 0));
			
			// zerando a quantidade de barcos deste porto
			for( int i = 0; i < last; ++i )
				qtde_barcos[i] = 0;
			
			// zerando os dados da partida para este porto
			write<int>("cashusado", 0);
			write<int>("cashganho", 0);
		}
		
		virtual void update(timediff dt) {
		}
		
		virtual void criarBarco(const string& tbarco, barcotype barco_t, bool debitar = true) {
			// soh cria se eh pra debitar e tem dinheiro OU se nao eh pra debitar
			if (((debitar) && (cash >= custo_barco[barco_t])) || (!debitar)) {
				component::base* barco = spawn(tbarco)->component("unit");
				barcos.push_back(barco);
				barco->write("porto", this);
				qtde_barcos[barco_t]++;
				
				// debita se for pra debitar
				if(debitar) {
					cash = (cash - custo_barco[barco_t]);
					add<int>("cashusado", custo_barco[barco_t]);
				}
			}
		}

		virtual void handleLife(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "hp.value"){
				if(read<int>("hp.value") <= 0) {
					cout<<"PORTO MORTO"<<endl;	
				}
			}
		}
		
		void updateCashText(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			stringstream ss;
			ss << "Cash: ";
			ss << cash;
			write("cash.text", ss.str());
		}
		
		void getTurnCash() {
			float desconto = 0;
			
			for( int i = 0; i < last; ++i ) {
				float peso = ( float( last * (last + 1) ) / 2 ) * (i + 1);
				desconto += ( peso * qtde_barcos[i] / cash_max_barcos[i] );
			}
			
			cash = cash + max<int>(min_cash_turn, max_cash_turn * ( 1 - desconto ));
		}
		
		void updateHpText(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			stringstream ss;
			ss << "HP: ";
			ss << hp;
			write("hp.text", ss.str());
		}
		
		void handlePainel(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			criarBarco(read<string>("spawn.tamanho"), read<barcotype>("spawn.tipo"));
		}
		
		void removeBarco(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			component::base* barco = read<component::base*>("barcomorrendo");
			--qtde_barcos[barco->read<barcotype>("tipo")];
			barcos.remove(barco);
		}
		

	private:
		static std::list<porto *> portos;
		static bool initialized;
		static int custo_barco[last];
		static int min_cash_turn;
		static int max_cash_turn;
		static int cash_max_barcos[last];
		
	private:
		static void initialize() {
			if (initialized) return;
			initialized = true;
		}
};

std::list<porto *> porto::portos;
bool porto::initialized = false;
int porto::custo_barco[last];
int porto::min_cash_turn;
int porto::max_cash_turn;
int porto::cash_max_barcos[last];

// the build function
extern "C" { component::base * build() { return new porto(); } }
