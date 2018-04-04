all:
	g++ -std=c++14 -Wall -O3 -fopenmp main.cpp

debug:
	g++ -std=c++14 -Wall -O3 -fopenmp -g main.cpp
