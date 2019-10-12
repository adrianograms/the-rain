#ifndef HALF_TYPES_H
#define HALF_TYPES_H

#include <cstdint>

// index type, into the half_vector
typedef int64_t index_t;

class triangle{
public:
    // index into the half edge vector
    index_t v[3];

    triangle(){
        v[0] = v[1] = v[2] = -1;
    }

    triangle(index_t i, index_t j, index_t k){
        v[0] = i;
        v[1] = j;
        v[2] = k;
    }

    triangle &operator = (const triangle &rhs){
        if(this == &rhs){
            return *this;
        }
        this->v[0] = rhs.v[0];
        this->v[1] = rhs.v[1];
        this->v[2] = rhs.v[2];

        return *this;
    }
};

class edge{
public:
    // index into the half edge vector
    index_t e[2];

    edge(){
        e[0] = e[1] = -1;
    }

    edge(index_t i, index_t j){
        e[0] = i;
        e[1] = j;
    }

    edge &operator = (const edge &rhs){
        if(this == &rhs){
            return *this;
        }
        this->e[0] = rhs.e[0];
        this->e[1] = rhs.e[1];

        return *this;
    }
};

#endif
