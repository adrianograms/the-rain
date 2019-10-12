#include "include/half_edge.hpp"

half_edge::half_edge() : next(-1),
                         pair(-1),
                         edge(-1),
                         face(-1),
                         vertex(-1) { }

half_edge &half_edge::operator = (const half_edge &rhs){
    if(this == &rhs){
        return *this;
    }
    this->edge = rhs.edge;
    this->face = rhs.face;
    this->next = rhs.next;
    this->pair = rhs.pair;
    this->vertex = rhs.vertex;
    // TODO: swap this for the sf::vertex
    this->xyz = rhs.xyz;

    return *this;
}
