#ifndef PIPELINE_H
#define PIPELINE_H

#include <SFML/Graphics.hpp>
#include "half_mesh.hpp"

struct Point;
// return as faces por index que sao visiveis
// n é o vetor n da câmera
std::vector<index_t> filter_normal(const half_mesh &terrain, vec3f n);

void guro_face(std::pair <double, sf::Color> **z_buffer, index_t face);

void scanline(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim);

std::vector<double> calc_ilumination(std::vector<index_t> &pontos);

#endif
