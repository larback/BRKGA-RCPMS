#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import math
import os
from gurobipy import *

def main():
    arq = open("RELAXACAO_RESULTADO_2004","w") # Arquivo de resultados
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

        moldes = []
        linha = problem_file.readline().split()
        for i in range(qte_tarefas):
          moldes.append(int(linha[i]))

        linha = problem_file.readline().split()
        tarefas = []
        for i in range(qte_tarefas):
          tarefas.append(int(linha[i]))
        # ==================================

        modelo = Model("Mokotoff2004")
        
        #variáveis
        
        #p_i 
        p = []
        for i in range(qte_tarefas):
            p.append(modelo.addVar(obj=0, name="p[%d]" % (i)))
        x = []
        for i in range(qte_tarefas):
            x.append(modelo.addVar(obj=0, name="x[%d]" % (i), vtype= GRB.BINARY))
        #cmax - makespan
        cmax = modelo.addVar(obj=1, name="cmax")

        #Restrições - As númerações identificam as restrições em acordo com o relatório

        for i in range(qte_tarefas):
            modelo.addConstr(p[i] == tarefas[i])

        modelo.addConstr(cmax - sum(p[i] * x[i] for i in range(qte_tarefas))>=0)
        modelo.addConstr(cmax - sum(p[i]*(1-x[i]) for i in range(qte_tarefas))>=0)
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
        arq.write("%g" %(modelo.objVal))
        arq.flush()
    arq.close()
if __name__ == "__main__":
    main()