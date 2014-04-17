default: all

all: pagestats

clean:
	rm -f *.o

pagestats: pagestats.o
	g++ -O2 -o pagestats -Wall pagestats.o -lexpat -lpthread

pagestats.o: pagestats.cpp
	g++ -O2 -o pagestats.o -c -Wall pagestats.cpp

debug: dpagestats.o
	g++ -g -o pagestats -Wall dpagestats.o -lexpat -lpthread

dpagestats.o: pagestats.cpp
	g++ -g -o dpagestats.o -c -Wall pagestats.cpp
