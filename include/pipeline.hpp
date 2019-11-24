#ifndef PIPELINE_H
#define PIPELINE_H

#include "half_mesh.hpp"

// return as faces por index que sao visiveis
// n é o vetor n da câmera
std::vector<index_t> filter_normal(const half_mesh &terrain, vec3f n);

#endif
