#ifndef HALF_TYPES_H
#define HALF_TYPES_H

#include <cstdint>

// index type, into the half_vector
typedef int64_t index_t;

namespace half{
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
            v[0] = rhs.v[0];
            v[1] = rhs.v[1];
            v[2] = rhs.v[2];

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
            e[0] = rhs.e[0];
            e[1] = rhs.e[1];

            return *this;
        }
    };
}

// TODO: finish
class point{
public:
    float p[3];

    point();

    point(float i, float j, float k){
        p[0] = i;
        p[1] = j;
        p[2] = k;
    }

    float x(){
        return p[0];
    }

    float y(){
        return p[1];
    }

    float z(){
        return p[2];
    }
};

// TODO: finish
class triangle{
    index_t he[3];

    point cords[3];
};


typedef enum {WIREFRAME, FLAT, GOURAUD} view_mode;

#endif
