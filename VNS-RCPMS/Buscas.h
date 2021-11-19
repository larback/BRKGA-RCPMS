#ifndef BUSCAS_H
#define BUSCAS_H

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <string.h>
#include <bitset>
#include <random>
#include <ctime>
#include <ratio>
#include <string>


#include "KTNS.h"
#include "delta_avaliacao.h"
#define MAXBIT 40
extern std::vector < std::bitset <MAXBIT> > bitMatrix;
extern int m;
extern unsigned t;
extern std::vector<unsigned>tMoldes;
extern std::vector<unsigned>tProcessamento;
extern double tempoBuscas[5]; // IBNS, EFB, ONB, SUDECAP, FBI
extern long execucoesBuscas[5];
extern long melhoriasBuscas[5];
extern double mediaMelhorias[5];
extern int mnPenality;
extern unsigned tempoTroca;
extern int setupTime;
std::chrono::high_resolution_clock::time_point tb1;
std::chrono::high_resolution_clock::time_point tb2;
std::chrono::duration<double> time_span_b;
double IBSMakespan, EFBMakespan, ONBMakespan;
using namespace std;

void IBS(std::vector < std::vector <int> >& maquinas, std::vector < std::pair <double,int> >& idx_maquinas, double& makespan, std::vector<unsigned> tProcessamento){
    // maquinas - Todas as Máquinas
    // idx_maquinas - completionTime, idMaquina
    // makespan
    // tProcessamento - tempo de processamento das tarefas (carregado na decodificacao)
    std::vector<std::pair<double, int>>::iterator critica = idx_maquinas.begin();
    std::vector<std::pair<double, int>>::iterator best = idx_maquinas.end() -1;
    bool melhorou = true;
    double c1 = 0;
    double c2 = 0;
    double c3 = 0;
    int menorMolde = -1;
    int menorMoldeValor = 99999; // grande o suficiente para ser menor que qualquer tempo de processamento
    double c4 = makespan;
    std::vector<int>outJobs; // posicoes que serão removidas
    int outProcess = 0; // tempo de processamento das tarefas que serão removidas
    if (idx_maquinas.size() < m)
      melhorou = false;
    std::sort(idx_maquinas.rbegin(), idx_maquinas.rend());
    while (melhorou) {
      menorMolde = -1;
      menorMoldeValor = 99999;
      outProcess = 0;
      int melhorPosicao = 0;
      outJobs.clear();
      critica = idx_maquinas.begin();
      best = idx_maquinas.end() -1;
      std::vector<int> mCritica = maquinas.at(critica->second -1);
      int lastTask = mCritica[mCritica.size() -1];
    
      std::vector<long> tempoTotalMoldes(t,0); // Vetor com o tempo de processamento por molde
      for (int i=0;i<mCritica.size();++i){
        tempoTotalMoldes[tMoldes[mCritica[i]]] += tProcessamento[mCritica[i]];
        if (tempoTotalMoldes[tMoldes[mCritica[i]]]<menorMoldeValor){ // Como só comparo os que acabaram de receber valor, os zeros iniciais não interferem
          menorMoldeValor=tempoTotalMoldes[tMoldes[mCritica[i]]]; 
          menorMolde = tMoldes[mCritica[i]];
        }
      }

      // Depois colocar no loop anterior, carregando todos e depois só seleciona 
      for (int i=0;i<mCritica.size();++i){
        if (tMoldes[mCritica[i]]==menorMolde){
          outJobs.push_back(i);
          outProcess += tProcessamento[mCritica[i]];
        }
      }
 
      // se o tempo de processamento das tarefas + completion time já excede o makespan não ha porque continuar
      if (best->first + outProcess>=makespan)
        return;
      

      std::vector<int> mBest = maquinas.at(best->second -1);
      melhorPosicao = mBest.size()-1;
      for (int i=mBest.size()-1;i>=0;--i){
          if (tMoldes[mBest[i]] == menorMolde){
            melhorPosicao = i;
            break;
          }
      }

      // Nao juntar pois mudaria os indices na critica
      // inserindo
      for (int i=0;i<outJobs.size();++i)
        mBest.insert(mBest.begin()+melhorPosicao, mCritica[outJobs[i]]);
      
      int ajuste = 0;

      // removendo - Toda vez que removemos, os indices maiores que zero devem perder uma unidade.
      for (int i=0;i<outJobs.size();++i){
        outJobs[i]-=ajuste;
        if(outJobs[i]<0)
          outJobs[i]=0;
        ajuste++;
        mCritica.erase(mCritica.begin() + outJobs[i]);
      }


      std::vector<std::vector<int>> maquinasAux;
      std::vector < std::pair <double,int> > idx_maquinasAux;
      maquinasAux = maquinas;
      maquinasAux.at(critica->second -1)=mCritica;
      maquinasAux.at(best->second -1)=mBest;
      idx_maquinasAux = idx_maquinas;

      // Correcao do completion time considerando os intervalos necessários para tornar a solução viável
      // ATENCAO:
      // Esta versão desconsidera as trocas. Caso acrescentemos o setup time, será necessário somar as trocas de moldes.
      
      // New vars
      std::vector < std::pair <double,int> > TAmaquina; // Tempo de processamento atual < tempo, maquina>
      std::vector < int > liberaMolde; // Tempo no qual o molde está liberado
      std::vector < int > tarefaMaquina; // Quantas tarefas já foram alocadas à cada máquina
      std::vector < int > moldeMaquina_i; // Último molde utilizado pela máquina i
      std::vector<std::pair<double , int>>::iterator TAm;
      int mnJob;
   
      // Todas as máquinas disponíveis no tempo zero e sem tarefas
      
      for (int i=0; i<idx_maquinasAux.size(); ++i){
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
        if (tarefaMaquina[TAm->second] < maquinasAux[TAm->second].size()){
          // Seleciono a tarefa que deve ser alocada
          mnJob = maquinasAux[TAm->second][tarefaMaquina[TAm->second]];

          // Se a máquina não tinha molde (a primeira tarefa) ou não houve alteração de molde, desliga-se o setup
          if ((moldeMaquina_i[TAm->second]==-1) || (moldeMaquina_i[TAm->second]==tMoldes[mnJob])) 
            setupTime = 0;
          else
            setupTime = 1;

          // Registro o último molde carregado na máquina
          moldeMaquina_i[TAm->second] = tMoldes[mnJob];

          // Confiro se o molde está livre
          if (liberaMolde[tMoldes[mnJob]]<=TAm->first){
            TAm->first += tProcessamento[mnJob] + (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          } else {
            setupTime = 1; // Depois de toda espera tem setup
            TAm->first = liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob]+ (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          }
          tarefaMaquina[TAm->second]++;
          if (tarefaMaquina[TAm->second] == maquinasAux[TAm->second].size()) {
            // Ja alocou todas as tarefas desta máquina.
            // Atualiza o completion time e penaliza na TAmaquina 
            idx_maquinasAux[TAm->second] = std::pair<double,int>(TAm->first,TAm->second+1);
            TAm->first = mnPenality;
          }
        } 
      }
      sort(idx_maquinasAux.begin(), idx_maquinasAux.end());
      TAm = idx_maquinasAux.end()-1;  
      // Se necessario, atualiza o makespan 
      c3 = TAm->first;


	// END 	

      if (c3<makespan){
        maquinas = maquinasAux;
        idx_maquinas = idx_maquinasAux;
        makespan = c3;
      } else
        melhorou=false;
    }
}

void EFB(std::vector < std::vector <int> >& maquinas, std::vector < std::pair <double,int> >& idx_maquinas, double& makespan, std::vector<unsigned> tProcessamento){
    // maquinas - Todas as Máquinas
    // idx_maquinas - completionTime, idMaquina
    // makespan
    // tProcessamento - tempo de processamento das tarefas (carregado na decodificacao)
    // Objetivo: Trocar o maior bloco da critica com um menor da best
    
    std::vector<std::pair<double, int>>::iterator critica = idx_maquinas.begin();
    std::vector<std::pair<double, int>>::iterator best = idx_maquinas.end() -1;
    bool melhorou = true;
    double c1 = 0;
    double c2 = 0;
    double c3 = 0;
    int maiorMolde = -1;
    int maiorMoldeValor = 0; 
    int menorMolde = -1;
    int menorMoldeValor = 999999; 
    double c4 = makespan;
    std::vector<int>outIJobs1,outIJobs2; // posicoes que serão removidas na critica
    std::vector<int>outJobs1,outJobs2; // tarefas que serão removidas na best
    int outProcess = 0; // tempo de processamento das tarefas que serão removidas na critica
    int outProcess2 = 0; // tempo de processamento das tarefas que serão removidas da best
    if (idx_maquinas.size() < m)
      melhorou = false;
    

    std::sort(idx_maquinas.rbegin(), idx_maquinas.rend());
    while (melhorou) {
      maiorMolde = -1;
      maiorMoldeValor = 0;
      menorMolde = -1;
      menorMoldeValor = 999999;
      outProcess = 0;
      outProcess2 = 0;
      int melhorPosicao = 0;
      outJobs1.clear();
      outJobs2.clear();
      outIJobs1.clear();
      outIJobs2.clear();

      critica = idx_maquinas.begin();
      best = idx_maquinas.end() -1;
      std::vector<int> mCritica = maquinas.at(critica->second -1);
      std::vector<int> mBest = maquinas.at(best->second -1);
    
      std::vector<long> tempoTotalMoldes(t,0); // Vetor com o tempo de processamento por molde
      for (int i=0;i<mCritica.size();++i){
        tempoTotalMoldes[tMoldes[mCritica[i]]] += tProcessamento[mCritica[i]];
        if (tempoTotalMoldes[tMoldes[mCritica[i]]]>maiorMoldeValor){ 
          maiorMoldeValor=tempoTotalMoldes[tMoldes[mCritica[i]]]; 
          maiorMolde = tMoldes[mCritica[i]];
        }
      }

      // Depois colocar no loop anterior, carregando todos por molde e apenas seleciono o escolhido 
      for (int i=0;i<mCritica.size();++i){
        if (tMoldes[mCritica[i]]==maiorMolde){
          outIJobs1.push_back(i);
          outJobs1.push_back(mCritica[i]);
          outProcess += tProcessamento[mCritica[i]];
        }
      }

 
      // se o tempo de processamento das tarefas + completion time já excede o makespan não ha porque continuar
      if (best->first + outProcess>=makespan)
        return;
      
      // Selecionar molde que vai sair da best
       std::vector<long> tempoTotalMoldes2(t,0); // Vetor com o tempo de processamento por molde
      for (int i=0;i<mBest.size();++i){
        tempoTotalMoldes2[tMoldes[mBest[i]]] += tProcessamento[mBest[i]];
        if (tempoTotalMoldes2[tMoldes[mBest[i]]]<menorMoldeValor){ 
          menorMoldeValor=tempoTotalMoldes2[tMoldes[mBest[i]]]; 
          menorMolde = tMoldes[mBest[i]];
        }
      }

      // Depois colocar no loop anterior, carregando todos e depois só seleciona 
      for (int i=0;i<mBest.size();++i){
        if (tMoldes[mBest[i]]==menorMolde){
          outIJobs2.push_back(i);
          outJobs2.push_back(mBest[i]);
          outProcess2 += tProcessamento[mBest[i]];
        }
      }
      if (outProcess2>outProcess) // Saída da best é maior que a da crítica
        return;

      // Fazer as remoções e inserções

      // Remover da Crítica
      int ajuste = 0;
      // removendo da crítica - Toda vez que removemos, os indices maiores que zero devem perder uma unidade.
      for (int i=0;i<outIJobs1.size();++i){
        outIJobs1[i]-=ajuste;
        if(outIJobs1[i]<0)
          outIJobs1[i]=0;
        ajuste++;
        mCritica.erase(mCritica.begin() + outIJobs1[i]);
      }

      // Remover da Best
      ajuste = 0;
      // removendo da best - Toda vez que removemos, os indices maiores que zero devem perder uma unidade.
      for (int i=0;i<outIJobs2.size();++i){
        outIJobs2[i]-=ajuste;
        if(outIJobs2[i]<0)
          outIJobs2[i]=0;
        ajuste++;
        mBest.erase(mBest.begin() + outIJobs2[i]);
      }


      // Incluir as tarefas em bloco.

      // Incluindo na Best - maiorMolde é o molde da crítica, procuro por ele
      melhorPosicao = 0;
      for (int i=mBest.size()-1;i>=0;--i){
          if (tMoldes[mBest[i]] == maiorMolde){
            melhorPosicao = i;
            break;
          }
      }
      // inserindo
      for (int i=0;i<outJobs1.size();++i)
        mBest.insert(mBest.begin()+melhorPosicao, outJobs1[i]);
 

      // Incluindo na Crítica - menorMolde é o molde da best, procuro por ele  
      melhorPosicao = 0;
      for (int i=mCritica.size()-1;i>=0;--i){
          if (tMoldes[mCritica[i]] == menorMolde){
            melhorPosicao = i;
            break;
          }
      }
      // inserindo
      for (int i=0;i<outJobs2.size();++i)
        mCritica.insert(mCritica.begin()+melhorPosicao, outJobs2[i]);
      


      std::vector<std::vector<int>> maquinasAux;
      std::vector < std::pair <double,int> > idx_maquinasAux;
      maquinasAux = maquinas;
      maquinasAux.at(critica->second -1)=mCritica;
      maquinasAux.at(best->second -1)=mBest;
      idx_maquinasAux = idx_maquinas;

      // Correcao do completion time considerando os intervalos necessários para tornar a solução viável
      // ATENCAO:
      // Esta versão desconsidera as trocas. Caso acrescentemos o setup time, será necessário somar as trocas de moldes.
      
      // New vars
      std::vector < std::pair <double,int> > TAmaquina; // Tempo de processamento atual < tempo, maquina>
      std::vector < int > liberaMolde; // Tempo no qual o molde está liberado
      std::vector < int > tarefaMaquina; // Quantas tarefas já foram alocadas à cada máquina
      std::vector < int > moldeMaquina_i; // Último molde utilizado pela máquina i
      std::vector<std::pair<double , int>>::iterator TAm;
      int mnJob;
   
      // Todas as máquinas disponíveis no tempo zero e sem tarefas
      
      for (int i=0; i<idx_maquinasAux.size(); ++i){
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
        if (tarefaMaquina[TAm->second] < maquinasAux[TAm->second].size()){
          // Seleciono a tarefa que deve ser alocada
          mnJob = maquinasAux[TAm->second][tarefaMaquina[TAm->second]];

          // Se a máquina não tinha molde (a primeira tarefa) ou não houve alteração de molde, desliga-se o setup
          if ((moldeMaquina_i[TAm->second]==-1) || (moldeMaquina_i[TAm->second]==tMoldes[mnJob])) 
            setupTime = 0;
          else
            setupTime = 1;

          // Registro o último molde carregado na máquina
          moldeMaquina_i[TAm->second] = tMoldes[mnJob];

          // Confiro se o molde está livre
          if (liberaMolde[tMoldes[mnJob]]<=TAm->first){
            TAm->first += tProcessamento[mnJob] + (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          } else {
            setupTime = 1; // Depois de toda espera tem setup
            TAm->first = liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob]+ (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          }
          tarefaMaquina[TAm->second]++;
          if (tarefaMaquina[TAm->second] == maquinasAux[TAm->second].size()) {
            // Ja alocou todas as tarefas desta máquina.
            // Atualiza o completion time e penaliza na TAmaquina 
            idx_maquinasAux[TAm->second] = std::pair<double,int>(TAm->first,TAm->second+1);
            TAm->first = mnPenality;
          }
        } 
      }
      sort(idx_maquinasAux.begin(), idx_maquinasAux.end());
      TAm = idx_maquinasAux.end()-1;  
      // Se necessario, atualiza o makespan 
      c3 = TAm->first;


	// END 	

      if (c3<makespan){
        maquinas = maquinasAux;
        idx_maquinas = idx_maquinasAux;
        makespan = c3;
      } else
        melhorou=false;
    }
}


void ONB(std::vector < std::vector <int> >& maquinas, std::vector < std::pair <double,int> >& idx_maquinas, double& makespan, std::vector<unsigned> tProcessamento){
    // maquinas - Todas as Máquinas
    // idx_maquinas - completionTime, idMaquina
    // makespan
    // tProcessamento - tempo de processamento das tarefas (carregado na decodificacao)
    extern std::vector<std::vector<int>> matrixFerramentas;
    extern unsigned n; // tarefas
    std::vector<std::pair<double, int>>::iterator critica;
    std::pair<int, int>ONB1, ONB2;
    int deltad, deltae;
    double c1,c2;




    std::vector < std::pair <double,int> > TAmaquina; // Tempo de processamento atual < tempo, maquina>
    std::vector < int > liberaMolde; // Tempo no qual o molde está liberado
    std::vector < int > tarefaMaquina; // Quantas tarefas já foram alocadas à cada máquina
    std::vector < int > moldeMaquina_i; // Último molde utilizado pela máquina i
    std::vector<std::pair<double , int>>::iterator TAm;

    std::vector<std::vector<int>> maquinasAux;
    std::vector < std::pair <double,int> > idx_maquinasAux, idx_maquinasAux2;


    if (idx_maquinas.size() < m)
      return ;
    bool melhorou = true;
    double cTimeCritica = 0;
    while (melhorou){
        melhorou = false;
        std::sort(idx_maquinas.rbegin(), idx_maquinas.rend());
        critica = idx_maquinas.begin();
        cTimeCritica = critica->first;
        makespan = cTimeCritica;
        std::vector<int> mCritica = maquinas.at(critica->second -1);
        std::vector<int> mAux1,mAux2;
        if (KTNS(mCritica)==0)
            return ;
        for (unsigned i =0; i<=n+2;++i) bitMatrix[i].reset();

    	  for (unsigned j = 0; j<t; ++j){
    		    for (unsigned i = 0; i<mCritica.size(); ++i){
    			       if
                 (matrixFerramentas[j][mCritica[i]]==1){
    				           bitMatrix[i+1].set(t-1-j);
                  }
    		    }
        }
 
        std::vector<int> linhas;
        for(int i = 0; i<t; ++i)
          linhas.push_back(i);
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        shuffle (linhas.begin(), linhas.end(), std::default_random_engine(seed));

        for (vector<int>::const_iterator i = linhas.begin(); i!= linhas.end(); ++i){
          ONB1 = std::make_pair(-1,-1);
          ONB2 = std::make_pair(-1,-1);
          for (unsigned j=0; j<mCritica.size();++j){
            // Pega o ponto inicial e final do primeiro e do segundo one block
            if (matrixFerramentas[*i][mCritica[j]]==1){
              if (ONB1.first == -1){
                ONB1.first = j;
                while (j < mCritica.size() && matrixFerramentas[*i][mCritica[j]]==1) ++j;
                ONB1.second = j-1;
              } else {
                if (ONB2.first == -1){
                  ONB2.first = j;
                  while (j < mCritica.size() && matrixFerramentas[*i][mCritica[j]]==1) ++j;
                  ONB2.second = j-1;
                }
              }
              if (ONB2.first!=-1){
                int nMovimentos = ONB1.first - ONB1.second +1;
                int pivo = ONB1.first;
                int TPivo = 0;
                mAux1 = mCritica;
                mAux2 = mCritica;
                for (int p=0;p<nMovimentos;++p){
                  //delta avalicao indice la começa em 1
                  deltae = deltaShift(pivo+1,ONB2.first+1);
                  deltad = deltaShift(pivo+1,ONB2.second+2);
                  if (deltae<=0 || deltad <=0){
                    // Insiro a esquerda do 2 ONB
                    TPivo=mAux1[pivo];
                    for (int pe=pivo;pe<ONB2.first-1;++pe)
                      mAux1[pe]=mAux1[pe+1];
                    mAux1[ONB2.first-1] = TPivo;

                    // ******************************************
                    // Correcao do completion time do primeiro movimento considerando os intervalos necessários para tornar a solução viável
                    // ATENCAO:
                    // Esta versão desconsidera as trocas. Caso acrescentemos o setup time, será necessário somar as trocas de moldes.
                    
                    // New vars
                    maquinasAux.clear();
                    idx_maquinasAux.clear();
                    
                    maquinasAux = maquinas;
                    maquinasAux.at(critica->second -1)=mAux1;
                    idx_maquinasAux = idx_maquinas;


                    int mnJob = 0;
                    int mnPenality = 0;
                    liberaMolde.clear();
                    for(std::vector<unsigned>::const_iterator it = tProcessamento.begin(); it!=tProcessamento.end(); ++it)
                      mnPenality+= *it;
                    mnPenality*=2;

                    // Todas as máquinas disponíveis no tempo zero e sem tarefas
                    TAmaquina.clear();
                    tarefaMaquina.clear();
                    moldeMaquina_i.clear();
                    for (int ki=0; ki<idx_maquinasAux.size(); ++ki){
                      TAmaquina.push_back(std::pair<double,int>(0,ki)); 
                      tarefaMaquina.push_back(0);
                      moldeMaquina_i.push_back(-1); // nenhum molde utilizado até então
                    }
                    // Todos os moldes liberados no tempo zero;

                    for (int ki=0; ki<t; ++ki)
                      liberaMolde.push_back(0);

                    // Colocando os gaps
                    for (int ki=0; ki<n; ++ki){
                      sort(TAmaquina.begin(), TAmaquina.end());
                      TAm = TAmaquina.begin();
                      if (tarefaMaquina[TAm->second] < maquinasAux[TAm->second].size()){
                        // Seleciono a tarefa que deve ser alocada
                        mnJob = maquinasAux[TAm->second][tarefaMaquina[TAm->second]];

                        // Se a máquina não tinha molde (a primeira tarefa) ou não houve alteração de molde, desliga-se o setup
                        if ((moldeMaquina_i[TAm->second]==-1) || (moldeMaquina_i[TAm->second]==tMoldes[mnJob])) 
                          setupTime = 0;
                        else
                          setupTime = 1;

                        // Registro o último molde carregado na máquina
                        moldeMaquina_i[TAm->second] = tMoldes[mnJob];

                        
                        // Confiro se o molde está livre
                        if (liberaMolde[tMoldes[mnJob]]<=TAm->first){
                          TAm->first += tProcessamento[mnJob] + (tempoTroca * setupTime);
                          liberaMolde[tMoldes[mnJob]]=TAm->first;
                        } else {
                          setupTime = 1; // Depois de toda espera tem setup
                          TAm->first = liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob]+ (tempoTroca * setupTime);
                          liberaMolde[tMoldes[mnJob]]=TAm->first;
                        }
                        tarefaMaquina[TAm->second]++;
                        if (tarefaMaquina[TAm->second] == maquinasAux[TAm->second].size()) {
                          // Ja alocou todas as tarefas desta máquina.
                          // Atualiza o completion time e penaliza na TAmaquina 
                          idx_maquinasAux[TAm->second] = std::pair<double,int>(TAm->first,TAm->second+1);
                          TAm->first = mnPenality;
                        }
                      } 
                    }
                    sort(idx_maquinasAux.begin(), idx_maquinasAux.end());
                    TAm = idx_maquinasAux.end()-1;  
                    c1 = TAm->first;
                    // ********************

                    // c1 = completionTime(tProcessamento,mAux1);

                    //std::cout << "\nDelta Esquerda " << deltae << "\n";
                    // Insiro a direita do 2 ONB
                    TPivo=mAux2[pivo];
                    for (int pd=pivo;pd<ONB2.second;++pd)
                      mAux2[pd]=mAux2[pd+1];
                    mAux2[ONB2.second] = TPivo;

                    // ******************************************
                    // Correcao do completion time do segundo movimento considerando os intervalos necessários para tornar a solução viável
                    // ATENCAO:
                    // Esta versão desconsidera as trocas. Caso acrescentemos o setup time, será necessário somar as trocas de moldes.
                    
                    // New vars
                    // maquinasAux.clear();
                    idx_maquinasAux2.clear();
                    
                    // maquinasAux = maquinas;
                    maquinasAux.at(critica->second -1)=mAux2;
                    idx_maquinasAux2 = idx_maquinas;


                    mnJob = 0;
                    mnPenality = 0;
                    liberaMolde.clear();
                    for(std::vector<unsigned>::const_iterator it = tProcessamento.begin(); it!=tProcessamento.end(); ++it)
                      mnPenality+= *it;
                    mnPenality*=2;

                    // Todas as máquinas disponíveis no tempo zero e sem tarefas
                    TAmaquina.clear();
                    tarefaMaquina.clear();
                    moldeMaquina_i.clear();

                    for (int ki=0; ki<idx_maquinasAux2.size(); ++ki){
                      TAmaquina.push_back(std::pair<double,int>(0,ki)); 
                      tarefaMaquina.push_back(0);
                      moldeMaquina_i.push_back(-1); // nenhum molde utilizado até então
                    }
                    // Todos os moldes liberados no tempo zero;

                    for (int ki=0; ki<t; ++ki)
                      liberaMolde.push_back(0);

                    // Colocando os gaps
                    for (int ki=0; ki<n; ++ki){
                      sort(TAmaquina.begin(), TAmaquina.end());
                      TAm = TAmaquina.begin();
                      if (tarefaMaquina[TAm->second] < maquinasAux[TAm->second].size()){
                        // Seleciono a tarefa que deve ser alocada
                        mnJob = maquinasAux[TAm->second][tarefaMaquina[TAm->second]];
                        // Se a máquina não tinha molde (a primeira tarefa) ou não houve alteração de molde, desliga-se o setup
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
                        } else {
                          setupTime = 1; // Depois de toda espera tem setup
                          TAm->first = liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob]+ (tempoTroca * setupTime);
                          liberaMolde[tMoldes[mnJob]]=TAm->first;
                        }
                        tarefaMaquina[TAm->second]++;
                        if (tarefaMaquina[TAm->second] == maquinasAux[TAm->second].size()) {
                          // Ja alocou todas as tarefas desta máquina.
                          // Atualiza o completion time e penaliza na TAmaquina 
                          idx_maquinasAux2[TAm->second] = std::pair<double,int>(TAm->first,TAm->second+1);
                          TAm->first = mnPenality;
                        }
                      } 
                    }
                    sort(idx_maquinasAux2.begin(), idx_maquinasAux2.end());
                    TAm = idx_maquinasAux2.end()-1;  
                    c2 = TAm->first;

                    // ********************

                    // c2 = completionTime(tProcessamento,mAux2);

                    //std::cout << "\nDelta Direita " << deltad << "\n";
                    if (c1 < cTimeCritica || c2 < cTimeCritica){
                      melhorou = true;
                      /*
                      std::cout << "Melhorou" << "\n deltaE: " << deltae << " deltaD: " << deltad << "\n";
                      if (deltae > 0 && deltad > 0) {
                        std::cout << "wtf" << '\n';
                        for (unsigned i = 0; i<mAux2.size()+2; ++i){
                              std::cout << bitMatrix[i] << '\n';
                        }
                        cout << pivo+1 << " para " << ONB2.first+1 << "\n";
                        cout << pivo+1 << " para " << ONB2.second+2 << "\n";
                      }
                      // *****************************************/
                      if (c1<c2){
                        // Fica à esquerda
                      //  std::cout << "\nFicou a esquerda\n";
                        mCritica = mAux1;
                        ONB2.first = ONB2.first -1;
                        cTimeCritica = c1;
                      } else {
                        // Fica à direita
                      //  std::cout << "\nFicou a direita\n";
                        mCritica = mAux2;
                        cTimeCritica = c2;
                      }
                    }
                  } // fim da delta avaliacao <= 0
                  if (!melhorou)
                    ++pivo; // se tiver melhorado, a tarefa andou, então o pivo fica.
                  mAux1=mCritica;
                  mAux2=mCritica;
                } // Fim dos movimentos ONB1->ONB2
                // Procura-se o proximo ONB
                ONB1.first = ONB2.first;
                ONB1.second = ONB2.second;
                ONB2 = make_pair(-1,-1);
              }
            }
          }
        } // fim das linhas
        if (melhorou){
          critica->first = cTimeCritica;
          maquinas.at(critica->second -1) = mCritica;
          if (c1<c2)
            idx_maquinas = idx_maquinasAux;
          else
            idx_maquinas = idx_maquinasAux2;
          
        }

    } // wend

}


void SUDECAP(std::vector < std::vector <int> >& maquinas, std::vector < std::pair <double,int> >& idx_maquinas, double& makespan, std::vector<unsigned> tProcessamento){
   // cout << "Operacao tapa buraco\n";
    // maquinas - Todas as Máquinas
    // idx_maquinas - completionTime, idMaquina
    // makespan
    // tProcessamento - tempo de processamento das tarefas (carregado na decodificacao)
    std::vector<std::pair<double, int>>::iterator critica ;
    std::vector<std::pair<double, int>>::iterator best;
    vector<vector<int>> intervalos;
    bool melhorou = true;
    double c3 = 0;
    if (idx_maquinas.size() < m)
      melhorou = false;
  
    while (melhorou) {
     
      std::vector<std::vector<int>> maquinasAux;
      std::vector < std::pair <double,int> > idx_maquinasAux;
      maquinasAux = maquinas;
      idx_maquinasAux = idx_maquinas;
      // Correcao do completion time considerando os intervalos necessários para tornar a solução viável
      // ATENCAO:
      // Esta versão desconsidera as trocas. Caso acrescentemos o setup time, será necessário somar as trocas de moldes.
      
      // New vars
      std::vector < std::pair <double,int> > TAmaquina; // Tempo de processamento atual < tempo, maquina>
      std::vector < int > liberaMolde; // Tempo no qual o molde está liberado
      std::vector < int > tarefaMaquina; // Quantas tarefas já foram alocadas à cada máquina
      std::vector < int > moldeMaquina_i; // Último molde utilizado pela máquina i
      std::vector<std::pair<double , int>>::iterator TAm;
      int mnJob;

      // Todas as máquinas disponíveis no tempo zero e sem tarefas
      
      for (int i=0; i<idx_maquinasAux.size(); ++i){
        TAmaquina.push_back(std::pair<double,int>(0,i)); 
        tarefaMaquina.push_back(0);
        intervalos.push_back(vector<int>(0)); // vetor com os intervalos, cada linha uma máquina
        moldeMaquina_i.push_back(-1); // nenhum molde utilizado até então
      }
      // Todos os moldes liberados no tempo zero;
      for (int i=0; i<t; ++i)
        liberaMolde.push_back(0);

      // Colocando os gaps
      for (int i=0; i<n; ++i){
        sort(TAmaquina.begin(), TAmaquina.end());
        TAm = TAmaquina.begin();
        if (tarefaMaquina[TAm->second] < maquinasAux[TAm->second].size()){
          // Seleciono a tarefa que deve ser alocada
          mnJob = maquinasAux[TAm->second][tarefaMaquina[TAm->second]];
          
          // Se a máquina não tinha molde (a primeira tarefa) ou não houve alteração de molde, desliga-se o setup
          if ((moldeMaquina_i[TAm->second]==-1) || (moldeMaquina_i[TAm->second]==tMoldes[mnJob])) 
            setupTime = 0;
          else
            setupTime = 1;

          // Registro o último molde carregado na máquina
          moldeMaquina_i[TAm->second] = tMoldes[mnJob];
          
          // Confiro se o molde está livre
          if (liberaMolde[tMoldes[mnJob]]<=TAm->first){
            TAm->first += tProcessamento[mnJob] + (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          } else { // molde ocupado, registrar gap
            setupTime = 1; // Depois de toda espera tem setup
            intervalos[TAm->second].push_back(tarefaMaquina[TAm->second]); // Indice da tarefa que tem gap antes nesta máquina
            intervalos[TAm->second].push_back(liberaMolde[tMoldes[mnJob]]-TAm->first); // Tamanho do gap
            intervalos[TAm->second].push_back(tMoldes[mnJob]); // molde que causou o gap
            TAm->first = liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob]+ (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          }
          tarefaMaquina[TAm->second]++;
          if (tarefaMaquina[TAm->second] == maquinasAux[TAm->second].size()) {
            // Ja alocou todas as tarefas desta máquina.
            // Atualiza o completion time e penaliza na TAmaquina 
            idx_maquinasAux[TAm->second] = std::pair<double,int>(TAm->first,TAm->second+1);
            TAm->first = mnPenality;
          }
        } 
      }
      sort(idx_maquinasAux.begin(), idx_maquinasAux.end());
      TAm = idx_maquinasAux.end()-1;  
      // Se necessario, atualiza o makespan 
      c3 = TAm->first;


      // cout << "Makespan original " << c3 << endl << endl;


      std::vector<int> mCritica = maquinasAux.at(TAm->second-1);
      int tarefaCandidata = -1;
      int tarefaAntesGap = -1;
      int candidata = -1;
      int pCandidata = 999999999; // pontuacao para eleger o melhor candidata
      bool sairExterno = false;
      int acrescimo = 0;
      for (int i=0;i<intervalos[TAm->second-1].size();i+=3){
	      // cout << "Antes de " << intervalos[TAm->second-1][i] << " gap de " << intervalos[TAm->second-1][i+1] << " molde diferente de " << intervalos[TAm->second-1][i+2] << endl;
        for (int j=intervalos[TAm->second-1][i] + 1; j<mCritica.size(); ++j){
          // cout << "Inserir " << j << " (tarefa "<< mCritica[j] <<") antes de " << intervalos[TAm->second-1][i] << "(tarefa " << mCritica[intervalos[TAm->second-1][i]]<<")" << endl;
          if ( (intervalos[TAm->second-1][i+1] >= tProcessamento[mCritica[j]]) && (tMoldes[mCritica[j]]!=intervalos[TAm->second-1][i+2]) ){
            // Se achei alguém para preencher este gap, sair do loop externo
            sairExterno = true;
            if (tMoldes[mCritica[j]]!=tMoldes[intervalos[TAm->second-1][i]]) // molde diferente liga o setupTime
              setupTime = 1;
            else
              setupTime = 0;
            acrescimo = tempoTroca *setupTime;

            tarefaAntesGap = intervalos[TAm->second-1][i];
            if(( (intervalos[TAm->second-1][i+1] - tProcessamento[mCritica[j]])+acrescimo) < pCandidata){
              candidata = j;
              pCandidata = (intervalos[TAm->second-1][i+1] - tProcessamento[mCritica[j]])+acrescimo;
              if (pCandidata == 0)
                break;
            }
          }
        }
        // cout << "Melhor candidata " << candidata << " pontuacao " << pCandidata << endl;
        // Alterar máquina critica, atualizar lista de máquinas, recalcular gaps e ver se melhorou
        if (sairExterno) break;
      }
      if (sairExterno){ // Temos uma possível troca.
        // cout << "Máquina crítica atual\n";
        // for (int i=0; i<mCritica.size(); ++i)
        //  cout << mCritica[i] << " ";
        // cout << endl;
        tarefaCandidata = mCritica[candidata];
        // cout << "Candidata: " << candidata << " Tarefa Antes do Gap " << tarefaAntesGap << endl;
        for (int i=candidata; i>tarefaAntesGap; --i)
          mCritica[i] = mCritica[i-1];
        mCritica[tarefaAntesGap] = tarefaCandidata;
        // cout << "Máquina crítica alterada\n";
        // for (int i=0; i<mCritica.size(); ++i)
        //  cout << mCritica[i] << " ";
        // cout << "\n <--";
        
      }
      // Atualizar máquinas e recalcular makespan
      maquinasAux.at(TAm->second-1) = mCritica;

    // ######################################
    // New vars
      TAmaquina.clear(); // Tempo de processamento atual < tempo, maquina>
      liberaMolde.clear(); // Tempo no qual o molde está liberado
      tarefaMaquina.clear(); // Quantas tarefas já foram alocadas à cada máquina
      moldeMaquina_i.clear(); // Ultimo molde utilizado na maquina i
      // Todas as máquinas disponíveis no tempo zero e sem tarefas
      
      for (int i=0; i<idx_maquinasAux.size(); ++i){
        TAmaquina.push_back(std::pair<double,int>(0,i)); 
        tarefaMaquina.push_back(0);
        moldeMaquina_i.push_back(-1); // nenhum molde utilizado até então
        intervalos.push_back(vector<int>(0)); // vetor com os intervalos, cada linha uma máquina
      }
      // Todos os moldes liberados no tempo zero;
      for (int i=0; i<t; ++i)
        liberaMolde.push_back(0);

      // Colocando os gaps
      for (int i=0; i<n; ++i){
        sort(TAmaquina.begin(), TAmaquina.end());
        TAm = TAmaquina.begin();
        if (tarefaMaquina[TAm->second] < maquinasAux[TAm->second].size()){
          // Seleciono a tarefa que deve ser alocada
          mnJob = maquinasAux[TAm->second][tarefaMaquina[TAm->second]];
          // Se a máquina não tinha molde (a primeira tarefa) ou não houve alteração de molde, desliga-se o setup
          if ((moldeMaquina_i[TAm->second]==-1) || (moldeMaquina_i[TAm->second]==tMoldes[mnJob])) 
            setupTime = 0;
          else
            setupTime = 1;

          // Registro o último molde carregado na máquina
          moldeMaquina_i[TAm->second] = tMoldes[mnJob];
          
          // Confiro se o molde está livre
          if (liberaMolde[tMoldes[mnJob]]<=TAm->first){
            TAm->first += tProcessamento[mnJob] + (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          } else { // molde ocupado, registrar gap
            setupTime = 1; // Depois de toda espera tem setup
            intervalos[TAm->second].push_back(tarefaMaquina[TAm->second]); // Indice da tarefa que tem gap antes nesta máquina
            intervalos[TAm->second].push_back(liberaMolde[tMoldes[mnJob]]-TAm->first); // Tamanho do gap
            intervalos[TAm->second].push_back(tMoldes[mnJob]); // molde que causou o gap
            TAm->first = liberaMolde[tMoldes[mnJob]] + tProcessamento[mnJob] + (tempoTroca * setupTime);
            liberaMolde[tMoldes[mnJob]]=TAm->first;
          }
          tarefaMaquina[TAm->second]++;
          if (tarefaMaquina[TAm->second] == maquinasAux[TAm->second].size()) {
            // Ja alocou todas as tarefas desta máquina.
            // Atualiza o completion time e penaliza na TAmaquina 
            idx_maquinasAux[TAm->second] = std::pair<double,int>(TAm->first,TAm->second+1);
            TAm->first = mnPenality;
          }
        } 
      }
      sort(idx_maquinasAux.begin(), idx_maquinasAux.end());
      TAm = idx_maquinasAux.end()-1;  
      // Se necessario, atualiza o makespan 
      c3 = TAm->first;

      // cout << "Makespan final " << c3 << endl << endl;
    // ######################################
	// END 	

      if (c3<makespan){
        // cout << "********************************** MELHOROU ************************";
        maquinas = maquinasAux;
        idx_maquinas = idx_maquinasAux;
        makespan = c3;
      } else
        melhorou=false;
    }
}




void FBI(std::vector < std::vector <int> >& maquinas, std::vector < std::pair <double,int> >& idx_maquinas, double& makespan, std::vector<unsigned> tProcessamento){
  bool melhorou = true;
  std::vector < std::vector <int> > FBI_maquinas, RFBI_maquinas;
  std::vector < std::pair <double,int> >FBI_idx_maquinas;
  std::vector<std::pair<double , int>>::iterator FBI_idx;

  // std::vector < int > tarefaMaquina; // Quantas tarefas já foram alocadas à cada máquina
  double FBI_makespan = 0;
  while(melhorou){
    melhorou = false;
    FBI_maquinas.clear();
    FBI_idx_maquinas.clear();
    RFBI_maquinas.clear();
    // RODA A BUSCA LOCAL
    FBI_maquinas = maquinas;
    FBI_idx_maquinas = idx_maquinas;

    std::vector<int> myvector;
    // myvector.push_back(0);
    for (int i=0;i<FBI_maquinas.size();++i){
      RFBI_maquinas.push_back(myvector);
    }
 
   
    for (int i=0;i<n;++i){
      sort(FBI_idx_maquinas.rbegin(), FBI_idx_maquinas.rend());
      FBI_idx = FBI_idx_maquinas.begin();
      // cout << "Máquina: " << FBI_idx->second-1 << "\n";
      // cout << "Completion Time: " << FBI_idx->first << "\n";

      RFBI_maquinas[FBI_idx->second-1].push_back(FBI_maquinas[FBI_idx->second-1].back());
      FBI_maquinas[FBI_idx->second-1].pop_back();
  
      if (FBI_maquinas[FBI_idx->second-1].size() == 0){
        FBI_idx->first = -1*mnPenality;
      } else{
        FBI_idx->first -= tProcessamento[RFBI_maquinas[FBI_idx->second-1].front()];
      }
      // cout << endl << endl;
    }

    // **********

    // cout << "Makespan: " << makespan << endl; 
    
    FBI_makespan = avaliacaoTotal(RFBI_maquinas, FBI_idx_maquinas, tProcessamento);

    // cout << "FBI Makespan: " << FBI_makespan << endl;
    /*
    maquina = 0;
    for (std::vector<std::vector<int>>::const_iterator i=RFBI_maquinas.begin(); i!=RFBI_maquinas.end(); ++i){
      std::vector<int>maquinaAux = *i;
      std::cout << "Máquina " << ++maquina << ": ";
      for (unsigned j=0;j<maquinaAux.size();++j)
        std::cout << maquinaAux[j] << " ";
    }
    cout << endl;
    */

    if (FBI_makespan < makespan){
      melhorou = true;
      makespan = FBI_makespan;
      maquinas = RFBI_maquinas;
      idx_maquinas = FBI_idx_maquinas;
    }
  }     
}



double buscas(std::vector< double >& chromosome, int vezes = 1){
  std::vector < std::vector <int> > maquinas; // Todas as máquinas
  std::vector < std::pair <double,int> > idx_maquinas; // completionTime, machine
  int maquina = 0;
  int at = 0;
  double tPr = 0;
  double makespan = 0.0;
  std::vector < std::pair < double, unsigned > > ranking(chromosome.size());
  for(unsigned i = 0; i < chromosome.size(); ++i){
    ranking[i]=std::pair<double,unsigned>(chromosome[i],i);
  }

  std::sort(ranking.begin(), ranking.end());
  std::vector<int> processos;
  int nTrocas = 0;
  long tTrocas = 0;
  extern unsigned tempoTroca;
  tPr = 0.0;
  for(std::vector<std::pair<double, unsigned>>::const_iterator i = ranking.begin(); i!=ranking.end(); ++i){
    at = (int) i->first; // A parte inteira do alelo representa a máquina
    if (maquina==0){
      maquina = at;
      processos.clear();
    }
    if (maquina==at){
      tPr += tProcessamento[i->second];
      processos.push_back(i->second);
    } else {
      nTrocas=KTNS(processos);
      tTrocas = tempoTroca*nTrocas;

      if ((tPr+tTrocas)>makespan){
        makespan = tPr+tTrocas;
      }

      maquinas.push_back(processos);
      idx_maquinas.push_back(std::pair<double,int>((tPr+tTrocas),maquina));

      maquina = at;
      processos.clear();
      tPr = tProcessamento[i->second];
      processos.push_back(i->second);

    }
  }
  nTrocas=KTNS(processos);
  tTrocas = tempoTroca*nTrocas;
  if ((tPr+tTrocas)>makespan){
    makespan = tPr+tTrocas;
  }
  maquinas.push_back(processos);
  idx_maquinas.push_back(std::pair<double,int>((tPr+tTrocas),maquina));
  makespan = avaliacaoTotal(maquinas, idx_maquinas ,tProcessamento);


  double oldMakespan = makespan;
  // vezes -1 = VND
  if (vezes == -1 ){
    int vnd = 1;
    while (vnd<6){

      if (vnd == 1){
        if (true) {
          tb1 = std::chrono::high_resolution_clock::now();
          IBSMakespan = makespan;
          IBS(maquinas,idx_maquinas,makespan,tProcessamento);
          if (IBSMakespan>makespan) {
              vnd = 1;
              ++melhoriasBuscas[0];
              mediaMelhorias[0]+= (IBSMakespan-makespan);
          } else
            ++vnd;
          tb2 = std::chrono::high_resolution_clock::now();
          std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          tempoBuscas[0]+=time_span_b.count();
          execucoesBuscas[0]++;
        } else
          ++vnd;

      }
      if (vnd == 2){
        if (true) {
          tb1 = std::chrono::high_resolution_clock::now();
          EFBMakespan = makespan;
          EFB(maquinas,idx_maquinas,makespan,tProcessamento);

          if (EFBMakespan>makespan) {
            vnd = 1;
            ++melhoriasBuscas[1];
            mediaMelhorias[1]+= (EFBMakespan-makespan);
          } else
            ++vnd;
          tb2 = std::chrono::high_resolution_clock::now();
          std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          tempoBuscas[1]+=time_span_b.count();
          execucoesBuscas[1]++;
        } else
          ++vnd;
      }
      // /*
      if (vnd == 3){
        if (false){
          tb1 = std::chrono::high_resolution_clock::now();
          ONBMakespan = makespan; // aproveitei a variavel
          FBI(maquinas,idx_maquinas,makespan,tProcessamento);
          if (ONBMakespan>makespan){
            vnd = 1;
            ++melhoriasBuscas[4];
            mediaMelhorias[4]+= (ONBMakespan-makespan);
          } else
            ++vnd;
          tb2 = std::chrono::high_resolution_clock::now();
          std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          tempoBuscas[4]+=time_span_b.count();
          execucoesBuscas[4]++;
        }else
          ++vnd;
      }

      // */

      if (vnd == 4){
        if (true){
          tb1 = std::chrono::high_resolution_clock::now();
          ONBMakespan = makespan; // aproveitei a variavel
          SUDECAP(maquinas,idx_maquinas,makespan,tProcessamento);
          if (ONBMakespan>makespan){
            vnd = 1;
            ++melhoriasBuscas[3];
            mediaMelhorias[3]+= (ONBMakespan-makespan);
          } else
            ++vnd;
          tb2 = std::chrono::high_resolution_clock::now();
          std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          tempoBuscas[3]+=time_span_b.count();
          execucoesBuscas[3]++;
        }else
          ++vnd;

      }


      if (vnd == 5){
        if (true){
          tb1 = std::chrono::high_resolution_clock::now();
          ONBMakespan = makespan;
          ONB(maquinas,idx_maquinas,makespan,tProcessamento);
          if (ONBMakespan>makespan){
            vnd = 1;
            ++melhoriasBuscas[2];
            mediaMelhorias[2]+= (ONBMakespan-makespan);
          } else
            ++vnd;
          tb2 = std::chrono::high_resolution_clock::now();
          std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
          tempoBuscas[2]+=time_span_b.count();
          execucoesBuscas[2]++;
        } else
          ++vnd;
      }
    }
  } else {  // Se não for aplicar o VND
    for(int i = 0; i < vezes; ++i){
      
      tb1 = std::chrono::high_resolution_clock::now();
      IBSMakespan = makespan;
      IBS(maquinas,idx_maquinas,makespan,tProcessamento);
      if (IBSMakespan>makespan) {
          ++melhoriasBuscas[0];
          mediaMelhorias[0]+= (IBSMakespan-makespan);
      }
      tb2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
    	time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
      tempoBuscas[0]+=time_span_b.count();
      execucoesBuscas[0]++;
      
      
      tb1 = std::chrono::high_resolution_clock::now();
      EFBMakespan = makespan;

      EFB(maquinas,idx_maquinas,makespan,tProcessamento);
      if (EFBMakespan>makespan) {
        ++melhoriasBuscas[1];
        mediaMelhorias[1]+= (EFBMakespan-makespan);
      }

      tb2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
    	time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
      tempoBuscas[1]+=time_span_b.count();
      execucoesBuscas[1]++;
     
      /*
      tb1 = std::chrono::high_resolution_clock::now();
      ONBMakespan = makespan;
      FBI(maquinas,idx_maquinas,makespan,tProcessamento);
      if (ONBMakespan>makespan){
        ++melhoriasBuscas[4];
        mediaMelhorias[4]+= (ONBMakespan-makespan);
      }
      tb2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
    	time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
      tempoBuscas[4]+=time_span_b.count();
      execucoesBuscas[4]++;
      
      */ 
      
      
      tb1 = std::chrono::high_resolution_clock::now();
      ONBMakespan = makespan;
      SUDECAP(maquinas,idx_maquinas,makespan,tProcessamento);
      if (ONBMakespan>makespan){
        ++melhoriasBuscas[3];
        mediaMelhorias[3]+= (ONBMakespan-makespan);
      }
      tb2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
    	time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
      tempoBuscas[3]+=time_span_b.count();
      execucoesBuscas[3]++;
      
     
      tb1 = std::chrono::high_resolution_clock::now();
      ONBMakespan = makespan;
      ONB(maquinas,idx_maquinas,makespan,tProcessamento);
      if (ONBMakespan>makespan){
        ++melhoriasBuscas[2];
        mediaMelhorias[2]+= (ONBMakespan-makespan);
      }
      tb2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
    	time_span_b = std::chrono::duration_cast<std::chrono::duration<double>>(tb2 - tb1);
      tempoBuscas[2]+=time_span_b.count();
      execucoesBuscas[2]++;

    }
  }
  if (oldMakespan>makespan){
 //   std::cout << "foi: " << oldMakespan << " voltou "  << makespan << "\n";
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    int pRanking = 0;
    // Corrijo o  chromosome
    int pMaquina = 0;
    double lInferior = 0;
    for(std::vector<std::vector<int>>::const_iterator i = maquinas.begin(); i!=maquinas.end(); ++i){
      std::vector<int>maquinaAux = *i;
      ++pMaquina;
      // Sortear N numeros, ordenar e distribuir
      std::vector<double> tempKey;
      std::uniform_real_distribution<double> distribution(pMaquina,pMaquina+1);
      for (int b=0;b<maquinaAux.size();++b)
        tempKey.push_back(distribution(generator));
      std::sort(tempKey.begin(), tempKey.end());
      int kIdx = 0;
      for (std::vector<int>::const_iterator j = maquinaAux.begin(); j!=maquinaAux.end(); ++j){
        ranking[pRanking].second = *j;
        ranking[pRanking].first = tempKey[kIdx];
        ++kIdx;
        /*
        if ((ranking[pRanking].first >= pMaquina+1) ||(ranking[pRanking].first < pMaquina) ){
          if (ranking[pRanking-1].first < pMaquina) lInferior = pMaquina; else lInferior = ranking[pRanking-1].first;
          std::uniform_real_distribution<double> distribution(lInferior,pMaquina+1);
          ranking[pRanking].first = distribution(generator);
        }
        */
        chromosome[ranking[pRanking].second] = ranking[pRanking].first;
        ++pRanking;
      }
    }
  }
  return makespan;
  // Versao IBS + SUDECAP + ONB
}

#endif