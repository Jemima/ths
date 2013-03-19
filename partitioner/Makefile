CC=g++

CFLAGS=-Wall -Wextra -Wno-unknown-pragmas -std=c++11 -c -g
LDFLAGS= -lboost_program_options
SOURCES=Blif.cpp BlifNode.cpp Model.cpp partitioner.cpp Signal.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: partitioner

partitioner: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)
	ctags -R

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm $(OBJECTS)
