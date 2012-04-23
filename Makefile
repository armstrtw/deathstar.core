all: deathstar

deathstar: deathstar.cpp
	g++ -g -Wall deathstar.cpp -lzmq -o deathstar
