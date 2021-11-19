#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import math
import os
from gurobipy import *
# Este modelo retorna o valor da solução relaxada somado ao tempo de uma troca
def main():
    arq = open("RELAXACAO_RESULTADO","w") # Arquivo de resultados
    caminho = sys.argv[1:]
    for pf in caminho: # Executa-se para cada arquivo
        print(pf)
        #print(p)
        # lendo dados de entrada
        problem_file = open(pf, 'r')
        linha = problem_file.readline().split()
        
        qte_tarefas = int(linha[0])
        qte_maquinas = int(linha[1])
        qte_moldes =  int(linha[2])
        # pula a linha com o tempoTroca
        linha = problem_file.readline().split()
        tempo_troca = int(linha[0])
        moldes = []
        linha = problem_file.readline().split()
        for i in range(qte_tarefas):
          moldes.append(int(linha[i]))

        linha = problem_file.readline().split()
        tarefas = []
        for i in range(qte_tarefas):
          tarefas.append(int(linha[i]))
        # ==================================

        modelo = Model("Geral2020")
        
        #variáveis
        
        #p_i - Tempo de processamento da tarefa i
        p = []
        for i in range(qte_tarefas):
            p.append(modelo.addVar(obj=0, name="p[%d]" % (i)))
        #x_i_j - Tarefa J está alocada a maquina I
        x = []
        for i in range(qte_maquinas):
            x.append([])
            for j in range(qte_tarefas):
                x[i].append(modelo.addVar(obj=0, name="x[%d, %d]" % (i, j), vtype= GRB.BINARY))
        #delta - makespan
        delta = modelo.addVar(obj=1, name="cmax")

         
        # tempo de processamento
        for j in range(qte_tarefas):
            modelo.addConstr(p[j] == tarefas[j])
        # cada tarefa tem que estar em apenas uma máquina
        for j in range(qte_tarefas):
            modelo.addConstr(sum(x[i][j] for i in range(qte_maquinas)) == 1)
        
        #calcula o makespan
        for i in range(qte_maquinas):
            modelo.addConstr(sum(x[i][j] * p[j] for j in range(qte_tarefas)) <= delta)

        modelo.update()
        
        modelo.Params.TimeLimit = 1200
        modelo.Params.DisplayInterval = 60
        modelo.Params.OutputFlag = 0
        modelo.optimize()
        '''
        if modelo.status == GRB.Status.OPTIMAL:
            print ('\n\nSolução ótima: %g' % modelo.objVal);
            for m in range(qte_maquinas):
                sys.stdout.write('Máquina %d: '%(m+1))
                for k in range(qte_tarefas):
                    x1 =modelo.getVarByName("x[%d]" % (k))
                    if(x1.x==1-m):
                        sys.stdout.write ('%d ' % (k))
                print('')
        else:
            print('\n\nNão foi encontrada uma solução ótima')
            print("Lower Bound: %g" % modelo.objBound)
            print("Solução encontrada: %g" %modelo.objVal)
        print ("Tempo de execuçao: %g" % modelo.Runtime)
        print ("Número de variáveis: %g" % modelo.NumVars)
        print ("Número de restrições: %g" % modelo.NumConstrs)
        print ("GAP: %g " % (modelo.MIPGap*100))

        print('\n\n\n')
        '''
        print("Solução encontrada: %g" %modelo.objVal)
        print("Lower Bound: %g" % modelo.objBound)
        print ("Tempo de execuçao: %g" % modelo.Runtime)
        print ("Número de variáveis: %g" % modelo.NumVars)
        print ("Número de restrições: %g" % modelo.NumConstrs)
        arq.write("%g" %(modelo.objVal + 0))
        arq.flush()
    arq.close()
if __name__ == "__main__":
    main()
