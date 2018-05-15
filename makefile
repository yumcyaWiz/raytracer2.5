all:
	g++ -std=c++14 -Wall -O3 -mavx -fopenmp -lglut -lGLU -lGL -lprofiler main.cpp

asb:
	g++ -std=c++14 -Wall -S -O3 -mavx -fopenmp -lglut -lGLU -lGL main.cpp

debug:
	g++ -std=c++14 -Wall -O0 -g debug -lglut -lGLU -lGL main.cpp

profile:
	g++ -std=c++14 -Wall -lglut -lGLU -lGL -lprofiler -g main.cpp
