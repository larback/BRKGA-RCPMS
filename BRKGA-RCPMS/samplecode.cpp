#include <dirent.h>
#include <cstdlib>
#include <string>
#include <fstream>

#include <iostream>
#include <vector>
#include <ctime>
#include <ratio>
#include <chrono>

#include "SampleDecoder.h"
#include "MTRand.h"
#include "BRKGA.h"
#define MAXBIT 40

std::vector < std::bitset <MAXBIT> > bitMatrix(202);


std::vector<std::vector<int>> matrixFerramentas;
unsigned n = 0; // tarefas
int m = 0; // máquinas
unsigned t = 0; // moldes
unsigned c = 0; // tool magazine
unsigned tempoTroca = 0; // tempo de troca entre dois moldes
unsigned contadorUniversal = 0;
unsigned processamento;
unsigned molde;
std::vector<unsigned>tProcessamento; // tempo de processamento das tarefas
std::vector<unsigned>tMoldes; // vetor de moldes para gerar a matriz binaria

//4626
unsigned p = 1000;	// size of population
const double pe = 0.25;		// fraction of population to be the elite-set
const double pm = 0.10;		// fraction of population to be replaced by mutants
const double rhoe = 0.95;	// probability that offspring inherit an allele from elite parent
const unsigned K = 1;		// number of independent populations
const unsigned MAXT = 1;	// number of threads for parallel decoding
unsigned generation = 0;		// current generation
const unsigned X_INTVL = 100;	// exchange best individuals at every 100 generations
const unsigned X_NUMBER = 2;	// exchange top 2 best
const unsigned MAX_GENS = 1000;	// run for 1000 gens
const long unsigned rngSeed = 0;	// seed to the random number generator
MTRand rng(rngSeed);				// initialize the random number generator
const unsigned MAX_VOID = 300; //  gerações sem melhoria permitidos

const unsigned TIME_LIMIT = 3600; // tempo limite para execucao em segundos

unsigned nVoid = 0; // numero de gerações sem melhorias

double tempoBuscas[5]; // IBS, EFB, ONB, SUDECAP, FBI
long execucoesBuscas[5];
long melhoriasBuscas[5];
double mediaMelhorias[5];
double primeiroMakespan;

int mnPenality;
int dominante;

int setupTime; // Liga e desliga o tempo de setup (desligar apenas para a primeira tarefa);

using namespace std::chrono;
high_resolution_clock::time_point tempoGeral1 ;
high_resolution_clock::time_point tempoGeral2;
duration<double> time_span;

int main(int argc, char* argv[]) {

	


	std::string lowerbound = argv[1];
	double limiteInferior = stod(lowerbound);


	// const unsigned n = 100;		// size of chromosomes
  tProcessamento.clear();
  tMoldes.clear();

  matrixFerramentas.clear();
  cin >> n;
  cin >> m;
  cin >> t;
  c = 1;
  //cin >> c;
  cin >> tempoTroca;
  p = 10*n;
	
	// Vetor com o tempo total de cada molde para criação de um lower bound trivial
  	std::vector<long> tempoTotalMoldes(t,0);
	long maiorTempoMolde = 0;

	// Lendo os moldes
	for (unsigned i = 0; i<n; ++i){
		cin >> molde;
		tMoldes.push_back(molde);
	}

	// Lendo os tempos de processamento das tarefas
	for (unsigned i = 0; i<n; ++i){
		cin >> processamento;
		tProcessamento.push_back(processamento);
		tempoTotalMoldes[tMoldes[i]]+=processamento;
		if (tempoTotalMoldes[tMoldes[i]]>maiorTempoMolde)
			maiorTempoMolde = tempoTotalMoldes[tMoldes[i]];
	}

	// Comparo os dois lowerBounds (modelo x maior molde)
	if (limiteInferior<maiorTempoMolde)
		limiteInferior = maiorTempoMolde;

	// gerando a matrix de ferramentas
	for (unsigned j = 0; j<t; j++){
		std::vector<int> tmpF;
		for (unsigned i = 0; i<n; ++i){
			if (tMoldes[i] == j)
				tmpF.push_back(1);
			else
				tmpF.push_back(0);
		}
		matrixFerramentas.push_back(tmpF);
	}
  for (int i=0;i<5;++i){
    tempoBuscas[i]=0;
    execucoesBuscas[i]=0;
    melhoriasBuscas[i]=0;
    mediaMelhorias[i]=0;
  }
	/*
	cout << endl;
	for (unsigned j = 0; j<t; j++){
		for (unsigned i = 0; i<n; ++i){
			cout << matrixFerramentas[j][i] << " ";
		}
		cout << endl;
	}
	cout << n << " " << m;
	*/



	tempoGeral1 = high_resolution_clock::now();

		// Penalidades 
	for(std::vector<unsigned>::const_iterator it = tProcessamento.begin(); it!=tProcessamento.end(); ++it)
        mnPenality+= *it;
    mnPenality*=2;

	SampleDecoder decoder(tProcessamento);			// initialize the decoder
	// initialize the BRKGA-based heuristic
	BRKGA< SampleDecoder, MTRand > algorithm(n, p, pe, pm, rhoe, decoder, rng, K, MAXT, m);
	double teste_makespan = 9999999999;
	double solucaoInicial = -1;
	primeiroMakespan = -1;
	long teste_geracao = 0;
	// cout << "Limite inferior: " << limiteInferior << endl;
	do {
		algorithm.evolve();// evolve the population for one generation
		if((++generation) % X_INTVL == 0) {
			algorithm.exchangeElite(X_NUMBER);	// exchange top individuals
		}
		if (solucaoInicial==-1) solucaoInicial = algorithm.getBestFitness();

		if (algorithm.getBestFitness()  < teste_makespan){
				nVoid = 0;
				teste_geracao = generation;
				teste_makespan = algorithm.getBestFitness();
		} else {
				++nVoid;
		}

		tempoGeral2 = high_resolution_clock::now();
		time_span = duration_cast<duration<double>>(tempoGeral2 - tempoGeral1);

	} while (generation < MAX_GENS && nVoid < MAX_VOID && time_span.count()<TIME_LIMIT &&  algorithm.getBestFitness() > limiteInferior);
	tempoGeral2 = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(tempoGeral2 - tempoGeral1);
  /*
  cout << "\n\nMakespan: " << algorithm.getBestFitness() << "\n\n";

	cout << "Tempo de execução: " << time_span.count() << " segundos \n\n";
  cout <<endl<< "Melhor geração: " << teste_geracao << endl;
  cout << "Melhor solução" << std::endl;
  */
	vector<double>ch =algorithm.getBestChromosome();

		int maquina = -1;
		int at = 0;
		double tPr = 0.0;
		double makespan = 0.0;
		std::vector < std::pair < double, unsigned > > ranking(ch.size());
		for(unsigned i = 0; i < ch.size(); ++i){
			ranking[i]=std::pair<double,unsigned>(ch[i],i);
		}
		std::sort(ranking.begin(), ranking.end());
		std::vector<int> processos;
		int nTrocas = 0;
		long tTrocas = 0;
		// extern unsigned tempoTroca;
    // *************
    std::vector < std::vector <int> > maquinas; // Todas as máquinas
		std::vector < std::pair <double,int> >idx_maquinas; // completionTime, machine
    // *************
		for(std::vector<std::pair<double, unsigned>>::const_iterator i = ranking.begin(); i!=ranking.end(); ++i){
			at = (int) i->first;
			if (maquina==-1){
				maquina = at;
				processos.clear();
			}
			if (maquina==at){
				tPr += tProcessamento[i->second];
				processos.push_back(i->second);
			} else {
				nTrocas=KTNS(processos,false);
				tTrocas = tempoTroca*nTrocas;

				if ((tPr+tTrocas)>makespan){
					makespan = tPr+tTrocas;
				}
        // ***************************
        maquinas.push_back(processos);
        idx_maquinas.push_back(std::pair<double,int>((tPr+tTrocas),maquina));
        // ****************************

    		maquina = at;
				processos.clear();
				tPr = tProcessamento[i->second];
				processos.push_back(i->second);
			}
			 // std::cout << i->first << ":" << i->second << " ";
		}

		nTrocas=KTNS(processos,false);
		tTrocas = tempoTroca*nTrocas;
		if ((tPr+tTrocas)>makespan){
			makespan = tPr+tTrocas;
		}
    // ***************************
    maquinas.push_back(processos);
    idx_maquinas.push_back(std::pair<double,int>((tPr+tTrocas),maquina));


	// Correcao do completion time considerando os intervalos necessários para tornar a solução viável
	// ATENCAO:
	// Esta versão desconsidera as trocas. Caso acrescentemos o setup time, será necessário somar as trocas de moldes.
	
	// New vars
	std::vector < std::pair <double,int> > TAmaquina; // Tempo de processamento atual < tempo, maquina>
	std::vector < int > liberaMolde; // Tempo no qual o molde está liberado
	std::vector < int > tarefaMaquina; // Quantas tarefas já foram alocadas à cada máquina
	std::vector < int > moldeMaquina_i; // Último molde utilizado pela máquina i
	std::vector<std::pair<double , int>>::iterator TAm;

	std::vector < std::vector < int > > detailedResult; // 0 | Tarefa | MOLDE | Inicio | Fim - Cada linha uma máquina
	int mnJob;
	int mnPenality;
	for(std::vector<unsigned>::const_iterator it = tProcessamento.begin(); it!=tProcessamento.end(); ++it)
		mnPenality+= *it;
	mnPenality*=2;

		// Todas as máquinas disponíveis no tempo zero e sem tarefas

	for (int i=0; i<idx_maquinas.size(); ++i){
		TAmaquina.push_back(std::pair<double,int>(0,i)); 
		tarefaMaquina.push_back(0);
		moldeMaquina_i.push_back(-1); // nenhum molde utilizado até então
		detailedResult.push_back(std::vector<int>());
	}
	// Todos os moldes liberados no tempo zero;
	for (int i=0; i<t; ++i)
		liberaMolde.push_back(0);

	// Colocando os gaps
	for (int i=0; i<n; ++i){
		sort(TAmaquina.begin(), TAmaquina.end());
		TAm = TAmaquina.begin();
		if (tarefaMaquina[TAm->second] < maquinas[TAm->second].size()){
			// Seleciono a tarefa que deve ser alocada
			mnJob = maquinas[TAm->second][tarefaMaquina[TAm->second]];

			// Se a máquina não tinha molde (a primeira tarefa) ou não houve alteração de molde, desliga-se o setup
			if ((moldeMaquina_i[TAm->second]==-1) || (moldeMaquina_i[TAm->second]==tMoldes[mnJob])) 
				setupTime = 0;
			else
				setupTime = 1;

			// Registro o último molde carregado na máquina
			moldeMaquina_i[TAm->second] = tMoldes[mnJob];

			// Confiro se o molde está livre
			if (liberaMolde[tMoldes[mnJob]]<=TAm->first){
				detailedResult[TAm->second].push_back(mnJob);
				detailedResult[TAm->second].push_back(tMoldes[mnJob]);
				detailedResult[TAm->second].push_back(TAm->first);
				detailedResult[TAm->second].push_back(TAm->first + tProcessamento[mnJob] + (tempoTroca * setupTime));
				
				TAm->first += tProcessamento[mnJob] + (tempoTroca * setupTime);
				liberaMolde[tMoldes[mnJob]]=TAm->first;
				
			} else {
				// Registra espera
				setupTime = 1; // Depois de toda espera tem setup
				detailedResult[TAm->second].push_back(-1);
				detailedResult[TAm->second].push_back(-1);
				detailedResult[TAm->second].push_back(TAm->first);
				detailedResult[TAm->second].push_back(liberaMolde[tMoldes[mnJob]]);

				// Registra o processamento
				detailedResult[TAm->second].push_back(mnJob);
				detailedResult[TAm->second].push_back(tMoldes[mnJob]);
				detailedResult[TAm->second].push_back(liberaMolde[tMoldes[mnJob]]);
				detailedResult[TAm->second].push_back(liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob] + (tempoTroca * setupTime));
				
				TAm->first = liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob] + (tempoTroca * setupTime);
				liberaMolde[tMoldes[mnJob]]=TAm->first;
			}
			tarefaMaquina[TAm->second]++;
			if (tarefaMaquina[TAm->second] == maquinas[TAm->second].size()) {
				// Ja alocou todas as tarefas desta máquina.
				// Atualiza o completion time e penaliza na TAmaquina 
				idx_maquinas[TAm->second] = std::pair<double,int>(TAm->first,TAm->second+1);
				TAm->first = mnPenality;
			}
		} 
	}
	sort(idx_maquinas.begin(), idx_maquinas.end());
	TAm = idx_maquinas.end()-1;  
	// Se necessario, atualiza o makespan 
	if (TAm->first> makespan)
		makespan = TAm->first;
	// END 	
	dominante = 0;
	if (makespan == maiorTempoMolde)
		dominante = 1;
	if (algorithm.getBestFitness()!=makespan)
		cout << "E";
    cout << n << " " << t << " " <<  m << " " << makespan << " " << time_span.count() << " " << teste_geracao << " "  << " " << solucaoInicial << " " << tempoBuscas[0]/execucoesBuscas[0] << " " << tempoBuscas[1]/execucoesBuscas[1] << " " << tempoBuscas[2]/execucoesBuscas[2] << " " << tempoBuscas[3]/execucoesBuscas[3] << " " << tempoBuscas[4]/execucoesBuscas[4] << " " << mediaMelhorias[0]/melhoriasBuscas[0] << " " << mediaMelhorias[1]/melhoriasBuscas[1] << " " << mediaMelhorias[2]/melhoriasBuscas[2]  << " " << mediaMelhorias[3]/melhoriasBuscas[3] << " " << mediaMelhorias[4]/melhoriasBuscas[4] << " " <<  generation << " " << dominante << " " << limiteInferior << " " << tempoTroca <<endl;
    cout << "Parametros do Algoritmo\n";
  	cout << "Tamanho da população: " << p << "\n";
  	cout << "Fração da população para elite: " << pe << "\n";
  	cout << "Fração da população trocada por mutantes: " << pm << "\n";
  	cout << "Probabilidade de se herdar um alelo da elite: " << rhoe << "\n";
  	cout << "Populações independentes: " << K << "\n";
  	cout << "Threads para decodificacao paralela: " << MAXT << "\n";
  	cout << "Quantidade máxima de gerações: " << MAX_GENS << "\n";
	cout << "Gerações executadas: " << generation << "\n";

  	cout << "\n\nParâmetros da instância\n";
  	cout << "Tarefas: " << n << "\n";
  	cout << "Máquinas: " << m << "\n";
  	cout << "Ferramentas: " << t << "\n";
  	cout << "Capacidade Magazine: " << c << "\n";
  	cout << "Tempo de troca: " << tempoTroca << "\n";


    cout << "\n\nMakespan: " << makespan << "\n\n";
    cout << "Tempo de execução: " << time_span.count() << " segundos \n\n";
    cout <<endl<< "Melhor geração: " << teste_geracao << endl;


    cout << "\nMakespan ao final da 1ª geração: " << solucaoInicial << "\n\n";

    cout <<"\nBuscas Locais\n";
    cout << "IBS" << endl;
    cout << "Tempo médio: " << tempoBuscas[0]/execucoesBuscas[0] << endl;
    cout << "Número melhorias: " << melhoriasBuscas[0] << endl;
    cout << "Melhora média: " << mediaMelhorias[0]/melhoriasBuscas[0] << endl;
    cout << "Execuções: " << execucoesBuscas[0]<<endl;

    cout << "EFB" << endl;
    cout << "Tempo médio: " << tempoBuscas[1]/execucoesBuscas[1] << endl;
    cout << "Número melhorias: " << melhoriasBuscas[1] << endl;
    cout << "Melhora média: " << mediaMelhorias[1]/melhoriasBuscas[1] << endl;
    cout << "Execuções: " << execucoesBuscas[1]<<endl;

    cout << "ONB" << endl;
    cout << "Tempo médio: " << tempoBuscas[2]/execucoesBuscas[2] << endl;
    cout << "Número melhorias: " << melhoriasBuscas[2] << endl;
    cout << "Melhora média: " << mediaMelhorias[2]/melhoriasBuscas[2] << endl;
    cout << "Execuções: " << execucoesBuscas[2]<<endl;

    cout << "SUDECAP" << endl;
    cout << "Tempo médio: " << tempoBuscas[3]/execucoesBuscas[3] << endl;
    cout << "Número melhorias: " << melhoriasBuscas[3] << endl;
    cout << "Melhora média: " << mediaMelhorias[3]/melhoriasBuscas[3] << endl;
    cout << "Execuções: " << execucoesBuscas[3]<<endl;

	cout << "FBI" << endl;
    cout << "Tempo médio: " << tempoBuscas[4]/execucoesBuscas[4] << endl;
    cout << "Número melhorias: " << melhoriasBuscas[4] << endl;
    cout << "Melhora média: " << mediaMelhorias[4]/melhoriasBuscas[4] << endl;
    cout << "Execuções: " << execucoesBuscas[4]<<endl;
    /*
    cout << "\nMelhor solução" << std::endl;
	maquina = 0;
    for (std::vector<std::vector<int>>::const_iterator i=maquinas.begin(); i!=maquinas.end(); ++i){
      std::vector<int>maquinaAux = *i;
      std::cout << "Máquina " << ++maquina << ": ";
      for (unsigned j=0;j<maquinaAux.size();++j)
        std::cout << maquinaAux[j] << " ";
      std::cout << std::endl;
      std::cout << "Trocas: ";
      std::cout << KTNS(maquinaAux) <<std::endl;
      std::cout << "Tempo de processamento: " << completionTime(tProcessamento,maquinaAux) << "\n";
    }
	*/

    // std::cout << "Total de gerações: " << generation << "\n";
    // ******************************
	int idx_aux = 0;
	int idx_aux2 = 0;
	std::cout << "\nEscalonamento detalhado: Tarefa:Molde[inicio, fim] - Esperas por liberação de moldes são identificadas pelo valor -1 " << endl;
	for(std::vector < std::vector < int > >::const_iterator dr = detailedResult.begin(); dr!=detailedResult.end(); ++dr){
		std::vector<int>maquinaAux = *dr;
		std::cout << "Máquina " << ++idx_aux<< ": ";
		while(idx_aux2 < maquinaAux.size()){
			std::cout << maquinaAux[idx_aux2] << ":" << maquinaAux[idx_aux2+1] << "[" << maquinaAux[idx_aux2+2] << " - "  << maquinaAux[idx_aux2+3] << "]  ";
			idx_aux2+=4;
		}
		std::cout << endl;
		idx_aux2 = 0;
		std::cout << endl;
	}

	return 0;
}
