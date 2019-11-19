#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <omp.h>
#include <string>
using namespace std;


int main(int argc, char* argv[]) {

	if (argc != 4) {
		cout << "You entered " <<argc << " arguments, " << 4 << " are required!";
		return;
	}

	int numThreads = argv[1];
	int numCities = argv[2];
	string cityFile = argv[3];

}