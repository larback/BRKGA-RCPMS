#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
import math
import os
from gurobipy import *

def main():
    # As instancias devem estar em um subdiretorio chamado 'instancias_moldes' - o programa lerá todas as instancias deste subdiretorio
    #caminho = [os.path.join("./testes", nome) for nome in os.lisßtdir("./testes")]
    caminho = ["./DB/instancias_padronizadas/A4.txt"]
    caminho = ["./instancias/HA008.TXT"]
    DEBUG = 2
    arq = open("MODELO_RESULTADOS_H","w") # Arquivo de resultados
    for pf in caminho: # Executa-se para cada arquivo
        print(pf)
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

        modelo = Model("Chung2019")
        modelo.modelSense = GRB.MINIMIZE

        #print(qte_tarefas)
        #print(qte_maquinas)
        #print(qte_moldes)

        #print(moldes)
        #print(tarefas)
        #variáveis

        M = modelo.addVar(obj=0, name="M")

        #z_{ih} = (m_i - m_h)^2
        z=[]
        for i in range(qte_tarefas):
            z.append([])
            for h in range(qte_tarefas):
                z[i].append(modelo.addVar(obj=0, name="z[%d, %d]" % (i, h), vtype = GRB.CONTINUOUS))


        #y_h if job h is the fisrt job on one of the machines
        y=[]
        for h in range(qte_tarefas):
            y.append(modelo.addVar(obj=0, name="y[%d]" % (h), vtype=GRB.BINARY))
        
        #x_{ih} if job i is scheduled directly before job h on the same machine
        x = []
        for i in range(qte_tarefas):
            x.append([])
            for h in range(qte_tarefas+1):
                x[i].append(modelo.addVar(obj=0, name="x[%d, %d]" % (i, h), vtype = GRB.BINARY))
        
        #b_{ih} if job i begin its processing before job h 
        b = []
        for i in range(qte_tarefas):
            b.append([])
            for h in range(qte_tarefas):
                b[i].append(modelo.addVar(obj=0, name="b[%d, %d]" % (i, h), vtype = GRB.BINARY))

        #c_i - completion time of job i
        c = []
        for i in range(qte_tarefas):
            c.append(modelo.addVar(obj=0, name="c[%d]" % (i), vtype= GRB.CONTINUOUS))
        
        #p_i - nao consta na descricao do modelo, mas é usado em uma restricao
        p = []
        for i in range(qte_tarefas):
            p.append(modelo.addVar(obj=0, name="p[%d]" % (i), vtype= GRB.CONTINUOUS))
        #cmax - makespan
        cmax = modelo.addVar(obj=1, name="cmax")



        #Restrições - As númerações identificam as restrições em acordo com o relatório

        for i in range(qte_tarefas):
            for h in range(qte_tarefas):
                modelo.addConstr(z[i][h] == (moldes[i]-moldes[h])**2)
        sP = 0
        for i in range(qte_tarefas):
            modelo.addConstr(p[i] == tarefas[i])
            sP += p[i]
        
        modelo.addConstr(M == sP * qte_moldes)

        # (2)
        modelo.addConstr(sum(y[h] for h in range(qte_tarefas))  <= 2)
        
        # (3)
        for h in range(qte_tarefas):
            modelo.addConstr(y[h] + sum(x[i][h] for i in range(qte_tarefas) if i!=h) == 1)
    
        # (4) 
        for h in range(qte_tarefas):
            modelo.addConstr(sum(x[h][i] for i in range(qte_tarefas+1) if i!=h) == 1)
        
        # (5)
        for h in range(qte_tarefas):
            modelo.addConstr(c[h] - (p[h] * y[h]) >= 0)
        
        # (6)
        for h in range(qte_tarefas):
            for i in range(qte_tarefas):
                if (h!=i):
                    modelo.addConstr(b[h][i] + b[i][h] == 1)
           
        # (7)
        for h in range(qte_tarefas-1):
            modelo.addConstr(sum ((b[h][i] + b[i][h])  for i in range(qte_tarefas) if i!=h) +1 == qte_tarefas)
        
        # (8)
        for h in range(qte_tarefas):
            for i in range(qte_tarefas):
                if (h!=i):
                    modelo.addConstr(c[h]-c[i]+ M*(1-b[i][h])+M*z[i][h]>=p[h])
        
        # (9) 
        for h in range(qte_tarefas):
            for i in range(qte_tarefas):
                if (h!=i):
                    modelo.addConstr(c[h]-c[i] + M * (1-x[i][h])>=p[h])
        
        # (10)
        modelo.addConstr(sum (x[i][qte_tarefas] for i in range(qte_tarefas)) <=2)
        
        # (11)
        for i in range(qte_tarefas):
            for h in range(qte_tarefas):
                if (h!=i):
                    modelo.addConstr(x[i][h] + x[h][i] <= 1)
        
        # (12)
        for h in range(qte_tarefas):
            modelo.addConstr(cmax - c[h] >= 0)
        
        modelo.addConstr(cmax>=0)

        for h in range(qte_tarefas):
            modelo.addConstr(c[h] >= 0)


                    
        modelo.update()
        #print(modelo)
        #modelo.Params.Threads = 4
        modelo.Params.TimeLimit = 300
        modelo.Params.DisplayInterval = 60
        modelo.optimize()
        if modelo.status == GRB.Status.OPTIMAL:
            print ('\n\nSolução ótima: %g' % modelo.objVal);
            if DEBUG==2:        
                maquina = [ [], [] ]
                k=0
                for i in range(qte_tarefas):
                    y1 = modelo.getVarByName("y[%d]" % (i))
                    if y1.x > 0:
                        maquina[k].append(i)
                        proximo = i
                        while proximo!=qte_tarefas:
                            for j in range(qte_tarefas+1):
                                x1 = modelo.getVarByName("x[%d, %d]" % (proximo,j))
                                if (x1.x > 0):
                                    proximo = j
                                    if j != qte_tarefas:
                                        maquina[k].append(j)
                                    break
                        k = k+1
                        if (k>1):
                            break
                print('Máquina 1: ',end='')
                print(' '.join(map(str, maquina[0])))  
                print('Máquina 2: ',end='')
                print(' '.join(map(str, maquina[1])))  
        else:
            print('\n\nNão foi encontrada uma solução ótima')
            print("Lower Bound: %g" % modelo.objBound)
            print("Solução encontrada: %g" %modelo.objVal)
        print ("Tempo de execuçao: %g" % modelo.Runtime)
        print ("Número de variáveis: %g" % modelo.NumVars)
        print ("Número de restrições: %g" % modelo.NumConstrs)
        print ("GAP: %g " % (modelo.MIPGap*100))
        
        if DEBUG == 1:
            print("y[h]")
            for j in range(qte_tarefas):
                yh = modelo.getVarByName("y[%d]"% (j))
                print(yh.x, " ", end='')
            print("\nx[ih]") 
            for i in range(qte_tarefas):
                for j in range(qte_tarefas+1):
                    xih = modelo.getVarByName("x[%d, %d]"% (i,j))
                    print(xih.x, " ", end='')
                print()
            print()
            print("bih")
            for i in range(qte_tarefas):
                for j in range(qte_tarefas):
                    bih = modelo.getVarByName("b[%d, %d]"% (i,j))
                    print(bih.x, " ", end='')
                print()
        
       
        #for v in modelo.getVars():
        #        print('%s %g' % (v.varName, v.x))
        print('\n\n\n')
        arq.write("%s" %pf)
        arq.write(" %g %g %g %g %g %g \n" %(modelo.objVal, modelo.objBound, modelo.RunTime, modelo.NumVars, modelo.NumConstrs,modelo.MIPGap*100))
        arq.flush()
    arq.close()
if __name__ == "__main__":
    DEBUG = 0 # 0 - nada, 1 - variaveis, 2 - solucao
    #Todas as pequenas
    caminho = ["./DB/instancias_padronizadas/A0.txt",
    "./DB/instancias_padronizadas/A1.txt",
    "./DB/instancias_padronizadas/A2.txt",
    "./DB/instancias_padronizadas/A3.txt",
    "./DB/instancias_padronizadas/A4.txt",
    "./DB/instancias_padronizadas/A5.txt",
    "./DB/instancias_padronizadas/A6.txt",
    "./DB/instancias_padronizadas/A7.txt",
    "./DB/instancias_padronizadas/A8.txt",
    "./DB/instancias_padronizadas/A9.txt",
    "./DB/instancias_padronizadas/B0.txt",
    "./DB/instancias_padronizadas/B1.txt",
    "./DB/instancias_padronizadas/B2.txt",
    "./DB/instancias_padronizadas/B3.txt",
    "./DB/instancias_padronizadas/B4.txt",
    "./DB/instancias_padronizadas/B5.txt",
    "./DB/instancias_padronizadas/B6.txt",
    "./DB/instancias_padronizadas/B7.txt",
    "./DB/instancias_padronizadas/B8.txt",
    "./DB/instancias_padronizadas/B9.txt",
    "./DB/instancias_padronizadas/C0.txt",
    "./DB/instancias_padronizadas/C1.txt",
    "./DB/instancias_padronizadas/C2.txt",
    "./DB/instancias_padronizadas/C3.txt",
    "./DB/instancias_padronizadas/C4.txt",
    "./DB/instancias_padronizadas/C5.txt",
    "./DB/instancias_padronizadas/C6.txt",
    "./DB/instancias_padronizadas/C7.txt",
    "./DB/instancias_padronizadas/C8.txt",
    "./DB/instancias_padronizadas/C9.txt",
    "./DB/instancias_padronizadas/D0.txt",
    "./DB/instancias_padronizadas/D1.txt",
    "./DB/instancias_padronizadas/D2.txt",
    "./DB/instancias_padronizadas/D3.txt",
    "./DB/instancias_padronizadas/D4.txt",
    "./DB/instancias_padronizadas/D5.txt",
    "./DB/instancias_padronizadas/D6.txt",
    "./DB/instancias_padronizadas/D7.txt",
    "./DB/instancias_padronizadas/D8.txt",
    "./DB/instancias_padronizadas/D9.txt",
    "./DB/instancias_padronizadas/E0.txt",
    "./DB/instancias_padronizadas/E1.txt",
    "./DB/instancias_padronizadas/E2.txt",
    "./DB/instancias_padronizadas/E3.txt",
    "./DB/instancias_padronizadas/E4.txt",
    "./DB/instancias_padronizadas/E5.txt",
    "./DB/instancias_padronizadas/E6.txt",
    "./DB/instancias_padronizadas/E7.txt",
    "./DB/instancias_padronizadas/E8.txt",
    "./DB/instancias_padronizadas/E9.txt",
    "./DB/instancias_padronizadas/F0.txt",
    "./DB/instancias_padronizadas/F1.txt",
    "./DB/instancias_padronizadas/F2.txt",
    "./DB/instancias_padronizadas/F3.txt",
    "./DB/instancias_padronizadas/F4.txt",
    "./DB/instancias_padronizadas/F5.txt",
    "./DB/instancias_padronizadas/F6.txt",
    "./DB/instancias_padronizadas/F7.txt",
    "./DB/instancias_padronizadas/F8.txt",
    "./DB/instancias_padronizadas/F9.txt",
    "./DB/instancias_padronizadas/G0.txt",
    "./DB/instancias_padronizadas/G1.txt",
    "./DB/instancias_padronizadas/G2.txt",
    "./DB/instancias_padronizadas/G3.txt",
    "./DB/instancias_padronizadas/G4.txt",
    "./DB/instancias_padronizadas/G5.txt",
    "./DB/instancias_padronizadas/G6.txt",
    "./DB/instancias_padronizadas/G7.txt",
    "./DB/instancias_padronizadas/G8.txt",
    "./DB/instancias_padronizadas/G9.txt",
    "./DB/instancias_padronizadas/H0.txt",
    "./DB/instancias_padronizadas/H1.txt",
    "./DB/instancias_padronizadas/H2.txt",
    "./DB/instancias_padronizadas/H3.txt",
    "./DB/instancias_padronizadas/H4.txt",
    "./DB/instancias_padronizadas/H5.txt",
    "./DB/instancias_padronizadas/H6.txt",
    "./DB/instancias_padronizadas/H7.txt",
    "./DB/instancias_padronizadas/H8.txt",
    "./DB/instancias_padronizadas/H9.txt",
    "./DB/instancias_padronizadas/I0.txt",
    "./DB/instancias_padronizadas/I1.txt",
    "./DB/instancias_padronizadas/I2.txt",
    "./DB/instancias_padronizadas/I3.txt",
    "./DB/instancias_padronizadas/I4.txt",
    "./DB/instancias_padronizadas/I5.txt",
    "./DB/instancias_padronizadas/I6.txt",
    "./DB/instancias_padronizadas/I7.txt",
    "./DB/instancias_padronizadas/I8.txt",
    "./DB/instancias_padronizadas/I9.txt",
    "./DB/instancias_padronizadas/J0.txt",
    "./DB/instancias_padronizadas/J1.txt",
    "./DB/instancias_padronizadas/J2.txt",
    "./DB/instancias_padronizadas/J3.txt",
    "./DB/instancias_padronizadas/J4.txt",
    "./DB/instancias_padronizadas/J5.txt",
    "./DB/instancias_padronizadas/J6.txt",
    "./DB/instancias_padronizadas/J7.txt",
    "./DB/instancias_padronizadas/J8.txt",
    "./DB/instancias_padronizadas/J9.txt",
    "./DB/instancias_padronizadas/K0.txt",
    "./DB/instancias_padronizadas/K1.txt",
    "./DB/instancias_padronizadas/K2.txt",
    "./DB/instancias_padronizadas/K3.txt",
    "./DB/instancias_padronizadas/K4.txt",
    "./DB/instancias_padronizadas/K5.txt",
    "./DB/instancias_padronizadas/K6.txt",
    "./DB/instancias_padronizadas/K7.txt",
    "./DB/instancias_padronizadas/K8.txt",
    "./DB/instancias_padronizadas/K9.txt",
    "./DB/instancias_padronizadas/L0.txt",
    "./DB/instancias_padronizadas/L1.txt",
    "./DB/instancias_padronizadas/L2.txt",
    "./DB/instancias_padronizadas/L3.txt",
    "./DB/instancias_padronizadas/L4.txt",
    "./DB/instancias_padronizadas/L5.txt",
    "./DB/instancias_padronizadas/L6.txt",
    "./DB/instancias_padronizadas/L7.txt",
    "./DB/instancias_padronizadas/L8.txt",
    "./DB/instancias_padronizadas/L9.txt",
    "./DB/instancias_padronizadas/M0.txt",
    "./DB/instancias_padronizadas/M1.txt",
    "./DB/instancias_padronizadas/M2.txt",
    "./DB/instancias_padronizadas/M3.txt",
    "./DB/instancias_padronizadas/M4.txt",
    "./DB/instancias_padronizadas/M5.txt",
    "./DB/instancias_padronizadas/M6.txt",
    "./DB/instancias_padronizadas/M7.txt",
    "./DB/instancias_padronizadas/M8.txt",
    "./DB/instancias_padronizadas/M9.txt",
    "./DB/instancias_padronizadas/N0.txt",
    "./DB/instancias_padronizadas/N1.txt",
    "./DB/instancias_padronizadas/N2.txt",
    "./DB/instancias_padronizadas/N3.txt",
    "./DB/instancias_padronizadas/N4.txt",
    "./DB/instancias_padronizadas/N5.txt",
    "./DB/instancias_padronizadas/N6.txt",
    "./DB/instancias_padronizadas/N7.txt",
    "./DB/instancias_padronizadas/N8.txt",
    "./DB/instancias_padronizadas/N9.txt",
    "./DB/instancias_padronizadas/O0.txt",
    "./DB/instancias_padronizadas/O1.txt",
    "./DB/instancias_padronizadas/O2.txt",
    "./DB/instancias_padronizadas/O3.txt",
    "./DB/instancias_padronizadas/O4.txt",
    "./DB/instancias_padronizadas/O5.txt",
    "./DB/instancias_padronizadas/O6.txt",
    "./DB/instancias_padronizadas/O7.txt",
    "./DB/instancias_padronizadas/O8.txt",
    "./DB/instancias_padronizadas/O9.txt",
    "./DB/instancias_padronizadas/P0.txt",
    "./DB/instancias_padronizadas/P1.txt",
    "./DB/instancias_padronizadas/P3.txt",
    "./DB/instancias_padronizadas/P4.txt",
    "./DB/instancias_padronizadas/P5.txt",
    "./DB/instancias_padronizadas/P6.txt",
    "./DB/instancias_padronizadas/P7.txt",
    "./DB/instancias_padronizadas/P8.txt",
    "./DB/instancias_padronizadas/P9.txt",
    "./DB/instancias_padronizadas/Q0.txt",
    "./DB/instancias_padronizadas/Q1.txt",
    "./DB/instancias_padronizadas/Q2.txt",
    "./DB/instancias_padronizadas/Q3.txt",
    "./DB/instancias_padronizadas/Q4.txt",
    "./DB/instancias_padronizadas/Q5.txt",
    "./DB/instancias_padronizadas/Q6.txt",
    "./DB/instancias_padronizadas/Q7.txt",
    "./DB/instancias_padronizadas/Q8.txt",
    "./DB/instancias_padronizadas/Q9.txt",
    "./DB/instancias_padronizadas/R0.txt",
    "./DB/instancias_padronizadas/R1.txt",
    "./DB/instancias_padronizadas/R2.txt",
    "./DB/instancias_padronizadas/R3.txt",
    "./DB/instancias_padronizadas/R4.txt",
    "./DB/instancias_padronizadas/R5.txt",
    "./DB/instancias_padronizadas/R6.txt",
    "./DB/instancias_padronizadas/R7.txt",
    "./DB/instancias_padronizadas/R8.txt",
    "./DB/instancias_padronizadas/R9.txt",
    "./DB/instancias_padronizadas/S0.txt",
    "./DB/instancias_padronizadas/S1.txt",
    "./DB/instancias_padronizadas/S2.txt",
    "./DB/instancias_padronizadas/S3.txt",
    "./DB/instancias_padronizadas/S4.txt",
    "./DB/instancias_padronizadas/S5.txt",
    "./DB/instancias_padronizadas/S6.txt",
    "./DB/instancias_padronizadas/S7.txt",
    "./DB/instancias_padronizadas/S8.txt",
    "./DB/instancias_padronizadas/S9.txt",
    "./DB/instancias_padronizadas/T0.txt",
    "./DB/instancias_padronizadas/T1.txt",
    "./DB/instancias_padronizadas/T2.txt",
    "./DB/instancias_padronizadas/T3.txt",
    "./DB/instancias_padronizadas/T4.txt",
    "./DB/instancias_padronizadas/T5.txt",
    "./DB/instancias_padronizadas/T6.txt",
    "./DB/instancias_padronizadas/T7.txt",
    "./DB/instancias_padronizadas/T8.txt",
    "./DB/instancias_padronizadas/T9.txt",
    "./DB/instancias_padronizadas/U0.txt",
    "./DB/instancias_padronizadas/U1.txt",
    "./DB/instancias_padronizadas/U2.txt",
    "./DB/instancias_padronizadas/U3.txt",
    "./DB/instancias_padronizadas/U4.txt",
    "./DB/instancias_padronizadas/U5.txt",
    "./DB/instancias_padronizadas/U6.txt",
    "./DB/instancias_padronizadas/U7.txt",
    "./DB/instancias_padronizadas/U8.txt",
    "./DB/instancias_padronizadas/U9.txt",
    "./DB/instancias_padronizadas/V0.txt",
    "./DB/instancias_padronizadas/V1.txt",
    "./DB/instancias_padronizadas/V2.txt",
    "./DB/instancias_padronizadas/V3.txt",
    "./DB/instancias_padronizadas/V4.txt",
    "./DB/instancias_padronizadas/V5.txt",
    "./DB/instancias_padronizadas/V6.txt",
    "./DB/instancias_padronizadas/V7.txt",
    "./DB/instancias_padronizadas/V8.txt",
    "./DB/instancias_padronizadas/V9.txt",
    "./DB/instancias_padronizadas/W0.txt",
    "./DB/instancias_padronizadas/W1.txt",
    "./DB/instancias_padronizadas/W2.txt",
    "./DB/instancias_padronizadas/W3.txt",
    "./DB/instancias_padronizadas/W4.txt",
    "./DB/instancias_padronizadas/W5.txt",
    "./DB/instancias_padronizadas/W6.txt",
    "./DB/instancias_padronizadas/W7.txt",
    "./DB/instancias_padronizadas/W8.txt",
    "./DB/instancias_padronizadas/W9.txt",
    "./DB/instancias_padronizadas/X0.txt",
    "./DB/instancias_padronizadas/X1.txt",
    "./DB/instancias_padronizadas/X2.txt",
    "./DB/instancias_padronizadas/X3.txt",
    "./DB/instancias_padronizadas/X4.txt",
    "./DB/instancias_padronizadas/X5.txt",
    "./DB/instancias_padronizadas/X6.txt",
    "./DB/instancias_padronizadas/X7.txt",
    "./DB/instancias_padronizadas/X8.txt",
    "./DB/instancias_padronizadas/X9.txt",
    "./DB/instancias_padronizadas/Y0.txt",
    "./DB/instancias_padronizadas/Y1.txt",
    "./DB/instancias_padronizadas/Y2.txt",
    "./DB/instancias_padronizadas/Y3.txt",
    "./DB/instancias_padronizadas/Y4.txt",
    "./DB/instancias_padronizadas/Y5.txt",
    "./DB/instancias_padronizadas/Y6.txt",
    "./DB/instancias_padronizadas/Y7.txt",
    "./DB/instancias_padronizadas/Y8.txt",
    "./DB/instancias_padronizadas/Y9.txt",
    "./DB/instancias_padronizadas/Z0.txt",
    "./DB/instancias_padronizadas/Z1.txt",
    "./DB/instancias_padronizadas/Z2.txt",
    "./DB/instancias_padronizadas/Z3.txt",
    "./DB/instancias_padronizadas/Z4.txt",
    "./DB/instancias_padronizadas/Z5.txt",
    "./DB/instancias_padronizadas/Z6.txt",
    "./DB/instancias_padronizadas/Z7.txt",
    "./DB/instancias_padronizadas/Z8.txt",
    "./DB/instancias_padronizadas/Z9.txt",
    "./DB/instancias_padronizadas/p2.txt"]

    main()
