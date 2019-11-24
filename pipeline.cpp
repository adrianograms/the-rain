#include "include/pipeline.hpp"

// return as faces por index que sao visiveis
// n é o vetor n da câmera
std::vector<index_t> filter_normal(const half_mesh &terrain, vec3f n){
    std::vector<index_t> ret;

    // n . N > 0
    for(index_t i = 0; i < (index_t)terrain.face_vector.size(); i++){
        if(n.dot(terrain.get_face_normal(i)) > 0){
            ret.push_back(i);
        }
    }
    return ret;
}
