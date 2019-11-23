#ifndef HALF_MESH_H
#define HALF_MESH_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <cassert>
#include <tuple>
#include <functional>

#include "half_types.hpp"
#include "half_edge.hpp"

// based on trimesh from https://github.com/yig/halfedge

class half_mesh{

    void _gen_objects(std::vector<half::triangle> &, std::vector<half::edge> &, uint64_t, uint64_t);
    void _update_normal();
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


    std::vector<half_edge> half_vector;

    std::vector<index_t> vertex_vector;

    std::vector<index_t> face_vector;

    std::vector<index_t> edge_vector;

    std::map<std::pair<index_t, index_t>, index_t> edge_index_map;

    std::vector<point> points;
    std::vector<point> normals;

    half_mesh();

    void clear_mesh();


    // the value of x:y must be a single number following x + y * mx;
    void build_mesh(uint64_t, uint64_t, std::function<point(index_t)>);

    void update_mesh_z(std::function<point(index_t)>);

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


    std::vector<index_t> get_vertex_faces(index_t);
    std::vector<index_t> get_face_vertexes(index_t);
};

// ---------- public ----------

#endif
