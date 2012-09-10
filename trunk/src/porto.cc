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
		int hp_max;
		float hp_ypos;
		std::list< component::base* > barcos;
		
		/* numero de barcos prontos */
		int barcos_prontos;
		
		int qtde_barcos[lastsize];
		component::base* painel;
		gear2d::link<float> spawn_x;
		gear2d::link<float> spawn_y;
		
		gear2d::link<bool> paused;
		
		component::base* animation;
		
	public:
		// constructor and destructor
		porto() { 
		}
		virtual ~porto() { 
		}
		
		virtual gear2d::component::family family() { return "porto"; }
		virtual gear2d::component::type type() { return "porto"; }

		virtual std::string depends() { return "renderer/renderer pause/paused"; }

		virtual void setup(object::signature & sig) {
// 			initialize();
			
			paused = fetch<bool>("paused");
			
			barcos_prontos = 0;
			init<int>("porto.player", sig["porto.player"], 0);
			player = fetch<int>("porto.player");
			write<bool>("gamesetup", false);
			write<bool>("gameplay", false);
			hook("gamesetup", (component::call)&porto::handleGameSetup);
			hook("gameplay", (component::call)&porto::handleGamePlay);
			
			write<component::base*>("done", 0); /* se todos os barcos reportarem done */
			
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
			hp = fetch<int>("hp.value");
			hp_max = hp;
			hp_ypos = read<float>("hp.position.y");
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
			
			// setta a velocidade da animacao de abertura de turno
			init<float>("animacao.x.speed", sig["animacao.x.speed"], 0.0f);
			init<float>("animacao.y.speed", sig["animacao.y.speed"], 0.0f);
			
			// inicia o caminho das imagens de barco
			init<string>("images.path", sig["images.path"], "");
			init<string>("images.direction", sig["images.direction"], "");
		}
		
		virtual void update(timediff dt) {
			if (paused) return;
			// destroi o porto
			if (hp <= 0) {
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
			
			/* se nao tivermos barcos, estamos done no gameplay */
			if (barcos.size() == 0 && read<bool>("gameplay")) { 
				if (read<component::base*>("done") == NULL)
					write("done", this);
			}
			
			// barra de hp
			int icon_h = read<int>("hpback.clip.h");
			write("hp.clip.h", (hp*icon_h)/hp_max);
			write("hp.clip.y", icon_h - (hp*icon_h)/hp_max);
			write<float>("hp.position.y", hp_ypos + icon_h - (hp*icon_h)/hp_max);
		}
		
		virtual void criarBarco(const string& tbarco, barcotype barco_t, bool debitar = true) {
			if (paused) return;
			// soh cria se eh pra debitar e tem dinheiro OU se nao eh pra debitar
			if (((debitar) && (cash >= custo_barco[barco_t])) || (!debitar)) {
				// cria e poe na lista
				component::base* barco = spawn(tbarco)->component("unit");
				
				// posicao inicial
				barco->write<float>("x", spawn_x);
				barco->write<float>("y", spawn_y);
				
				// cria as surfaces dos barcos de acordo com o diretorio do porto e tambem a direcao padrao
				string path = read<string>("images.path") + tbarco;
				string imagens = string(" barco=") + path + ".png";
				imagens += string(" barcoinv=") + path + "_inv.png";
				barco->add<string>("renderer.surfaces", imagens);
				if (read<string>("images.direction") == "default") {
					barco->write<bool>("barco.render", true);
					barco->write<bool>("barcoinv.render", false);
				}
				else {
					barco->write<bool>("barco.render", false);
					barco->write<bool>("barcoinv.render", true);
				}
				
				barco->write("porto", this);
				
				hook(barco, "done", (component::call)&porto::handleBarcoDone);
				
				barcos.push_back(barco);
				
				// incrementa os contadores
				qtde_barcos[barco_t]++;
				switch (barco_t) {
					case big:		add<int>("grandefabricado", 1);		break;
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
			}
		}

		void updateCashText(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			if (paused) return;
			stringstream ss;
			ss << cash;
			write("cash.text", ss.str());
		}
		
		void getTurnCash() {
			if (paused) return;
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
			if (paused) return;
			stringstream ss;
			ss << "HP: ";
			ss << hp;
			write("hp.text", ss.str());
		}
		
		void handleBarcoDone(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			modinfo("nw-porto");
			if (paused) return;
			if (lastwrite->read<component::base*>("done") == 0 || lastwrite == this) return;
			barcos_prontos++;
			trace("Novo barco pronto");
			
			if (barcos_prontos >= barcos.size()) {
				trace("Todos os", barcos_prontos, "barcos estao prontos do porto", this->owner->name(), "estao prontos!");
				barcos_prontos = 0;
				/* escreve done em si mesmo */
				write("done", this);
			}
		}
		
		void handlePainel(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			if (paused) return;
			criarBarco(read<string>("spawn.tamanho"), read<barcotype>("spawn.tipo"));
		}
		
		void removeBarco(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			if (paused) return;
			// remove da lista
			component::base* barco = read<component::base*>("barcomorrendo");
			barcos.remove(barco);
			
			unhook(barco, "done");
			
			// decrementa os contadores
			int tipo = barco->read<barcotype>("tipo");
			--qtde_barcos[tipo];
			switch (tipo) {
				case big:		add<int>("grandedestruido", 1);		break;
				case medium:	add<int>("mediodestruido", 1);		break;
				case small:		add<int>("pequenodestruido", 1);	break;
				default:
					break;
			}
		}
		
		void handleGameSetup(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			if (!read<bool>("gamesetup")) {
				if (animation) {
					animation->destroy();
					animation = 0;
				}
				return;
			}
			
			animation = spawn("abre-turno")->component("renderer");
			hook(animation, "morri", (component::call)&porto::handleAnimationMorta);
			animation->write("fade.x.speed", read<float>("animacao.x.speed"));
			animation->write("fade.y.speed", read<float>("animacao.y.speed"));
			stringstream ss;
			ss << "Turno do Player ";
			ss << player;
			animation->write<string>("msg.text", ss.str());
		}
		
		void handleGamePlay(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			if (animation) {
				animation->destroy();
				animation = 0;
			}
		}
		
		void handleAnimationMorta(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			animation = 0;
		}
		
	private:
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

bool porto::initialized = false;
int porto::custo_barco[lastsize];
int porto::min_cash_turn;
int porto::max_cash_turn;
int porto::cash_max_barcos[lastsize];

// the build function
extern "C" { component::base * build() { return new porto(); } }
