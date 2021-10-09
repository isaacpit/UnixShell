# makefile

all: start

start: start.cpp
	g++ -g -w -std=c++11 -o start start.cpp

miguel-start: miguel-start.cpp
	g++ -g -w -std=c++11 -o miguel-start miguel-start.cpp

clean:
	rm start
