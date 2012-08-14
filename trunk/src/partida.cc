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
		bool force_update;
		
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
			
			// hooka o tab para mostrar dados da partida
			hook("key.tab", (component::call)&partida::handleTab);
			
			// hooka os parametros dos portos na apresentacao dos dados da partida
			for (std::list<component::base*>::iterator it = portos.begin(); it != portos.end(); ++it) {
				hook(*it, "cashusado");
				hook(*it, "cashganho");
				hook(*it, "grandefabricado");
				hook(*it, "mediofabricado");
				hook(*it, "pequenofabricado");
				hook(*it, "grandedestruido");
				hook(*it, "mediodestruido");
				hook(*it, "pequenodestruido");
			}
			
			// flag para atualizar os textos quando os parametros de um porto forem alterados
			force_update = false;
		}
		
		virtual void update(timediff dt) {
			
		}
		
		virtual void handle(parameterbase::id pid, component::base * last, object::id owns) {
			// faz com que o update dos textos ocorra quando um parametro dos portos for alterado
			int tab = read<int>("key.tab");
			if (tab == 2) {
				force_update = true;
				handleTab("", 0, 0);
			}
		}
		
		virtual void handleTab(parameterbase::id pid, component::base * last, object::id owns) {
			// mostra (e atualiza) ou esconde os dados da partida se o jogador estiver segurando tab
			int tab = read<int>("key.tab");
			if ( (tab < 2) || (force_update) ) {
				// cancela o update forcado para o proximo handleTab (ocorre apenas quando um parametro de porto eh alterado)
				if (force_update) {
					force_update = false;
					tab = 1;
				}
				
				write("stat.render", tab);	// esconde ou mostra o fundo dos dados
				
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
			if (tab) {
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
		
	private:
		// static vars declaration
		static std::list<component::base*> portos;
};

// static vars definition
std::list<component::base*> partida::portos;

// the build function
extern "C" { component::base * build() { return new partida(); } }
