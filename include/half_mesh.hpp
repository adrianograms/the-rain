#ifndef HALF_MESH_H
#define HALF_MESH_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <cassert>
#include <tuple>

#include "half_types.hpp"
#include "half_edge.hpp"

// based on trimesh from https://github.com/yig/halfedge

class half_mesh{
private:

    void _gen_mesh_triangles(std::vector<triangle> &, uint64_t, uint64_t);

    // public for now
public:
    // basic extructure of each face
    //                              <---- half edge from 3 to 1
    //
    //                           1 -------- 3
    //                           |        /
    //                           |       /
    //                        |  |      /   /\ half edge from 2 to 3
    //                        |  |  ↺  /  //
    // half edge from 1 to 2 \|  |    /  //
    //                           |   / \/ pair edege
    //                           |  /
    //                           | /
    //                           |/
    //                           2

    // the mesh is formed by the regular triangles
    // 1 --------2   for a mesh 2 x 2
    // |        /|
    // |       / |   the number of half_edges will be 9
    // |      /  |   the number of vextexes will be 4
    // |     /   |   the number of edges will be 5
    // |    /    |   the number of faces will be 2
    // |   /     |
    // |  /      |
    // | /       |
    // |/        |
    // 3---------4

    // this vector contains all the half edges
    std::vector<half_edge> half_vector;

    // index into the half_vector for a given edge
    std::vector<index_t> vertex_vector;

    // index into the half_vector for a face
    // each face will index into a half vector of the face
    std::vector<index_t> face_vector;

    // index into the half_vetor for a given edge
    std::vector<index_t> edge_vector;

    std::map<std::pair<index_t, index_t>, index_t> edge_index_map;

    std::vector<std::tuple<float, float ,float>> vertexes;

    half_mesh();

    void clear_mesh();

    // F is the function to call to get the z value for the point at x:y
    // the value of x:y must be a single number following x + y * mx;
    template<typename F>
    void build_mesh(uint64_t, uint64_t, F);

    template<typename F>
    void update_mesh_z(F);

    // get the half_edge of the index
    const half_edge &half_at(index_t) const;
    // get the direction of the index
    std::pair<index_t, index_t> half_direction(index_t);
    // get the index of the direction
    index_t direction_half(std::pair<index_t, index_t>);
    // get the neighbors vertexes of the vertex
    std::vector<index_t> vertex_vertexes(index_t);
    // get the neightbors faces of the vertex
    std::vector<index_t> vertex_faces(index_t);
    // is the vertex a boundary
    bool vertex_boundary(index_t);
};

// ---------- public ----------

// update the z component of each half edge
template<typename F>
void half_mesh::update_mesh_z(F fn){

    this->vertexes.clear();

    for(uint64_t i = 0; i < vertex_vector.size(); i++){
        this->vertexes.push_back(fn(i));
    }

}

// generate the half edge mesh
template<typename F>
void half_mesh::build_mesh(uint64_t mx, uint64_t my, F fn){
    uint64_t num_of_verts = mx * my;

    std::map<std::pair<index_t, index_t>, index_t> edge_face_map;
    std::vector<triangle> triangles;
    std::vector<edge> edges;

    // get all the edges from the triangles
    auto triangle_to_edge = [&triangles, &edges](){
        std::set<std::pair<index_t, index_t>> edge_set;
        for(auto &t : triangles){
            edge_set.insert(std::make_pair(std::min(t.v[0], t.v[1]), std::max(t.v[0], t.v[1])));
            edge_set.insert(std::make_pair(std::min(t.v[1], t.v[2]), std::max(t.v[1], t.v[2])));
            edge_set.insert(std::make_pair(std::min(t.v[2], t.v[0]), std::max(t.v[2], t.v[0])));
        }
        for(auto &p : edge_set){
            edges.emplace_back(p.first, p.second);
        }
    };

    // given a direction return a face
    auto direction_face = [&edge_face_map](std::pair<index_t, index_t> key) -> index_t {
        return edge_face_map.find(key) == edge_face_map.end() ? -1 :  edge_face_map[key];
    };

    this->_gen_mesh_triangles(triangles, mx, my);

    triangle_to_edge();

    // create the face map
    for(index_t i = 0; i < (index_t)triangles.size(); i++){
        const triangle &tri = triangles[i];

        edge_face_map[std::make_pair(tri.v[0], tri.v[1])] = i;
        edge_face_map[std::make_pair(tri.v[1], tri.v[2])] = i;
        edge_face_map[std::make_pair(tri.v[2], tri.v[0])] = i;
    }

    this->clear_mesh();
    this->vertex_vector.resize(num_of_verts,   -1);
    this->face_vector.resize(triangles.size(), -1);
    this->edge_vector.resize(edges.size(),     -1);

    // create the half_edges
    this->half_vector.clear();
    index_t hei = 0;            // half_edge index
    for(index_t i = 0; i < (index_t)edges.size(); ++i){

        const edge &e = edges[i];

        index_t index_e = hei++;
        this->half_vector.push_back(half_edge());
        half_edge half_e;

        index_t index_p = hei++;
        this->half_vector.push_back(half_edge());
        half_edge half_p;

        std::pair<index_t, index_t> dir_one = std::make_pair(e.e[0], e.e[1]);
        std::pair<index_t, index_t> dir_two = std::make_pair(e.e[1], e.e[0]);

        half_e.face = direction_face(dir_one);
        half_e.vertex = e.e[1];
        half_e.edge = i;
        half_e.pair = index_p;

        half_p.face = direction_face(dir_two);
        half_p.vertex = e.e[0];
        half_p.edge = i;
        half_p.pair = index_e;

        // store the direction in the index to dir map
        assert(this->edge_index_map.find(dir_one) == this->edge_index_map.end());
        assert(this->edge_index_map.find(dir_two) == this->edge_index_map.end());
        this->edge_index_map[dir_one] = index_e;
        this->edge_index_map[dir_two] = index_p;

        if(this->vertex_vector[half_e.vertex] == -1 || half_p.face == -1){
            assert(half_e.pair != -1);
            this->vertex_vector[half_e.vertex] = half_e.pair;
        }
        if(this->vertex_vector[half_p.vertex] == -1 || half_e.face == -1){
            assert(half_p.pair != -1);
            this->vertex_vector[half_p.vertex] = half_p.pair;
        }
        // if the face is not yet stored, store it
        if(half_e.face != -1 && this->face_vector[half_e.face] == -1){
            this->face_vector[half_e.face] = index_e;
        }
        if(half_p.face != -1 && this->face_vector[half_p.face] == -1){
            this->face_vector[half_p.face] = index_p;
        }

        assert(half_p.vertex != -1);
        assert(half_e.vertex != -1);
        assert(this->edge_vector[i] == -1);

        this->edge_vector[i] = index_e;
        this->half_vector[index_e] = half_e;
        this->half_vector[index_p] = half_p;
    }
    // store half_edges with no faces
    std::vector<index_t> bound_half;
    for(index_t i = 0; i < (index_t)this->half_vector.size(); i++){
        half_edge &hedge = this->half_vector[i];

        if(hedge.face == -1){
            bound_half.push_back(i);
            continue;
        }

        const triangle &face = triangles[hedge.face];
        const index_t curr = hedge.vertex;

        index_t next = -1;
        if(face.v[0] == curr){
            next = face.v[1];
        }
        else if(face.v[1] == curr){
            next = face.v[2];
        }
        else if(face.v[2] == curr){
            next = face.v[0];
        }
        assert(next != -1);

        hedge.next = this->edge_index_map[std::make_pair(curr, next)];
    }

    std::map<index_t, std::set<index_t>> vertex_to_bound;
    for(auto &bound : bound_half){
        index_t start = this->half_vector[this->half_vector[bound].pair].vertex;
        vertex_to_bound[start].insert(bound);
    }

    for(auto &bound : bound_half){
        half_edge &hedge = this->half_vector[bound];

        std::set<index_t> &out = vertex_to_bound[hedge.vertex];
        if(!out.empty()){
            std::set<index_t>::iterator out_half = out.begin();

            hedge.next = *out_half;

            out.erase(out_half);
        }
    }
    // print all the shit
    {
        std::cout << "triangles:" << std::endl;
        std::size_t i = 0;
        for(auto &t : triangles){
            std::cout << i << " ["  << t.v[0] << ", " << t.v[1] << ", " << t.v[2] << "]" << std::endl;
            i++;
        }
        std::cout << std::endl;
    }{
        std::cout << "edges:" << std::endl;
        std::size_t i = 0;
        for(auto &e : edges){
            std::cout << i << " ["  << e.e[0] << ", " << e.e[1] << "]" << std::endl;
            i++;
        }
        std::cout << std::endl;
    }{
        std::cout << "vertexes:" << std::endl;
        std::size_t i = 0;
        for(auto &v : this->vertex_vector){
            std::cout << i << " [" << v << "]" << std::endl;
            i++;
        }
        std::cout << std::endl;
    }{
        std::cout << "faces:" << std::endl;
        std::size_t i = 0;
        for(auto &f : this->face_vector){
            std::cout << i << " [" << f << "]" << std::endl;
            i++;
        }
        std::cout << std::endl;
    }{
        std::cout << "edges:" << std::endl;
        std::size_t i = 0;
        for(auto &e : this->edge_vector){
            std::cout << i << " [" << e << "]" << std::endl;
            i++;
        }
        std::cout << std::endl;
    }{
        std::cout << "half_edges:" << std::endl;
        std::size_t i = 0;
        for(auto &h : this->half_vector){
            std::cout << i
                      << " [vertex: " << h.vertex
                      << ", pair: " << h.pair
                      << ", face: " << h.face
                      << ", next: " << h.next << "]" << std::endl;
            i++;
        }
        std::cout << std::endl;
    }

    this->update_mesh_z(fn);

    {
        std::cout << "vertexes:" << std::endl;
        std::size_t i = 0;
        for(auto &h : this->vertexes){
            std::cout << i
                      << " ["
                      << std::get<0>(h)
                      << ", "
                      << std::get<1>(h)
                      << ", "
                      << std::get<2>(h)
                      << "]"
                      << std::endl;
            i++;
        }
        std::cout << std::endl;
    }
}

#endif
