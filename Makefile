SFML_LIBS := $(shell pkg-config sfml-all --libs)

OBJECTS = noise.o half_mesh.o half_edge.o utils.o pipeline.o

CXXFLAGS += -std=c++11 -Wall -Wpedantic -g

all: main.cpp $(OBJECTS)
	g++ $(CXXFLAGS) -o main $^ $(SFML_LIBS) include/button.hpp

$(OBJECTS): include/noise.hpp include/half_edge.hpp include/half_mesh.hpp include/half_types.hpp include/utils.hpp include/pipeline.hpp



.PHONY: clean

clean:
	rm -f *.o
	rm -f main
