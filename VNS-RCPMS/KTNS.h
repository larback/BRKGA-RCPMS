#ifndef KTNS_H
#define KTNS_H

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;
extern unsigned t;
extern unsigned n; // tarefas
extern unsigned tempoTroca;
extern int setupTime;
extern std::vector<unsigned>tMoldes;
extern std::vector<unsigned>tProcessamento;

long KTNS(const vector<int>processos, bool debug=false) {
	extern std::vector<std::vector<int>> matrixFerramentas;
	extern unsigned t; // ferramentas
	extern int c; // capacidade do magazine
	vector<int> carregadas(t,0);
	int u=0; // ferramentas no magazine
	int prioridades[t][processos.size()];
	int magazine[t][processos.size()];

	if (debug) {
	std::cout << std::endl << "Matriz de Ferramentas no KTNS" << std::endl;
			for (unsigned j = 0; j<t; j++){
				for (unsigned i = 0; i<8; ++i){
					std::cout<<matrixFerramentas[j][i] << " ";
				}
				std::cout<<std::endl;
			}
			std::cout << " --------------------- " <<std::endl;
	std::cout << "Processos" << std::endl;
	for (unsigned i =0; i<processos.size(); i++) {
		std::cout<<processos[i] << " ";
	}
	std::cout << endl;
	std::cout << endl;
	}

	for (unsigned j=0; j<t; j++) {
		carregadas[j]=matrixFerramentas[j][processos[0]];
		if (matrixFerramentas[j][processos[0]]==1)
			++u;

		for (unsigned i =0; i<processos.size(); i++) {
				magazine[j][i] = matrixFerramentas[j][processos[i]];
				if (debug) {
					cout << magazine[j][i] << " ";
				}
			}
			if (debug) {
			 cout << endl;
			}
	}
	// Preenche a matriz de prioridades
	for (unsigned i=0; i<t; ++i){
		for (unsigned j=0; j < processos.size(); ++j){
			if (magazine[i][j]==1)
				prioridades[i][j] = 0;
			else {
				int proxima = 0;
				bool usa = false;
				for (unsigned k=j+1;k<processos.size();++k){
					++proxima;
					if (magazine[i][k]==1){
						usa = true;
						break;
					}
				}
				if (usa)
					prioridades[i][j]=proxima;
				else
					prioridades[i][j]=-1;
			}
		}
	}
	if (debug) {

	for (unsigned j=0; j<t; j++) {
		for (unsigned i =0; i<processos.size(); i++) {
				cout << prioridades[j][i] << " ";
			}
			cout << endl;
	}

	cout << "Ferramentas carregadas: " << endl;
	for (unsigned j=0; j<t; j++) {
		if (carregadas[j]==33) exit(0);
				cout << carregadas[j] << endl;
	}
	}


	// Calcula as trocas
	if (debug) {
	 cout << u << " carregadas na primeira tarefa" << endl;
	}
	int trocas = 0;
	for (unsigned i=1; i<processos.size(); ++i) {
		for (unsigned j=0; j<t; ++j){
			if ((magazine[j][i]==1) && (carregadas[j]==0)){
				carregadas[j]=1;
				++u;
			}
		}
		if (debug) {
			cout << u << " Ferramentas carregadas" << endl;
		}
		while (u>c){
			int maior = 0;
			int pMaior = -1;
			for (unsigned j=0; j<t; ++j) {
				if (magazine[j][i]!=1){ // Ferramenta não utilizada pelo processo atual
					if ((carregadas[j]==1) && (prioridades[j][i] == -1)) { // Essa ferramenta não será mais utilizada e é um excelente candidato a remoção
						pMaior = j;
						break;
					} else {
						if ((prioridades[j][i]>maior) && carregadas[j]==1) {
							maior = prioridades[j][i];
							pMaior = j;
						}
					}
				}
			}
			carregadas[pMaior] = 0;
			if (debug) {
				cout << "Retirou " << i << ":" << pMaior << endl;
			}
			--u;
			++trocas;
			if (debug) {
				cout << trocas << " trocas " << endl;
			}
		}
		if (debug) {

		cout << "Ferramentas carregadas: " << endl;
		for (unsigned j=0; j<t; j++) {
				cout << carregadas[j] << endl;
		}
	}
	}
	if (debug) {
	 cout << ": " << trocas << "trocas" << endl;
	}
	return trocas;
}
double completionTime(std::vector<unsigned> tProcessamento, std::vector<int >& tarefas){
	extern unsigned tempoTroca;
	double tPr = 0;
	for (std::vector<int>::const_iterator i = tarefas.begin(); i!=tarefas.end(); ++i)
		tPr+= tProcessamento[*i];
	long nTrocas = KTNS(tarefas);
	double tTrocas = nTrocas*tempoTroca;
	return (tPr+tTrocas);
}

double avaliacaoTotal(std::vector < std::vector <int> >& d_maquinas, std::vector < std::pair <double,int> >& d_idx_maquinas, std::vector<unsigned> tProcessamento){
      // Correcao do completion time considerando os intervalos necessários para tornar a solução viável
      // ATENCAO:
      // Esta versão desconsidera as trocas. Caso acrescentemos o setupTime time, será necessário somar as trocas de moldes.
      
      // New vars
      std::vector < std::pair <double,int> > TAmaquina; // Tempo de processamento atual < tempo, maquina>
      std::vector < int > liberaMolde; // Tempo no qual o molde está liberado
      std::vector < int > tarefaMaquina; // Quantas tarefas já foram alocadas à cada máquina
	  std::vector < int > moldeMaquina_i; // Último molde utilizado pela máquina i
      std::vector<std::pair<double , int>>::iterator TAm;
      int mnJob;
      extern int mnPenality;
	  extern int setupTime;
      
      // Todas as máquinas disponíveis no tempo zero e sem tarefas
	 
      
      for (int i=0; i<d_idx_maquinas.size(); ++i){
        TAmaquina.push_back(std::pair<double,int>(0,i)); 
        tarefaMaquina.push_back(0);
		moldeMaquina_i.push_back(-1); // nenhum molde utilizado até então
      }
      // Todos os moldes liberados no tempo zero;
      for (int i=0; i<t; ++i)
        liberaMolde.push_back(0);

      // Colocando os gaps
      for (int i=0; i<n; ++i){
        sort(TAmaquina.begin(), TAmaquina.end());
        TAm = TAmaquina.begin();
        if (tarefaMaquina[TAm->second] < d_maquinas[TAm->second].size()){
          // Seleciono a tarefa que deve ser alocada
          mnJob = d_maquinas[TAm->second][tarefaMaquina[TAm->second]];


		  // Se a máquina não tinha molde (a primeira tarefa) ou não houve alteração de molde, desliga-se o setupTime
			if ((moldeMaquina_i[TAm->second]==-1) || (moldeMaquina_i[TAm->second]==tMoldes[mnJob])) 
				setupTime = 0;
			else
				setupTime = 1;

			// Registro o último molde carregado na máquina
			moldeMaquina_i[TAm->second] = tMoldes[mnJob];

          // Confiro se o molde está livre
          if (liberaMolde[tMoldes[mnJob]]<=TAm->first){
            TAm->first += tProcessamento[mnJob]+ (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          } else { // molde ocupado
		  	setupTime = 1; // Depois de toda espera tem setup
            TAm->first = liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob]+ (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          }
          tarefaMaquina[TAm->second]++;
          if (tarefaMaquina[TAm->second] == d_maquinas[TAm->second].size()) {
            // Ja alocou todas as tarefas desta máquina.
            // Atualiza o completion time e penaliza na TAmaquina 
            d_idx_maquinas[TAm->second] = std::pair<double,int>(TAm->first,TAm->second+1);
            TAm->first = mnPenality;
          }
        } 
      }
	  sort(d_idx_maquinas.begin(), d_idx_maquinas.end());
	  TAm = d_idx_maquinas.end()-1;  
      return TAm->first;

}

double avaliaSolucao(vector<double> ch){
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
	}

	nTrocas=KTNS(processos,false);
	tTrocas = tempoTroca*nTrocas;
	if ((tPr+tTrocas)>makespan){
	makespan = tPr+tTrocas;
	}
	// ***************************
	maquinas.push_back(processos);
	idx_maquinas.push_back(std::pair<double,int>((tPr+tTrocas),maquina));

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
	return makespan;
}

#endif
