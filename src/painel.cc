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

class painel : public component::base {
	private:
		gear2d::link<bool> paused;
		bool gamesetup;

	public:
		// constructor and destructor
		painel() {
		}
		virtual ~painel() {
		}
		
		virtual gear2d::component::family family() { return "menu"; }

		virtual gear2d::component::type type() { return "painel"; }

		virtual std::string depends() { return "renderer/renderer mouse/mouse mouseover/mouseover pause/paused"; }

		virtual void setup(object::signature & sig) {
			write<component::base*>("porto", NULL);
			
			// hooka input do mouse para fabricacao de barcos
			hook("mouse.1", (component::call)&painel::checkSpawnRequest);
			
			paused = fetch<bool>("paused");
			
			// hooka no porto para, no handle, hookar o gamesetup
			hook("porto", (component::call)&painel::handlePorto);
		}
		
		virtual void update(timediff dt) {
			if (gamesetup)
				write<int>("painel.render", 1);
			else
				write<int>("painel.render", 0);
		}
		
		virtual void handlePorto(parameterbase::id pid, base* lastwrite, object::id owner) {
			hook(read<component::base*>("porto"), "gamesetup", (component::call)&painel::handleGameSetup);
			gamesetup = read<component::base*>("porto")->read<bool>("gamesetup");
			if (gamesetup)
				write<int>("painel.render", 1);
			else
				write<int>("painel.render", 0);
		}
		
		virtual void handleGameSetup(parameterbase::id pid, base* lastwrite, object::id owner) {
			gamesetup = read<component::base*>("porto")->read<bool>("gamesetup");
		}
		
		void checkSpawnRequest(parameterbase::id pid, component::base * last, object::id owns) {
			// soh spawna se, ao receber click, o ponteiro do mouse estiver encima e o click sendo SOLTO (e nao PRESSIONADO)
			if ( (read<bool>("mouseover")) && (!read<int>("mouse.1")) && (!paused) && (gamesetup) ) {
				float pos_y = read<int>("mouse.y") - read<float>("y");
				float painel_h = read<float>("h");
				string tamanho;
				barcotype tipo;
				
				if (pos_y < painel_h / 3) {
					tamanho = "barcogrande";
					tipo = big;
				} else if (pos_y < 2 * painel_h / 3) {
					tamanho = "barcomedio";
					tipo = medium;
				} else {
					tamanho = "barcopequeno";
					tipo = small;
				}
				
				// precisa escrever no tipo antes, pq o porto hooka o tamanho.
				// tipo e tamanho eh tudo igual, mas tem q ter as duas coisas pq a string eh o nome do objeto (yaml) e
				// o inteiro eh a posicao no vetor do componente do porto pra contagem de barcos de cada tipo
				read<component::base*>("porto")->write<barcotype>("spawn.tipo", tipo);
				read<component::base*>("porto")->write<string>("spawn.tamanho", tamanho);
			}
		}
		
	private:
		// static vars declaration
		
};

// static vars definition


// the build function
extern "C" { component::base * build() { return new painel(); } }
