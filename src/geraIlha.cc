#include <iostream>
#include <cmath>
#include "gear2d.h"
using namespace gear2d;
using namespace std;

#define TAMX 		32
#define TAMY 		32
#define VOL_MAX		512
#define LIM		5
#define NUM_EMERGENCIA_MAX 100000
typedef int Matriz [TAMX][TAMY];

struct Ponto
{
	int x, y;
};

class geraIlha : public component::base {
	private:
		
		Matriz mapa;
		
		int vol;
		
		Ponto direcoes[3];

	public:
		geraIlha() { }
		virtual ~geraIlha() { }

		virtual gear2d::component::family family() { return "unit"; }

		virtual gear2d::component::type type() { return "geraIlha"; }

		virtual std::string depends() {
			return "";
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
			
			vol = rng(VOL_MAX,10);
		}
		virtual int rng(int max, int min=0)
		{
			return rand()%(max)+min;
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
			xy.x =rng(TAMX-LIM,LIM);
			xy.y =rng(TAMY-LIM,LIM);
			ptAux = xy;
			mapa[xy.x][xy.y] = 1;
			mapa[TAMX - xy.x -1][TAMY - xy.y -1] = 1;

			//loop principal, checamos se chegamos ao volume desejado ou se já se foram 10000 repeticoes, pq depois disso nao da mais
			while (k < vol)
			{
				numEm++;
				
				op = rng(4);//se nao, escolhemos uma nova direcao

				ptAux.x += direcoes[op].x;
				ptAux.y += direcoes[op].y;

				if((ptAux.y>LIM)&&(ptAux.y<TAMY-LIM)&&(ptAux.x>LIM)&&(ptAux.x<TAMX-LIM))//estamos nos limites do mapa?
				{
					xy = ptAux;
					if(mapa[xy.x][xy.y]==0)//caso a posicao nao foi ocupada
					{
						mapa[xy.x][xy.y] = 1;
						mapa[TAMX - xy.x -1][TAMY - xy.y -1] = 1;	
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
		
		virtual bool sanityCheck()
		{
			vector<int> pathway1,pathway2,pathway3;
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
