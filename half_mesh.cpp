#include "include/half_mesh.hpp"

#include <iostream>
#include <set>

// ---------- private ----------

// this generates the mesh triangles
// mx = terrain size on the x axis
// my = terrain size on the y axix
void half_mesh::_gen_mesh_triangles(std::vector<triangle> &triangles, uint64_t mx, uint64_t my){

    triangles.clear();

    for(uint64_t i = 0; i < my - 1; i++){
        for(uint64_t j = 0; j < mx - 1; j++){
            // 1---3
            // |  /
            // | /
            // |/
            // 2
            triangles.emplace_back(j + i * mx, j + (i + 1) * mx, (j + i * mx) + 1);

            //     1
            //    /|
            //   / |
            //  /  |
            // 2---3
            triangles.emplace_back((j + i * mx) + 1, j + (i + 1) * mx, (j + (i + 1) * mx) + 1);
        }
    }
}

// ---------- public ----------

half_mesh::half_mesh() { }


// clear all the stuff, it is better to just make a new mesh
void half_mesh::clear_mesh(){
    this->half_vector.clear();
    this->vertex_vector.clear();
    this->face_vector.clear();
    this->edge_vector.clear();
    this->edge_index_map.clear();
}

const
half_edge &half_mesh::half_at(index_t i) const {
    return this->half_vector[i];
}

std::pair<index_t, index_t> half_mesh::half_direction(index_t i){
     half_edge half = this->half_vector[i];
     return std::make_pair(this->half_vector[half.pair].vertex, half.vertex);
}

index_t half_mesh::direction_half(std::pair<index_t, index_t> dir){
    return this->edge_index_map[dir];
}

std::vector<index_t> half_mesh::vertex_vertexes(index_t i){
    std::vector<index_t> ret;

    index_t half_start;
    index_t half_next;

    half_start = half_next = this->vertex_vector[i];

    do{
        const half_edge half_at = this->half_vector[i];
        ret.push_back(half_at.vertex);

        half_next = this->half_vector[half_at.pair].next;
    }while(half_start != half_next);

    return ret;
}

std::vector<index_t> half_mesh::vertex_faces(index_t i){
    std::vector<index_t> ret;

    index_t half_start;
    index_t half_next;

    half_start = half_next = this->vertex_vector[i];

    do{
        const half_edge half_at = this->half_vector[i];
        if(half_at.face != -1){
            ret.push_back(half_at.vertex);
        }

        half_next = this->half_vector[half_at.pair].next;
    }while(half_start != half_next);

    return ret;
}

bool half_mesh::vertex_boundary(index_t i){
    return this->half_vector[this->vertex_vector[i]].face == -1;

}
