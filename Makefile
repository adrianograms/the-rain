SFML_LIBS := $(shell pkg-config sfml-all --libs)

OBJECTS = noise.o half_mesh.o half_edge.o

CXXFLAGS += -std=c++11 -Wall -g -fopenmp -O3

all: main.cpp $(OBJECTS)
	g++ $(CXXFLAGS) -o main $^ $(SFML_LIBS) $(INCLUDE)

$(OBJECTS): include/noise.hpp include/half_edge.hpp include/half_mesh.hpp include/half_types.hpp

.PHONY: clean 

clean:
	rm -f *.o
	rm -f main
