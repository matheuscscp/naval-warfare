#include <iostream>
#include <cmath>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

#define TAMX 				32
#define TAMY 				32
#define SPR_W				32
#define SPR_H				32
#define VOL_MAX				512
#define LIM					5
#define RAIO_BRUSH			5
#define RAD					0.01745
#define NUM_EMERGENCIA_MAX 	100000

typedef int Matriz [TAMX][TAMY];

struct Ponto
{
	int x, y;
};

class geraIlha : public component::base {
	private:
		
		Matriz mapa;
		
		int vol;
		
		Ponto direcoes[7];

	public:
		geraIlha() { }
		virtual ~geraIlha() { }

		virtual gear2d::component::family family() { return "unit"; }

		virtual gear2d::component::type type() { return "geraIlha"; }

		virtual std::string depends() {
			return "";
		}

		virtual int rng(int max, int min=0)
		{
			return rand()%(max)+min;
		}

		virtual void setup(object::signature & sig)
		{
			srand(time(NULL));

			for(int j = 0;j<TAMY;++j)
				for(int i = 0;i<TAMX;++i)
					mapa[i][j] = 0;

			direcoes[0].x = 1;	direcoes[0].y = 0;
			direcoes[1].x =-1;	direcoes[1].y = 0;
			direcoes[2].x = 0;	direcoes[2].y =-1;
			direcoes[3].x = 0;	direcoes[3].y = 1;
			direcoes[4].x =-1;	direcoes[4].y =-1;
			direcoes[5].x =-1;	direcoes[5].y = 1;
			direcoes[6].x = 1;	direcoes[6].y =-1;
			direcoes[7].x = 1;	direcoes[7].y = 1;

			vol = rng(VOL_MAX,10);
			cout<<"Gerando mapa:"
			if(generate())
			{
				cout<<"OK"<<endl;
				cout<<"Sanity check:"
				if(sanityCheck())
				{
					cout<<"OK"<<endl;
					cout<<"Posicionando sprites/objetos:"<<endl;
					mapPrint();
				}
				else
					cout<<"Falha"<<endl;		
			}
			else
				cout<<"Falha"<<endl;
		}



		virtual void brush(Ponto xy, int i)
		{
			//							mapa[xy.x][xy.y-1] = i;
			//mapa[xy.x-1][xy.y] = i;
			mapa[xy.x][xy.y] = i;		//mapa[xy.x+1][xy.y] = i;
				//						mapa[xy.x][xy.y+1] = i;
		}

		virtual void brushSimetrico(Ponto xy, int i)
		{
			mapa[xy.x][xy.y] = i;
			mapa[TAMX - xy.x -1][TAMY - xy.y -1] = i;
		}

		virtual bool generate()
		{
			int k, numEm, op;
			Ponto ptAux;
			Ponto xy;
			k = numEm = op = ptAux.x = ptAux.y = xy.x = xy.y =0;

			for(int j = 0;j<TAMY;++j)
				for(int i = 0;i<TAMX;++i)
					mapa[i][j] = 0;

			//achando um ponto inicial, gravando ele na matriz
			xy.x =rng(TAMX,0);
			xy.y =rng(TAMY,0);

			ptAux = xy;
			brushSimetrico(xy,1);

			//loop principal, checamos se chegamos ao volume desejado ou se já se foram 10000 repeticoes, pq depois disso nao da mais
			while (k < vol)
			{
				numEm++;

				op = rng(4);//se nao, escolhemos uma nova direcao

				ptAux.x += direcoes[op].x;
				ptAux.y += direcoes[op].y;

				if((ptAux.y>0)&&(ptAux.y<TAMY)&&(ptAux.x>0)&&(ptAux.x<TAMX))//estamos nos limites do mapa?
				{
					xy = ptAux;
					if(mapa[xy.x][xy.y]==0)//caso a posicao nao foi ocupada
					{
						brushSimetrico(xy,1);
						k++;//aumentar k, ja que ocupou mais um espaco no volume da m
					}
				}
				else//caso nao estamos, voltamos pra posicao inicial
					ptAux =xy;

				if(numEm>NUM_EMERGENCIA_MAX)
					return false;
			}
			return true;
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

		virtual void circulo(Ponto xy,int raio)
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
			int numEm 	= 0;
			vector<Ponto> pathway1,pathway2,pathway3;

			//segura a mao de deus e tenha fe pq eu nao vou explicar isso direito
			
			//caminha ate nao poder mais pelo eixo X primeiro
			//mas gracas a simetria, vai caminhar pelo eixo Y em sentido contrario ao mesmo tempo
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
				++numEm;
			}while ((ini.x!=fim.x)||(ini.y!=fim.y)||(numEm<NUM_EMERGENCIA_MAX));

			if (numEm>NUM_EMERGENCIA_MAX)
				return false;

			pathway1.resize(passos);

			//caminha por ambos os eixos, alternando entre X e Y em cada loop.
			passos=numEm=0;
			counter=stepSize;
			ini=porto1;
			fim=porto2;
			while((ini.x!=fim.x)||(ini.y!=fim.y)||(numEm<NUM_EMERGENCIA_MAX))
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
				pathway3.push_back(ini);
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
				pathway3.push_back(ini);
				brushSimetrico(ini,0);
				--counter;
				++passos;
				++numEm;
			}

			if (numEm>NUM_EMERGENCIA_MAX)
				return false;

			pathway3.resize(passos);

			for(unsigned int i = 0;i<pathway1.size();++i)
				circulo(pathway1[i],RAIO_BRUSH);
			for(unsigned int i = 0;i<pathway2.size();++i)
				circulo(pathway2[i],RAIO_BRUSH);
			for(unsigned int i = 0;i<pathway3.size();++i)
				circulo(pathway3[i],RAIO_BRUSH);

			return true;
		}

		virtual void mapPrint()
		{
			for(int j=0;j<TAMY;++j)
				for(int i=0;i<TAMX;++i)
				{
					if(mapa[i][j]==1)
					{
						component::base* ilha = spawn(ilha)->component("spatial");
						ilha->write<float>("x", i*SPR_W);
						ilha->write<float>("y", j*SPR_H);
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