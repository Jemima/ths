CC=g++

CFLAGS=-Wall -O -Werror -std=c++11 -c
LDFLAGS=-O -lboost_program_options
SOURCES=Blif.cpp BlifNode.cpp Model.cpp partitioner.cpp Signal.cpp
OBJECTS=$(SOURCES:.cpp=.o)

all: partitioner

partitioner: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS)


.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean

clean:
	rm $(OBJECTS)
