/*
 * SampleDecoder.h
 *
 * Any decoder must have the format below, i.e., implement the method decode(std::vector< double >&)
 * returning a double corresponding to the fitness of that vector. If parallel decoding is to be
 * used in the BRKGA framework, then the decode() method _must_ be thread-safe; the best way to
 * guarantee this is by adding 'const' to the end of decode() so that the property will be checked
 * at compile time.
 *
 * The chromosome inside the BRKGA framework can be changed if desired. To do so, just use the
 * first signature of decode() which allows for modification. Please use double values in the
 * interval [0,1) when updating, thus obeying the BRKGA guidelines.
 *
 *  Created on: Jan 14, 2011
 *      Author: rtoso
 */

#ifndef SAMPLEDECODER_H
#define SAMPLEDECODER_H

#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <random>
#include <chrono>
#include "KTNS.h"
#include "Buscas.h"
class SampleDecoder {
public:
	SampleDecoder()  { }
	SampleDecoder(std::vector<unsigned> _tProcessamento): tProcessamento(_tProcessamento) { }
	~SampleDecoder() { }

	double decode(std::vector< double >& chromosome) const{
		extern int m;
		extern unsigned t;
		extern unsigned tempoTroca;
		extern int setupTime;
		extern std::vector<unsigned>tMoldes;
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
		// cout << "avaliado: " << makespan << " "; 
		return makespan;
		}

private:
	std::vector<unsigned> tProcessamento;
};

#endif