#include <iostream>
#include <cmath>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

#define RAD			0.01745
#define NUM_EMERGENCIA_MAX 	1000000
#define TAMX			64
#define TAMY			48

struct Ponto
{
	int x, y;
};

class geraIlha : public component::base {
	private:
		typedef int Matriz [TAMX][TAMY];

		Matriz mapa;
		
		int vol;

		gear2d::link<int>	SPR_W;
		gear2d::link<int>	SPR_H;
		gear2d::link<int>	VOL_MAX;
		gear2d::link<int>	VOL_MIN;
		gear2d::link<int>	RAIO_BRUSH;

		Ponto direcoes[8];

	public:
		geraIlha() { }
		virtual ~geraIlha() { }

		virtual gear2d::component::family family() { return "geraIlha"; }

		virtual gear2d::component::type type() { return "geraIlha"; }

		virtual std::string depends() {	return ""; }

		virtual int rng(int max, int min=0)
		{
			return rand()%(max)+min;
		}

		virtual void setup(object::signature & sig)
		{
			init<int>	("spr.w"	, sig["spr.w"]		, 32);
			init<int>	("spr.h"	, sig["spr.h"]		, 32);
			init<int>	("vol.max"	, sig["vol.max"]	, 10);
			init<int>	("vol.min"	, sig["vol.min"]	, 5);
			init<int>	("raiobrush"	, sig["raiobrush"]	, 5);

			SPR_W		= fetch<int>("spr.w");
			SPR_H		= fetch<int>("spr.h");
			VOL_MAX		= fetch<int>("vol.max");
			VOL_MIN		= fetch<int>("vol.min");
			RAIO_BRUSH	= fetch<int>("raiobrush");

			srand(time(NULL));

			for(int j = 0;j<TAMY;++j)
				for(int i = 0;i<TAMX;++i)
					mapa[i][j] = 0;

			vol = rng(VOL_MAX,VOL_MIN);
			for(int i=0;i<vol;++i)
			{
				brushSimetrico(rng(TAMX-2,2),rng(TAMY-2,2),rng(3,1));
			}
			sanityCheck({2,2},{30,22},5);
			mapPrint();
			return;
		}

		virtual void brush(Ponto xy, int i)
		{
			mapa[xy.x][xy.y] = i;
		}

		virtual void brushSimetrico(Ponto xy, int i)
		{
			mapa[xy.x][xy.y] = i;
			mapa[TAMX - xy.x-1][TAMY - xy.y-1] = i;
		}

		virtual void brushSimetrico(int x, int y, int i)
		{
			if((x>0)&&(y<TAMY)&&(x>0)&&(y<TAMX))
				mapa[x][y] = i;
			if((TAMX - x>0)&&(TAMY - y<TAMY)&&(TAMX - x>0)&&(TAMY - y<TAMX))
				mapa[TAMX - x-1][TAMY - y-1] = i;
		}


		virtual Ponto randomDir(Ponto xy)
		{
			Ponto aux=xy;
			int num = rng(7);
			xy.x += direcoes[num].x;
			xy.y += direcoes[num].y;
			if((xy.y>0)&&(xy.y<TAMY)&&(xy.x>0)&&(xy.x<TAMX))
				return xy;
			return aux;
		}

		void circulo(Ponto xy,int raio)
		{
			Ponto aux;
			for(int i=0;i<360;++i)
			{
				aux.x = cos(i*RAD)*raio+xy.x;
				aux.y = sin(i*RAD)*raio+xy.y;
				if((aux.y>0)&&(aux.y<TAMY)&&(aux.x>0)&&(aux.x<TAMX))
					brushSimetrico(aux,0);
			}
		}

		//recebe as coordenadas de dois portos e forca 3 caminhos distintos entre eles
		//stepSize eh a contagem de passos em media para que uma direcao aleatoria seja escolhida
		//no meio do caminho
		virtual bool sanityCheck(Ponto porto1, Ponto porto2, int stepSize)
		{
			Ponto ini, fim;
			int counter	= stepSize; //contador que serah usado para a checagem de passos aleatorios
			int passos 	= 0;		//contador para saber quantos passos foram feitos

			vector<Ponto> pathway1,pathway2;

			//segura a mao de deus e tenha fe pq eu nao vou explicar isso direito
			//caminha ate nao poder mais pelo eixo X primeiro
			ini=porto1;
			fim=porto2;
			do
			{
				if(counter != 0)
				{
					if(ini.x>fim.x)		ini.x--;
					else
					{
						if(ini.x<fim.x)		ini.x++;
						else
						{
							if(ini.y>fim.y)		ini.y--;
							else
								if(ini.y<fim.y)		ini.y++;
						}
					}
				}
				else
				{
					ini=randomDir(ini);
					counter=stepSize;
				}
				pathway1.push_back(ini);
				brushSimetrico(ini,0);
				--counter;
				++passos;
			}while ((ini.x!=fim.x)||(ini.y!=fim.y));
			pathway1.resize(passos);

			//caminha por ambos os eixos, alternando entre X e Y em cada loop.
			passos=0;
			counter=stepSize;
			ini=porto1;
			fim=porto2;
			while((ini.x!=fim.x)||(ini.y!=fim.y))
			{
				if(counter != 0)
				{
					if(ini.x<fim.x)		ini.x++;
					else
						if(ini.x>fim.x)		ini.x--;
				}
				else
				{
					ini=randomDir(ini);
					counter=stepSize;
				}
				pathway2.push_back(ini);
				brushSimetrico(ini,0);
				--counter;
				++passos;

				if(counter != 0)
				{
					if(ini.y<fim.y)		ini.y++;
					else
						if(ini.y>fim.y)		ini.y--;
				}
				else
				{
					ini=randomDir(ini);
					counter=stepSize;
				}
				pathway2.push_back(ini);
				brushSimetrico(ini,0);
				--counter;
				++passos;
			}
			pathway2.resize(passos);

			for(unsigned int i = 0;i<pathway1.size();++i)
				circulo(pathway1[i],RAIO_BRUSH);
			for(unsigned int i = 0;i<pathway2.size();++i)
				circulo(pathway2[i],RAIO_BRUSH);
			return true;
		}

		virtual void mapPrint()
		{
			
			for(int j=0;j<TAMY;++j)
				for(int i=0;i<TAMX;++i)
				{
					component::base* ilha;
					switch(mapa[i][j])
					{
						case 1:
							ilha = spawn("ilha")->component("spatial");
							ilha->write<float>("x", i*SPR_W-32);
							ilha->write<float>("y", j*SPR_H-32);
						break;
						case 2:
							ilha = spawn("ilha2")->component("spatial");
							ilha->write<float>("x", i*SPR_W-16);
							ilha->write<float>("y", j*SPR_H-16);
						break;
						case 3:
							ilha = spawn("ilha3")->component("spatial");
							ilha->write<float>("x", i*SPR_W-64);
							ilha->write<float>("y", j*SPR_H-64);
						break;
					}
				}
		}
		
		virtual void update(timediff dt) 
		{	
			
		}

		virtual void handle(parameterbase::id pid, base* lastwrite, object::id owner) 
		{
			
		}
};

// the build function
extern "C" { component::base * build() { return new geraIlha(); } }
