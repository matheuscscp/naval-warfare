#include <iostream>
#include <cmath>
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

class barco : public component::base {
	private:

		float cx, cy;
		float destx, desty;
		float targetx, targety;
		bool selected;
        bool gameplay; //flag para o barco saber se está na etapa de movimentação
        bool gamesetup; //flag para o barco saber se está na etapa de setup

		gear2d::link<float> x, y, w, h;
		gear2d::link<int> mouse1;


		struct atributos
		{
			gear2d::link<string> tipo;		//tipo do barco
			gear2d::link<int>    hp;		//vida
			gear2d::link<int>    range;		//range de ataque
			gear2d::link<int>    moverange;		//range de movimento
			gear2d::link<int>    speed;		//velocidade de movimento
			gear2d::link<int>    dmg;		//dano por segundo
			gear2d::link<int>    loot;		//loot dropado
		}atr;

		component::base * alvoPrincipal;
		component::base * porto;

	public:
		barco() { }
		virtual ~barco() { }

		virtual gear2d::component::family family() { return "unit"; }

		virtual gear2d::component::type type() { return "barco"; }

		virtual std::string depends() {
			return "kinematics/kinematic2d mouse/mouse mouseover/mouseover collider/collider2d";
		}

		virtual void setup(object::signature & sig) {
			barcotype t = eval(sig["barcotype"], small);
			init<int>("hp.value"  , sig["hp.value"]  , 100);
			init<int>("range"     , sig["range"]     , 64);
			init<int>("moverange" , sig["moverange"] , 128);
			init<int>("speed"     , sig["speed"]     , 300);
			init<int>("dmg"       , sig["dmg"]       , 10);
			init<int>("loot.value", sig["loot.value"], 100);

			atr.hp 		= fetch<int>("hp.value");
			atr.range	= fetch<int>("range");
			atr.moverange 	= fetch<int>("moverange");
			atr.speed	= fetch<int>("speed");
			atr.dmg		= fetch<int>("dmg");
			atr.loot	= fetch<int>("loot.value");
			x 		= fetch<float>("x");
			y 		= fetch<float>("y");
			w 		= fetch<float>("w");
			h 		= fetch<float>("h");
			mouse1 		= fetch<int>("mouse.1");

			write<component::base *>("porto", NULL);
			porto = NULL;

			hook("porto");
			hook("mouse.1", (component::call)&barco::handleClick);
			hook("collider.collision",(component::call)&barco::handleCollision);//hookando a colisao
			hook("hp.value", (component::call)&barco::handleLife);//hookando a life
			hook("hp.value", (component::call)&barco::updateHpText);
			hook("key.m"   , (component::call)&barco::handleKill);
			hook("key.n"   , (component::call)&barco::handleKill);
			hook("key.b"   , (component::call)&barco::handleKill);

			write("range.render", false);
			write("target.render", false);

			cx = x + w/2;
			cy = y + h/2;

			destx = cx;
			desty = cy;

			selected = false;
            gameplay = true; //incializando como true para não prejudicar os demais testes
            gamesetup = true;
		}

		virtual void update(timediff dt) {
			cx = x + w/2;
			cy = y + h/2;

	        //verifica se é o turno do barco antes de executar a movimentação
	        if( gameplay && ( destx - cx || desty - cy ) )
	        {
	            write("x.speed", destx - cx);
	            write("y.speed", desty - cy);
	        }

	        if (selected)
	        {
				int mousex = read<int>("mouse.x");
				int mousey = read<int>("mouse.y");

				float dx = mousex - cx;
				float dy = mousey - cy;
				targetx = dx;
				targety = dy;

				float distance = sqrt(dx*dx + dy*dy);

				if(distance > atr.range) {
					targetx = targetx * (atr.range/distance);
					targety = targety * (atr.range/distance);
				}

				write("target.position.x", targetx + w/2 - 8);
				write("target.position.y", targety + h/2 - 8);
			}
			else
			{
				write("target.position.x", destx - cx + w/2 - 8);
				write("target.position.y", desty - cy + h/2 - 8);

				cout << "destx: " << destx << endl;
				cout << "cx: " << cx << endl;
				cout << "destx - cx: " << destx - cx << endl;
				cout << endl;
			}
		}

		virtual void handle(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (pid == "porto") {
				porto = read<component::base *>("porto");
				hook(porto, "gamesetup");
				hook(porto, "gameplay");
			}
		}

		//temporario, remover depois
		virtual void handleKill(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "key.b" || pid == "key.n" || pid == "key.m") {
				if (raw<int>("key.b") == 1 || raw<int>("key.n") == 1 || raw<int>("key.m") == 1){
					cout << "BARCO VIROU LOOT" << endl;
					write("hp.value",0);
				}
			}
		}

		virtual void handleCollision(parameterbase::id pid, base* lastwrite, object::id owner) {
			int dX,dY,dist,distTarget=0;
			bool colisaoAtaque = false;
			bool colisaoPerto = false;
			if (pid == "collider.collision"){
				component::base * target = read<component::base*>(pid);

				colisaoAtaque = sphereCollision(cx,cy,atr.range,target->read<int>("cx"),target->read<int>("cy"),target->read<int>("h"));
				colisaoPerto = sphereCollision(cx,cy,w,target->read<int>("cx"),target->read<int>("cy"),target->read<int>("h"));
				if(colisaoAtaque)
				{
					if(target->read<string>("collider.tag") == "barco")


					if ((target->read<string>("collider.tag") == "portoa")
					||(target->read<string>("collider.tag") == "barco")){
						/*if(alvoPrincipal==NULL)
							alvoPrincipal = target;
						else{
							if(target == alvoPrincipal){
								if(removeHP(target,1))
									alvoPrincipal=NULL;
								//TODO:: colocar um limite de tempo entre os ataques
							}
						}*/
						removeHP(target,atr.dmg);
					}
				}

				if(colisaoPerto)
				{
					if (target->read<string>("collider.tag") == "loot"){
						porto->write("cash.value",porto->read<int>("cash.value")+target->read<int>("value"));
						target->destroy();
					}
					if (target->read<string>("collider.tag") == "barco"){
						/*descomentar para causar dano ao encostar em um barco
						removeHP(target,atr.hp);
						atr.hp = atr.hp-target->read<int>("hp.value")
						write("hp.value",atr.hp-target->read<int>("hp.value"));
						*/
					}
				}
			}
		}

		//retorna true se o alvo morrer(vida <=0)
		bool removeHP(component::base * target,int dmg) {
			int newHp = 0;
			if(target->read<int>("hp.value")>0){
				newHp = target->read<int>("hp.value")-dmg;
				target->write("hp.value",newHp);
				return false;
			}
			return true;
		}

		bool sphereCollision(int ax,int ay, int ar, int bx, int by, int br) {
			int dX   = ax - bx;
			int dY   = ay - by;
			int dR   = br + ar;
			if((dX*dX)+(dY*dY) <= dR*dR)
				return true;
			return false;
		}

		virtual void handleClick(parameterbase::id pid, base* lastwrite, object::id owner) {
			if (mouse1 == 1) {
				if (selected) {
					destx = targetx + cx;
					desty = targety + cy;
					selected = false;
					write("range.render", false);
					//write("target.render", false);
				}
				else {
					if (read<bool>("mouseover")) {
						cout << "clicked over me" << endl;
						selected = true;
						write("range.render", true);
						write("target.render", true);
					}
				}
			}
		}
		virtual void handleLife(parameterbase::id pid, component::base * last, object::id owns) {
			if (pid == "hp.value"){
				if(read<int>("hp.value") <= 0) {
					int barco = read<int>("barcotype");
					int x = read<int>("x"), y = read<int>("y");
					destroy();

					component::base* loot;
					loot = spawn("loot")->component("spatial");
					loot->write("value",read<int>("loot.value"));
					loot->write("x",x);
					loot->write("y",y);
				 }
			}
		}

		void updateHpText(std::string pid, gear2d::component::base * lastwrite, gear2d::object * owner) {
			//stringstream ss;
			//ss << "HP: ";
			//ss << hp;
			//write("hp.text", ss.str());
		}
};

// the build function
extern "C" { component::base * build() { return new barco(); } }
