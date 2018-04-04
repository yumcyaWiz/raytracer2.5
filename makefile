all:
	g++ -std=c++14 -Wall -O3 -fopenmp main.cpp

debug:
	g++ -std=c++14 -Wall -O0 -g -o debug main.cpp
