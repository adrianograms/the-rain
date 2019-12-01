#include "include/half_mesh.hpp"

#include <iostream>
#include <set>

// ---------- private ----------

// this generates the mesh triangles and the edges
// mx = terrain size on the x axis
// my = terrain size on the y axix
void half_mesh::_gen_objects(std::vector<half::triangle> &triangles, std::vector<half::edge> &edges,
                            uint64_t mx, uint64_t my){

    std::set<std::pair<index_t, index_t>> edge_set;

    for(uint64_t i = 0; i < my - 1; i++){
        for(uint64_t j = 0; j < mx - 1; j++){

            uint64_t p1 = j + i * mx;
            uint64_t p2 = j + (i + 1) * mx;
            uint64_t p3 = (j + i * mx) + 1;
            uint64_t p4 = (j + (i + 1) * mx) + 1;
            // 1---3
            // |  /|
            // | / |
            // |/  |
            // 2---4

            triangles.emplace_back(p1, p2, p3);
            triangles.emplace_back(p3, p2, p4);

            edge_set.insert(std::make_pair(std::min(p1, p2), std::max(p1, p2)));
            edge_set.insert(std::make_pair(std::min(p2, p3), std::max(p2, p3)));
            edge_set.insert(std::make_pair(std::min(p3, p1), std::max(p3, p1)));

            edge_set.insert(std::make_pair(std::min(p3, p2), std::max(p3, p2)));
            edge_set.insert(std::make_pair(std::min(p2, p4), std::max(p2, p4)));
            edge_set.insert(std::make_pair(std::min(p4, p3), std::max(p4, p3)));

        }
    }

    for(auto &edge : edge_set){
        edges.emplace_back(edge.first, edge.second);
    }
}

// ---------- public ----------

half_mesh::half_mesh() { }

// update the z component of each ponit that indexes to a half edge
void half_mesh::update_mesh_z(std::function<vec3f(index_t)> fn){
    points.clear();

    for(uint64_t i = 0; i < vertex_vector.size(); i++){
        points.push_back(fn(i));
    }
}

// generate the half edge mesh
void half_mesh::build_mesh(uint64_t mx, uint64_t my, std::function<vec3f(index_t)> fn){
    uint64_t num_of_verts = mx * my;

    std::map<std::pair<index_t, index_t>, index_t> edge_face_map; // index an edge to its face

    std::vector<half::triangle> triangles;
    std::vector<half::edge> edges;

    // given a direction return a face
    auto direction_face = [&edge_face_map](std::pair<index_t, index_t> key) -> index_t {
        return edge_face_map.find(key) == edge_face_map.end() ? -1 :  edge_face_map[key];
    };

    _gen_objects(triangles, edges, mx, my);

    {
        index_t i = 0;
        for(auto &t : triangles){
            // map all the edges of a triangle to its index to create the face map
            edge_face_map[std::make_pair(t.v[0], t.v[1])] = i;
            edge_face_map[std::make_pair(t.v[1], t.v[2])] = i;
            edge_face_map[std::make_pair(t.v[2], t.v[0])] = i;

            i++;
        }
    }

    clear_mesh();
    vertex_vector.resize(num_of_verts,   -1);
    face_vector.resize(triangles.size(), -1);
    edge_vector.resize(edges.size(),     -1);

    // create the half_edges
    for(index_t hei = 0, i = 0; i < (index_t)edges.size(); ++i){

        const half::edge &e = edges[i];

        index_t index_e = hei++;
        half_vector.push_back(half_edge());
        half_edge half_e;

        index_t index_p = hei++;
        half_vector.push_back(half_edge());
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
        edge_index_map[dir_one] = index_e;
        edge_index_map[dir_two] = index_p;

        if(vertex_vector[half_e.vertex] == -1 || half_p.face == -1){
            vertex_vector[half_e.vertex] = half_e.pair;
        }
        if(vertex_vector[half_p.vertex] == -1 || half_e.face == -1){
            vertex_vector[half_p.vertex] = half_p.pair;
        }
        // if the face is not yet stored, store it
        if(half_e.face != -1 && face_vector[half_e.face] == -1){
            face_vector[half_e.face] = index_e;
        }
        if(half_p.face != -1 && face_vector[half_p.face] == -1){
            face_vector[half_p.face] = index_p;
        }

        edge_vector[i] = index_e;
        half_vector[index_e] = half_e;
        half_vector[index_p] = half_p;
    }
    // store half_edges with no faces
    std::vector<index_t> bound_half;
    for(index_t i = 0; i < (index_t)half_vector.size(); i++){
        half_edge &hedge = half_vector[i];

        if(hedge.face == -1){
            bound_half.push_back(i);
            continue;
        }

        const half::triangle &face = triangles[hedge.face];
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

        hedge.next = edge_index_map[{curr, next}];
    }

    std::map<index_t, std::set<index_t>> vertex_to_bound;
    for(auto &bound : bound_half){
        index_t start = half_vector[half_vector[bound].pair].vertex;
        vertex_to_bound[start].insert(bound);
    }

    for(auto &bound : bound_half){
        half_edge &hedge = half_vector[bound];

        std::set<index_t> &out = vertex_to_bound[hedge.vertex];
        if(!out.empty()){
            std::set<index_t>::iterator out_half = out.begin();

            hedge.next = *out_half;

            out.erase(out_half);
        }
    }

    update_mesh_z(fn);
}

// reset the mesh
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

std::pair<index_t, index_t> half_mesh::half_direction(index_t i) const {
     half_edge half = this->half_vector[i];
     return std::make_pair(this->half_vector[half.pair].vertex, half.vertex);
}

index_t half_mesh::direction_half(std::pair<index_t, index_t> dir){
    return this->edge_index_map[dir];
}

std::vector<index_t> half_mesh::vertex_vertexes(index_t i) const {
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

std::vector<index_t> half_mesh::vertex_faces(index_t i) const {
    std::vector<index_t> ret;

    index_t half_start;
    index_t half_next;

    half_start = half_next = this->vertex_vector[i];

    do{
        const half_edge half_at = half_vector[half_next];
        if(half_at.face != -1){
            ret.push_back(half_at.face);
        }

        half_next = this->half_vector[half_at.pair].next;
    }while(half_start != half_next);

    return ret;
}
// param i is the vertex number
std::vector<index_t> half_mesh::get_vertex_faces(index_t i) const {
    std::vector<index_t> ret;

    index_t start;
    index_t curr;

    start = curr = vertex_vector[i];

    do{
        const half_edge half = half_vector[curr];
        if(half.face != -1){
            ret.push_back(half.face);
        }

        curr = half_vector[half.pair].next;

    }while(start != curr);
    return ret;
}

// param i is the face number
std::vector<index_t> half_mesh::get_face_vertexes(index_t i) const {
    std::vector<index_t> ret;

    index_t start_he = i;
    index_t curr_he  = i;

    do{
        const auto &he = half_vector[curr_he];
        ret.push_back(he.vertex);

        curr_he = he.next;

    }while(start_he != curr_he);

    return ret;
}

vec3f half_mesh::get_face_normal(index_t i) const {
    auto vf = get_face_vertexes(i);

    vec3f A = points[vf[0]];
    vec3f B = points[vf[1]];
    vec3f C = points[vf[2]];

    vec3f BA = A - B;
    vec3f BC = C - B;

    return vec3f(BA.cross(BC).norm());
}

bool half_mesh::vertex_boundary(index_t i) const {
    return this->half_vector[this->vertex_vector[i]].face == -1;

}
