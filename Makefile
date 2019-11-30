SFML_LIBS := $(shell pkg-config sfml-all --libs)

OPENGL_LIBS := $(shell pkg-config opengl --libs)

OBJECTS = noise.o half_mesh.o half_edge.o utils.o pipeline.o overlay.o

CXXFLAGS += -std=c++11 -Wall -Wpedantic -g

all: main.cpp $(OBJECTS)
	g++ $(CXXFLAGS) -o main $^ $(SFML_LIBS) $(OPENGL_LIBS) include/button.hpp

$(OBJECTS): include/noise.hpp include/half_edge.hpp include/half_mesh.hpp include/half_types.hpp include/utils.hpp include/pipeline.hpp include/overlay.hpp

.PHONY: clean debug release

test: all
	./main

debug: CXXFLAGS += -g
debug: all

release: CXXFLAGS += -O3
release: all

clean:
	rm -f *.o
	rm -f main
