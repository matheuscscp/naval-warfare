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

class partida : public component::base {
	private:
		// private vars
		bool force_update;
		bool isinit;
		bool gameover; /* game over? */
		bool gameplay; /* gameplay? */
		component::base* pausemenu;
		
		std::list<component::base*> portos; /* todos os portos */
		std::list<component::base*>::iterator portoAtual; /* porto atual do turno */
		unsigned int turnos; /* numero de turnos */
		int portos_prontos;
		
		gear2d::link<bool> paused;
		
		gear2d::link<timediff> gplay_cooldown;
		timediff gplay_cooldown_timer;
		bool stop_gameplay;
		component::base* gplay_animation;
		
	public:
		// constructor and destructor
		partida() :
		isinit(false),
		force_update(false),
		gameover(false),
		gameplay(false),
		portos_prontos(0),
		pausemenu(NULL),
		gplay_cooldown_timer(0.0f),
		stop_gameplay(false),
		gplay_animation(0)
		{
		}
		virtual ~partida() {
// 			while (portos.size()) {
// 				// destroi os portos ainda vivos
// 				if (portos.back())
// 					portos.back()->destroy();
// 				
// 				// remove o ponteiro
// 				portos.pop_back();
// 			}
		}
		
		virtual gear2d::component::family family() { return "gamestate"; }

		virtual gear2d::component::type type() { return "partida"; }

		virtual std::string depends() { return "keyboard/keyboard renderer/renderer pause/paused"; }

		virtual void setup(object::signature & sig) {
			// spawna os portos
			portos.push_back(spawn("porto-p1")->component("porto"));
			portos.push_back(spawn("porto-p2")->component("porto"));
			
			//gerando mapa
			spawn("geraIlha");
			/* primeiro porto eh o porto atual */
			portoAtual = portos.begin();
			(*portoAtual)->write<bool>("gamesetup", true);
			
			paused = fetch<bool>("paused");
			
			// hooka o tab para mostrar dados da partida
			hook("key.tab", (component::call)&partida::handleTab);
			
			// o return passa de turno
			hook("key.return", (component::call)&partida::handleReturn);
			
			// hooka o ESC pra mostrar um menu
			hook("key.escape", (component::call)&partida::handleEscape);
			
			for (std::list<component::base*>::iterator it = portos.begin(); it != portos.end(); ++it) {
				// hooka a flag morto
				hook(*it, "morto");
				
				// hooka os parametros dos portos na apresentacao dos dados da partida
				hook(*it, "cashusado");
				hook(*it, "cashganho");
				hook(*it, "grandefabricado");
				hook(*it, "mediofabricado");
				hook(*it, "pequenofabricado");
				hook(*it, "grandedestruido");
				hook(*it, "mediodestruido");
				hook(*it, "pequenodestruido");
				hook(*it, "done", (component::call)&partida::handlePortoDone);
			}
			
			init<timediff>("gameplay.cooldown", sig["gameplay.cooldown"], 1.0f);
			gplay_cooldown = fetch<timediff>("gameplay.cooldown");
			init<float>("gameplay.animacao.y.speed", sig["gameplay.animacao.y.speed"], 0.0f);
			init<float>("gameplay.animacao.x", sig["gameplay.animacao.x"], 0.0f);
		}
		
		virtual void update(timediff dt) {
			if (!isinit) {
				isinit = true;
				force_update = true;	// forca a inicializacao dos textos
				updateDados();			// inicializa os textos
				updateDados();			// apaga a tela de dados
			}
			
			if (paused) return;
			
			gplay_cooldown_timer += dt;
			
			if ((gplay_cooldown_timer >= gplay_cooldown) && (stop_gameplay)) {
				stop();
				write((*portoAtual)->owner, "gamesetup", true);
				stop_gameplay = false;
				if (gplay_animation) {
					gplay_animation->destroy();
					gplay_animation = 0;
				}
			}
		}
		
		/* quando um porto tah done, dah proximo turno */
		virtual void handlePortoDone(parameterbase::id pid, component::base * last, object::id owns) {
			modinfo("nw-partida-done");
			if (paused) return;
			if (last == this) return;
			component::base * porto = last;
			if (porto->read<component::base *>("done")) {
				trace("Notificacao que o porto", last->owner->name(), "estah done", last->read<component::base *>("done"));
				portos_prontos++;
				trace(portos_prontos, "portos estao prontos!");
				if (portos_prontos >= portos.size()) {
					proximoTurno();
					portos_prontos = 0;
				}
			}
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "menu.trigger") {
				// destroi o pause menu
				string opt = pausemenu->read<string>("menu.focus");
				if (opt == "resumegame") {
					pausemenu->destroy();
					pausemenu = NULL;
					paused = false;
				}
			} else if (paused) {
				return;
			} else if (pid == "morto") {
				/* gato pra mostrar a tela de game-over */
				write<int>("key.tab", 1);
				gameover = true;
				write<int>("gameover.render", 1);
				write<int>("porto1.render", 1);
				write<int>("porto2.render", 1);
				if (last->read<int>("porto.player") == 1) {
					write<string>("gameover.text", "PLAYER 2 WINS");
					write<float>("gameover.font.r", read<float>("porto2.font.r"));
					write<float>("gameover.font.g", read<float>("porto2.font.g"));
					write<float>("gameover.font.b", read<float>("porto2.font.b"));
				}
				else {
					write<string>("gameover.text", "PLAYER 1 WINS");
					write<float>("gameover.font.r", read<float>("porto1.font.r"));
					write<float>("gameover.font.g", read<float>("porto1.font.g"));
					write<float>("gameover.font.b", read<float>("porto1.font.b"));
				}
				
				// atribui NULL ao morto que acabou de morrer
				std::list<component::base*>::iterator it = portos.begin();
				while (*it != last)
					++it;
				
				unhook(*it, "morto");
				unhook(*it, "cashusado");
				unhook(*it, "cashganho");
				unhook(*it, "grandefabricado");
				unhook(*it, "mediofabricado");
				unhook(*it, "pequenofabricado");
				unhook(*it, "grandedestruido");
				unhook(*it, "mediodestruido");
				unhook(*it, "pequenodestruido");
				unhook(*it, "done");
				
				*it = NULL;
			} else {
				// faz com que o update dos textos ocorra quando um parametro dos portos for alterado
				int tab = read<int>("key.tab");
				if (tab == 2) {
					force_update = true;
					updateDados();
				}
			}
		}
		
		/* O RETURN serve pra sinalizar o fim do setup da movimentação */
		virtual void handleReturn(parameterbase::id pid, component::base * last, object::id owns) {
			if (paused) return;
			if (pid != "key.return") return;
			int enter = read<int>("key.return");
			modinfo("nw-partida");
			if (enter == 1 && !gameplay) {
				proximoTurno();
			}
		}
		
		
		/* Calcula de quem eh o proximo turno, seta gameplay pra true se
		 * for a hora do gameplay */
		void proximoTurno() {
			modinfo("nw-partida");
			if (paused) return;
			
			if (gameplay) {
				portoAtual = portos.begin();
				stop_gameplay = true;
			} else {
				write((*portoAtual)->owner, "gamesetup", false);
				portoAtual++;
			}
			
			if (portoAtual == portos.end()) {
				play();
			} else if (!gameplay) {
				trace("Going to next porto");
				write((*portoAtual)->owner, "gamesetup", true);
			}
		}
		
		void stop() {
			modinfo("nw-partida");
			if (paused) return;
			trace("Stopping gameplay!");
			gameplay = false;
			for (std::list<component::base*>::iterator it = portos.begin(); it != portos.end(); ++it) {
				write((*it)->owner, "gameplay", false);
// 				write<component::base*>((*it)->owner, "done", 0);
			}
		}
		
		void play() {
			modinfo("nw-partida");
			if (paused) return;
			trace("Playing to gameplay!");
			gplay_cooldown_timer = 0.0f;
			gameplay = true;
			gplay_animation = spawn("abre-turno")->component("spatial");
			gplay_animation->write("fade.y.speed", read<float>("gameplay.animacao.y.speed"));
			gplay_animation->write<string>("msg.text", "Fire!!!");
			gplay_animation->write("x", read<float>("gameplay.animacao.x"));
			hook(gplay_animation, "morri", (component::call)&partida::handleGPlayAnimation);
			for (std::list<component::base*>::iterator it = portos.begin(); it != portos.end(); ++it) {
				write((*it)->owner, "gameplay", true);
			}
		}
		
		virtual void handleGPlayAnimation(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "morri") {
				gplay_animation = 0;
			}
		}
		
		virtual void handleTab(parameterbase::id pid, component::base * last, object::id owns) {
			if (gameover)
				return;
			
			updateDados();
		}
		
		void updateDados() {
			// mostra (e atualiza) ou esconde os dados da partida se o jogador estiver segurando tab
			int tab = read<int>("key.tab");
			if ( (tab < 2) || (force_update) ) {
				// cancela o update forcado para o proximo handleTab (ocorre apenas quando um parametro de porto eh alterado)
				if (force_update) {
					force_update = false;
					tab = 1;
				}
				
				// esconde ou mostra os dados
				write("stat.render", tab);
				write("porto1.render", tab);
				write("porto2.render", tab);
				
				// atualiza (se for pra mostrar) e esconde ou mostra os dados da partida
				int i = 1;
				for (std::list<component::base*>::iterator it = portos.begin(); it != portos.end(); ++it) {
					// dados de cada porto
					updateParamText(*it, "cashusado", i, "Cash usado", tab);
					updateParamText(*it, "cashganho", i, "Cash ganho", tab);
					updateParamText(*it, "grandefabricado", i, "+ Barcos grandes", tab);
					updateParamText(*it, "mediofabricado", i, "+ Barcos medios", tab);
					updateParamText(*it, "pequenofabricado", i, "+ Barcos pequenos", tab);
					updateParamText(*it, "grandedestruido", i, "- Barcos grandes", tab);
					updateParamText(*it, "mediodestruido", i, "- Barcos medios", tab);
					updateParamText(*it, "pequenodestruido", i, "- Barcos pequenos", tab);
					
					++i;
				}
			}
		}
		
		void updateParamText(component::base* porto, const string& param, int id, const string& param_name, int tab) {
			string paramstr = textSurfaceStr(param, id);
			
			// atualiza o texto, pois eh para mostrar e nao esconder
			if ( (tab) && (porto) ) {
				stringstream ss;
				ss << param_name;
				ss << ": ";
				ss << porto->read<int>(param);
				write(paramstr + ".text", ss.str());
			}
			
			// esconde ou mostra o dado
			write(paramstr + ".render", tab);
		}
		
		string textSurfaceStr(const string& param, int id) {
			stringstream ss;
			ss << param;
			ss << id;
			return ss.str();
		}
		
		// tecla de pausa
		void handleEscape(parameterbase::id pid, component::base * last, object::id owns) {
			if (read<int>("key.escape") == 1) {
				if (!pausemenu) {
					pausemenu = spawn("pausemenu")->component("spatial");
					hook(pausemenu, "menu.trigger");
					paused = true;
				} else {
					pausemenu->destroy();
					pausemenu = NULL;
					paused = false;
				}
			}
		}
		
};

// the build function
extern "C" { component::base * build() { return new partida(); } }
