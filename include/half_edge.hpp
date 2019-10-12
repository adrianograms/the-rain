#ifndef HALF_EDGE_H
#define HALF_EDGE_H
// TODO: swap this for the sf::Vertex
#include <tuple>

#include "half_types.hpp"

class half_edge{
    // all public for now
public:
    // coordenate of the vertex at the start of the half_edge
    std::tuple<float, float, float> xyz;

    index_t next;

    index_t pair;

    index_t edge;

    index_t face;

    index_t vertex;

    half_edge();

    half_edge &operator = (const half_edge &);
};

#endif
