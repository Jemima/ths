CC=g++

CKRAKENFLAGS=-I/home/dsmu186/boost/boost_1_53_0 -std=c++0x -Wall -Wextra -Wno-unknown-pragmas -c -g
LDKRAKENFLAGS=-L/home/dsmu186/boost/boost_1_53_0/stage/lib

CFLAGS=$(CKRAKENFLAGS)#-Wall -Wextra -Wno-unknown-pragmas -std=c++11 -c -g
LDFLAGS= $(LDKRAKENFLAGS) -lboost_program_options
SOURCES=Blif.cpp BlifNode.cpp Model.cpp partitioner.cpp Signal.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: partitioner

partitioner: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)
	#ctags -R

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm $(OBJECTS)
