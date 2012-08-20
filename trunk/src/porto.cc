#include <iostream>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

enum barcotype {
	big = 0,
	medium,
	small,
	lastsize
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
		gear2d::link<int> player;
		
		gear2d::link<int> cash;
		gear2d::link<int> hp;
		std::list< component::base* > barcos;
		int qtde_barcos[lastsize];
		component::base* painel;
		gear2d::link<float> spawn_x;
		gear2d::link<float> spawn_y;
		
	public:
		// constructor and destructor
		porto() { 
			portos.push_back(this);
		}
		virtual ~porto() { 
			portos.remove(this);
		}
		
		virtual gear2d::component::family family() { return "porto"; }
		virtual gear2d::component::type type() { return "porto"; }

		virtual std::string depends() { return "renderer/renderer"; }

		virtual void setup(object::signature & sig) {
			initialize();
			init<int>("porto.player", sig["porto.player"], 0);
			player = fetch<int>("porto.player");
			write<bool>("gamesetup", 0);
			write<bool>("gameplay", 0);
			
			// flag para a partida hookar
			write<bool>("morto", false);
			
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
			write<int>("spawn.tipo", lastsize);
			hook("spawn.tamanho", (component::call)&porto::handlePainel);
			painel = spawn("painel")->component("spatial");
			painel->write("porto", this);
			painel->write<float>("x", eval<float>(sig["painel.x"], 0));
			painel->write<float>("y", eval<float>(sig["painel.y"], 0));
			
			// zerando a quantidade de barcos deste porto
			for( int i = 0; i < lastsize; ++i )
				qtde_barcos[i] = 0;
			
			// zerando os dados da partida para este porto
			write<int>("cashusado", 0);
			write<int>("cashganho", 0);
			write<int>("grandefabricado", 0);
			write<int>("mediofabricado", 0);
			write<int>("pequenofabricado", 0);
			write<int>("grandedestruido", 0);
			write<int>("mediodestruido", 0);
			write<int>("pequenodestruido", 0);
			
			// inicia a posicao de spawn de barcos
			init<float>("spawn.x", sig["spawn.x"], 0);
			init<float>("spawn.y", sig["spawn.y"], 0);
			spawn_x = fetch<float>("spawn.x");
			spawn_y = fetch<float>("spawn.y");
		}
		
		virtual void update(timediff dt) {
			// destroi o porto
			if (hp <= 0 ) {
				// avisa a partida
				write<bool>("morto", true);
				
				// destroi o painel
				painel->destroy();
				
				// destroi os barcos
				for (int i = 0; i < lastsize; ++i) {
					qtde_barcos[i] = 0;
				}
				while (barcos.size()) {
					barcos.back()->destroy();
					barcos.pop_back();
				}
				
				destroy();
			}
		}
		
		virtual void criarBarco(const string& tbarco, barcotype barco_t, bool debitar = true) {
			// soh cria se eh pra debitar e tem dinheiro OU se nao eh pra debitar
			if (((debitar) && (cash >= custo_barco[barco_t])) || (!debitar)) {
				// cria e poe na lista
				component::base* barco = spawn(tbarco)->component("unit");
				barco->write("porto", this);
				
				barcos.push_back(barco);
				
				// incrementa os contadores
				qtde_barcos[barco_t]++;
				switch (barco_t) {
					case big:		add<int>("grandefabricado", 1);	break;
					case medium:	add<int>("mediofabricado", 1);		break;
					case small:		add<int>("pequenofabricado", 1);	break;
					default:
						break;
				}
				
				// debita se for pra debitar
				if(debitar) {
					cash = (cash - custo_barco[barco_t]);
					add<int>("cashusado", custo_barco[barco_t]);
				}
				
				// setta a posicao
				barco->write<float>("x", spawn_x);
				barco->write<float>("y", spawn_y);
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
			// itera nos 3 tipos de barco, somando a contribuicao do desconto no cash que sera ganho naquele turno
			float desconto = 0;
			for( int i = 0; i < lastsize; ++i ) {
				float peso = ( float( lastsize * (lastsize + 1) ) / 2 ) * (i + 1);
				desconto += ( peso * qtde_barcos[i] / cash_max_barcos[i] );
			}
			
			int cash_final = max<int>(min_cash_turn, max_cash_turn * ( 1 - desconto ));
			
			cash = cash + cash_final;
			add<int>("cashganho", cash_final);
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
			// remove da lista
			component::base* barco = read<component::base*>("barcomorrendo");
			barcos.remove(barco);
			
			// decrementa os contadores
			int tipo = barco->read<barcotype>("tipo");
			--qtde_barcos[tipo];
			switch (tipo) {
				case big:		add<int>("grandedestruido", 1);	break;
				case medium:	add<int>("mediodestruido", 1);		break;
				case small:		add<int>("pequenodestruido", 1);	break;
				default:
					break;
			}
		}
		

	private:
		static std::list<porto *> portos;
		static bool initialized;
		static int custo_barco[lastsize];
		static int min_cash_turn;
		static int max_cash_turn;
		static int cash_max_barcos[lastsize];
		
	private:
		static void initialize() {
			if (initialized) return;
			initialized = true;
		}
};

std::list<porto *> porto::portos;
bool porto::initialized = false;
int porto::custo_barco[lastsize];
int porto::min_cash_turn;
int porto::max_cash_turn;
int porto::cash_max_barcos[lastsize];

// the build function
extern "C" { component::base * build() { return new porto(); } }
