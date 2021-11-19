#include <dirent.h>
#include <cstdlib>
#include <string>
#include <fstream>

#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

int main(int argc, char* argv[]) {
	vector<vector<int>> intervalos;
	int m = 2;
	for (int i=0; i<2; ++i)
		intervalos.push_back(vector<int>(0));
	intervalos[0].push_back(2);
	intervalos[0].push_back(20);
	intervalos[0].push_back(1);

	intervalos[0].push_back(4);
	intervalos[0].push_back(10);
	intervalos[0].push_back(2);

	intervalos[0].push_back(6);
	intervalos[0].push_back(15);
	intervalos[0].push_back(1);


	intervalos[1].push_back(3);
	intervalos[1].push_back(20);
	intervalos[1].push_back(1);

	int critica = 1;
	for (int i=0;i<intervalos[critica].size();i+=3)
		cout << "Antes de " << intervalos[critica][i] << " gap de " << intervalos[critica][i+1] << " molde diferente de " << intervalos[critica][i+2] << endl;
	return 0;

}
