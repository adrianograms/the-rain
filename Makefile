SFML_LIBS := $(shell pkg-config sfml-all --libs)

OBJECTS = noise.o half_mesh.o half_edge.o

INCLUDE = include/half_types.hpp

CXXFLAGS += -std=c++11 -Wall -g -fopenmp

all: main.cpp $(OBJECTS)
	g++ -std=c++11 -Wall -o main $^ $(SFML_LIBS) -g -fopenmp $(INCLUDE)

$(OBJECTS): include/noise.hpp include/half_edge.hpp include/half_mesh.hpp

.PHONY: clean

clean:
	rm -f *.o
	rm -f main
