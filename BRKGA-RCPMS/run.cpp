#include <dirent.h>
#include <cstdlib>
#include <string>
#include <fstream>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    std::string nomeArq;
    std::ifstream file;
	std::ofstream fileR;
	if (argc != 3) {
        std::cerr << "Diretório com as instâncias não foi informado." << std::endl;
        exit (1);
    }
	std::string nomeDir = argv[1];
	int repeticoes = std::stoi(argv[2]);
	DIR *dir = 0;
    struct dirent *entrada = 0;
    unsigned char isFile = 0x8;

    dir = opendir (nomeDir.c_str());

    if (dir == 0) {
        std::cerr << "Nao foi possível abrir diretorio com as instâncias." << std::endl;
        exit (1);
    }
    int s;

	// Pego todas as instâncias do diretório
	while ((entrada = readdir (dir))){
        // if ((entrada->d_type == isFile) ){
         nomeArq = entrada->d_name;
         if ((nomeArq.compare(".")!=0) && (nomeArq.compare("..")!=0) && (nomeArq.compare("solucoes")!=0) && (nomeArq.compare(".DS_Store")!=0)){
                // Executo o programa N vezes
            std::string xF = nomeDir + "solucoes/BRKGA_SOLUCAO_"+entrada->d_name + "_10";
            const char * x = xF.c_str();
            FILE *TFile;// = fopen(x,"r");


            std::cout << "[Obtendo lower bound para instância...]" << std::endl;
            std::string cmd_modelo = "gurobi.sh modelo_geral.py " + nomeDir + entrada->d_name;
            const char * cm = cmd_modelo.c_str();
            s = system(cm);
            file.open("RELAXACAO_RESULTADO", std::ifstream::in);
            std::string lowerbound;
            file >> lowerbound;
            file.close();
            // std::cout << lowerbound << std::endl;

            for (int i=1;i<=repeticoes;++i) {
                    std::stringstream convert;
                    std::cout << i << " Execucao: " << nomeDir+entrada->d_name << std::endl;
                    convert << i;
                    std::string xF = nomeDir + "solucoes/BRKGA_SOLUCAO_"+entrada->d_name +  "_" + convert.str();
                    x = xF.c_str();
                    std::ifstream existeInstancia;
                    existeInstancia.open(xF);
                   // if (!existeInstancia.is_open()){ // Descomentar este if para nao reexecutar instancias
                                std::string cmd = "./samplecode " + lowerbound + " <" + nomeDir+entrada->d_name + " >" + nomeDir + "solucoes/BRKGA_SOLUCAO_"+entrada->d_name + "_" + convert.str();
                                const char * c = cmd.c_str();
                                s = system(c);
                  //  } else{
                  //      existeInstancia.close();
                  //      std::cout <<  "já executado. " << std::endl;
                  //  }
            }
        //} else {
        //  std::cout << nomeDir + "solucoes/BRKGA_SOLUCAO_"+entrada->d_name + " já executado." << std::endl;
        //}
        }
	}
    closedir (dir);

    nomeDir+="solucoes/";
    std::cout << nomeDir;
	dir = opendir (nomeDir.c_str());
	// Gerando o acumulado

	std::string dado, dado2, dado3, dado4, dado5, dado6, dado7, dado8, dado9, dado10, dado11, dado12, dado13, dado14, dado15, dado16, dado17, dado18, dado19, dado20, dado21;
	fileR.open(nomeDir+"BRKGA_RESUMO.txt");
	while ((entrada = readdir (dir))){
        //if ((entrada->d_type == isFile)){
			 nomeArq = entrada->d_name;
			  nomeArq = entrada->d_name;
    
         if ((nomeArq.compare(".")!=0) && (nomeArq.compare("..")!=0) && (nomeArq.compare("solucoes")!=0) && (nomeArq.compare(".DS_Store")!=0) && (nomeArq.compare("BRKGA_RESUMO.txt")!=0)){
				file.open(nomeDir+entrada->d_name, std::ifstream::in);
                
                
                file >> dado;
                file >> dado2;
                file >> dado3;
                file >> dado4;
                file >> dado5;
                file >> dado6;
                file >> dado7;
                file >> dado8;
                file >> dado9;
                file >> dado10;
                file >> dado11;
                file >> dado12;
                file >> dado13;
                file >> dado14;
                file >> dado15;
                file >> dado16;
                file >> dado17;
                file >> dado18;
                file >> dado19;
                file >> dado20;
		file >> dado21;
                // SO IMPRIME 300 - REMOVER
                //if (stoi(dado18)==300) {
                fileR << entrada->d_name << " " << dado << " " << dado2 << " " << dado3 << " " << dado4 << " " << dado5 << " " << dado6 << " " << dado7 << " " << dado8 << " " << dado9 << " " << dado10 <<  " " << dado11 <<  " " << dado12 <<  " " << dado13 <<  " " << dado14 <<  " " << dado15 << " " << dado16 << " " << dado17 << " " << dado18 << " " << dado19 << " " << dado20 << " " << dado21 << std::endl;
                //}
                file.close();
                // instancia n t m makespan tempo melhorGeracao solucaoInicial mediaTempoBuscas[4x] mediaMelhoriaBuscas[4x] geracoes
			 }
		// }
	}
	fileR.close();
	closedir(dir);
	return 0;
}
