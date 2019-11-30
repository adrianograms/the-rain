#ifndef HALF_EDGE_H
#define HALF_EDGE_H

#include "half_types.hpp"

struct half_edge{
    // index into the half_vector
    index_t next;
    // index into the half_vector
    index_t pair;
    // index into the edge_vector
    index_t edge;
    // index into the face_vector
    index_t face;
    // index int the vertex_vector
    index_t vertex;

    half_edge();

    half_edge &operator = (const half_edge &);
};

#endif
