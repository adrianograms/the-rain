#ifndef PIPELINE_H
#define PIPELINE_H

#include <SFML/Graphics.hpp>
#include "half_mesh.hpp"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtx/string_cast.hpp>
#include <gtx/transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtx/norm.hpp>

struct Point;
// return as faces por index que sao visiveis
// n é o vetor n da câmera
std::vector<index_t> filter_normal(const half_mesh &terrain, vec3f n);

void guro_face(std::pair <double, sf::Color> **z_buffer, index_t face);

void scanline(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim);

std::vector<double> calc_ilumination(std::vector<index_t> &pontos);

class Pipeline
{
    private:
        glm::mat4 PIPELINE; /// Matriz do glm onde todas as operações são feitas
        glm::mat4 JP;
        glm::mat4 SRC;
        glm::vec3 u, n, v;
        glm::vec3 VRP, FOCAL;

        float **Matriz_Pipeline; /// A mesma matriz PIPELINE (método getMatrix retorna ela)

        float grauXZ, grauXY; /// Grau usado nas rotações

        double raio, a, b;

        bool VRP_verticalSphereDefined;
        bool VRP_horizontalSphereDefined;
        bool FOCAL_verticalSphereDefined;
        bool FOCAL_horizontalSphereDefined;

    public:
        Pipeline();
        void setMatrixJP( float, float, float, float, float, float, float, float );
        void setMatrixSRC( const glm::vec3, const glm::vec3 );
        float** getMatrix();

        void VRP_UP();
        void VRP_DOWN();
        void VRP_LEFT();
        void VRP_RIGHT();

        void FOCAL_UP();
        void FOCAL_DOWN();
        void FOCAL_LEFT();
        void FOCAL_RIGHT();

        void FOCAL_VRP_UP();
        void FOCAL_VRP_DOWN();
        void FOCAL_VRP_LEFT();
        void FOCAL_VRP_RIGHT();

        void redoPipeline();

        float esferaVRP( float x, float y, float z );
        float esferaFOCAL( float x, float y, float z );
        float VRP_bhaskaraX( float a, float b, float c );
        float FOCAL_bhaskaraX( float a, float b, float c );
        float bhaskaraY( float a, float b, float c );
        float bhaskaraZ( float a, float b, float c );
};

#endif
