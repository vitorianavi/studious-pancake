all: all_p

all_p: database.o parser.o dashboard.o
	g++ database.o parser.o dashboard.o -O3 -o all_p -lpqxx -lpq

dashboard.o: dashboard.cpp
	g++ -c dashboard.cpp -std=c++11 -g

parser.o: parser.cpp
	g++ -c parser.cpp -std=c++11 -g

database.o: database.cpp
	g++ -c database.cpp -std=c++11 -g

clean:
	rm *o all_p
