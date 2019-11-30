#include "include/pipeline.hpp"

#include <iostream>
#include <string>
#include "math.h"

# define PI          3.14159265358979323846

/// Métodos do glm
///
///     l2Norm retorna a norma de um vetor
///     dot retorna o produto escalar entre dois vetores
///     cross retorna o produto vetorial entre dois vetores
///     to_string() sobrecarregado para vetores e matrizes, caso seja necessário a visualização
///     distance calcula a distância entre dois vetores (pontos)
///
/// ------------ OBSERVAÇÃO ---------------
///
/// NO GLM, AS MATRIZES SÃO MULTIPLICADAS NA ORDEM >>INVERSA<<
/// OU SEJA, DUAS MATRIZES DO TIPO mat4 SÃO MULTIPLICADAS NA ORDEM A * B, QUE NA VERDADE PRODUZEM RESULTADOS DE B * A
///
/// Para as mudanças em VRP/FOCAL acontecerem, é preciso de todos os pontos no SRU >intactos<
/// Os pontos após a multiplicação da matriz PIPELINE recalculada a cada alteração são definidos
/// a partir da multiplicação pelos pontos no SRU
///
///
/// ->>>> PARA PEGAR A MATRIZ COM PONTOS, USAR O MÉTODO getMatriz(), QUE RETORNA UM PONTEIRO COM A MATRIZ


/// Ao mover para cima ou para baixo o VRP/FOCO, é preciso calcular o valor de b de uma elipse
/// Essa elipse pode estar tanto no eixo XY como no eixo YZ
/// Aqui foi implementando no eixo YZ
/// O método abaixo descobre o valor de b e o retorna
float discoverB( float a , glm::vec3 VRP )
{
    float tolerancia = 0.01;
    float c;

    for(c = 0.001; c < a; c += 0.001)
    {
        glm::vec3 F1(0, c, 0);
        glm::vec3 F2(0, -c, 0);

        glm::vec3 P( VRP.z , VRP.y, 0 );

        float distancia_f1 = distance( F1, P );
        float distancia_f2 = distance( F2, P );
        float distancia_focos = 2*a - ( distancia_f1 + distancia_f2 );

        if( abs(distancia_focos) < tolerancia )
        {
            // std::cout << "VALOR DE C = " << c << std::endl;

            return sqrt( pow(a,2) - pow(c,2) );
        }
    }

    return 0.0;
}

/// MÉTODOS DA CLASSE PIPELINE

Pipeline::Pipeline()
{
    Matriz_Pipeline = new float*[4];
    for(int i = 0; i < 4; i++)
        Matriz_Pipeline[i] = new float[4];
}

/// Constrói a matriz JP usada no Pipeline
void Pipeline::setMatrixJP( float xMin, float xMax, float yMin, float yMax, float uMin , float uMax, float vMin, float vMax )
{

    JP[0][0] = ( uMax - uMin) / ( xMax - xMin );
    JP[1][1] = ( vMin - vMax) / ( yMax - yMin );
    JP[0][3] = -xMin * ( uMax - uMin ) / ( xMax - xMin ) + uMin;
    JP[1][3] = yMin * ( vMax - vMin ) / ( yMax - yMin ) + vMax;

    JP[0][1] = JP[0][2] = JP[1][0] = JP[1][2] = JP[2][0] = JP[2][1] = JP[2][3] = JP[3][0] = JP[3][1] = JP[3][2] = 0;
    JP[2][2] = JP[3][3] = 1;

    // std::cout << glm::to_string(JP) << std::endl;
}

/// Constrói a matriz SRC usada no Pipeline
void Pipeline::setMatrixSRC( glm::vec3 V, glm::vec3 F )
{
    VRP = V;
    FOCAL = F;

    n = ( VRP - FOCAL );
    n = n / l2Norm(n);

    glm::vec3 Y( 0, 1, 0 );

    v = Y - ( dot(n, Y) * n ) ;
    v = v / l2Norm(v);

    u = cross(v , n);

    glm::mat4 SRC_(

        u[0], u[1], u[2], -dot( VRP, u ),
        v[0], v[1], v[2], -dot( VRP, v ),
        n[0], n[1], n[2], -dot( VRP, n ),
           0,    0,    0,              1

    );

    SRC = SRC_;
    // std::cout << glm::to_string(SRC) << std::endl;
}

/// Após a mudança de VRP/FOCAL é preciso definir a matriz SRC novamente
void Pipeline::redoPipeline()
{
    n = ( VRP - FOCAL );
    n = n / l2Norm(n);

    glm::vec3 Y( 0, 1, 0 );

    v = Y - ( dot(n, Y) * n ) ;
    v = v / l2Norm(v);

    u = cross(v , n);

    glm::mat4 SRC_(

        u[0], u[1], u[2], -dot( VRP, u ),
        v[0], v[1], v[2], -dot( VRP, v ),
        n[0], n[1], n[2], -dot( VRP, n ),
           0,    0,    0,              1

    );

    SRC = SRC_;

    // std::cout << glm::to_string(SRC) << std::endl;
}

float** Pipeline::getMatrix()
{
    PIPELINE = SRC * JP;

    // std::cout << glm::to_string(PIPELINE) << std::endl;

    for( int i = 0 ; i < 4 ; i++)
    {
        for( int j = 0 ; j < 4 ; j++)
            Matriz_Pipeline[i][j] = PIPELINE[i][j];

    }

    return Matriz_Pipeline;
}

/// As transformações de câmera envolvem o ponto Focal e o VRP em uma ESFERA
///
/// -> Ir para direita/esquerda significa cortar um plano XZ em relação a altura de VRP/FOCAL (depende de qual está sendo movimentado)
///    definindo uma circunferência na esfera. É nessa esfera que o VRP/FOCAL se movimenta (de raio igual a distância no plano XZ
///    entre VRP e FOCAL)
///
/// -> Ir para cima/baixo significa cortar um plano perpendicular a Y, também definindo uma circunferência na esfera no qual
///    o VRP/FOCAL se movimenta (de raio igual a esfera).
///
/// Para tais cálculos, é preciso definir o ângulo formado entre os eixos e a reta VRP-FOCAL.
///     A partir desse ângulo, se calcula todos os novos valores de VRP/FOCAL.
///
///     Para a movimentação cima/baixo, o cálculo de 'y' é facilmente encontrado, mas para a definição de 'x' ou 'z' é preciso de uma elipse,
///     que é formada a partir do corte da circunferência perpendicular a Y (devido essa circunferência, na grande maioria dos casos,
///     não fazer parte de alguns dos eixos principais (XY, YZ), ao olhar para X ou Z constante (no código adotado X constante,
///     a circunferência forma uma elipse, com 'a' no eixo principal com valor igual ao raio da esfera, e b calculado através de um
///     método numérico. Com isso, definimos dois valores (y e z). O valor de 'x' é calculado através da equação da esfera.
///

void Pipeline::VRP_UP( )
{
    VRP_horizontalSphereDefined = FOCAL_horizontalSphereDefined = FOCAL_verticalSphereDefined = false;

    if( !VRP_verticalSphereDefined )
    {
        glm::vec3 F = VRP - FOCAL;

        glm::vec3 TerceiroPontoPlano; /// Y = 0
        TerceiroPontoPlano = VRP;
        TerceiroPontoPlano.y = FOCAL.y;

        float sizeX = distance( FOCAL, TerceiroPontoPlano );
        float sizeY = distance( VRP, TerceiroPontoPlano );

        grauXY = round (atan( sizeY / sizeX ) * 180/PI );

        if( F.z < 0 && F.y > 0 )
            grauXY = 180 - grauXY;
        else if( F.z < 0 && F.y < 0 )
            grauXY = 180 + grauXY;
        else if( F.z > 0 && F.y < 0 )
            grauXY = 360 - grauXY;

        raio = distance(VRP, FOCAL);

        a = raio;
        b = discoverB( a , (VRP - FOCAL) ) ;
        // std::cout << "B = " << b << "\n";

        // std::cout << "R = " << raio << "\n";

        VRP_verticalSphereDefined = true;
    }

    if(grauXY == 90)
    {
        return;
    }
    else if( grauXY > 270 || ( grauXY > 0 && grauXY < 90 ) )
    {
        grauXY++;
    }
    else
    {
        grauXY--;
    }

    if(grauXY == 90)  /// 90 graus é o máximo permitido para subida
        return;

    // std::cout << "GRAU = " << grauXY << std::endl;


    if(grauXY == 360)
        grauXY = 0;

    VRP.y = (  sin( (grauXY) * PI/180 ) * raio + FOCAL.y );


    if( grauXY > 270 || ( grauXY > 0 && grauXY < 90 ) )
        VRP.z = ( b * sqrt( pow(a,2) - pow(VRP.y,2) + 2*VRP.y*FOCAL.y - pow(FOCAL.y,2) ) / a + FOCAL.z);
    else
        VRP.z = ( -b * sqrt( pow(a,2) - pow(VRP.y,2) + 2*VRP.y*FOCAL.y - pow(FOCAL.y,2) ) / a + FOCAL.z);

    VRP.x = (  esferaVRP(0 , VRP.y, VRP.z) );

    // std::cout << "VRP = ";
    // std::cout << glm::to_string(VRP) << std::endl;

    redoPipeline();
}

void Pipeline::VRP_DOWN( )
{
    VRP_horizontalSphereDefined = FOCAL_horizontalSphereDefined = FOCAL_verticalSphereDefined = false;

    // std::cout << "VRP = ";
    // std::cout << glm::to_string(VRP) << std::endl;

    if( !VRP_verticalSphereDefined )
    {
        glm::vec3 F = VRP - FOCAL;

        glm::vec3 TerceiroPontoPlano; /// Y = 0
        TerceiroPontoPlano = VRP;
        TerceiroPontoPlano.y = ( FOCAL.y );

        float sizeX = distance( FOCAL, TerceiroPontoPlano );
        float sizeY = distance( VRP, TerceiroPontoPlano );

        grauXY = round (atan( sizeY / sizeX ) * 180/PI );

        if( F.z < 0 && F.y > 0 )
            grauXY = 180 - grauXY;
        else if( F.z < 0 && F.y < 0 )
            grauXY = 180 + grauXY;
        else if( F.z > 0 && F.y < 0 )
            grauXY = 360 - grauXY;

        // std::cout << "GRAU CALCULADO = " << grauXY << "\n";

        raio = distance(VRP, FOCAL);

        a = raio;
        b = discoverB( a , (VRP - FOCAL) ) ;
        // std::cout << "B = " << b << "\n";

        // std::cout << "R = " << raio << "\n";

        VRP_verticalSphereDefined = true;
    }

    if(grauXY == 270)
    {
        return;
    }
    else if( grauXY > 270 || ( grauXY > 0 && grauXY < 90 ) )
    {
        grauXY--;
    }
    else
    {
        grauXY++;
    }

    if(grauXY == 270)   /// 270 graus é o máximo permitido para descida
        return;

    // std::cout << "GRAU = " << grauXY << std::endl;


    if(grauXY == 360)
        grauXY = 0;

    VRP.y = (  sin( (grauXY) * PI/180 ) * raio + FOCAL.y );


    if( grauXY > 270 || ( grauXY > 0 && grauXY < 90 ) )
        VRP.z = ( b * sqrt( pow(a,2) - pow(VRP.y,2) + 2*VRP.y*FOCAL.y - pow(FOCAL.y,2) ) / a + FOCAL.z);
    else
        VRP.z = ( -b * sqrt( pow(a,2) - pow(VRP.y,2) + 2*VRP.y*FOCAL.y - pow(FOCAL.y,2) ) / a + FOCAL.z);

    VRP.x = (  esferaVRP(0 , VRP.y, VRP.z) );

    // std::cout << "VRP = ";
    // std::cout << glm::to_string(VRP) << std::endl;

    redoPipeline();
}

void Pipeline::VRP_LEFT( )
{
    VRP_verticalSphereDefined = FOCAL_horizontalSphereDefined = FOCAL_verticalSphereDefined = false;

    glm::vec3 F = VRP - FOCAL;

    if( !VRP_horizontalSphereDefined )
    {
        grauXZ = round( atan( abs(F.x) / abs(F.z) ) * 180/PI );

        /// PRIMEIRO QUADRANTE é o próprio grau calculado

        /// GRAU XZ
        if( F.x > 0 && F.z < 0 ) /// SEGUNDO QUADRANTE
            grauXZ = 180 - grauXZ;
        else if( F.x < 0 && F.z < 0 ) /// TERCEIRO QUADRANTE
            grauXZ = 180 + grauXZ;
        else if( F.x < 0 && F.z >= 0 ) /// QUARTO QUADRANTE
            grauXZ = 360 - grauXZ;

        VRP_horizontalSphereDefined = true;
    }

    double r = sqrt( pow(F.x,2) + pow(F.z,2) );

    // std::cout << "TAMANHO = " << r << "\n";

    // std::cout << "MOVEU PARA ESQUERDA - GRAU = " << grauXZ << "\n";

    grauXZ--;

    VRP.x = (  sin( (grauXZ) * PI/180 ) * r + FOCAL.x);
    VRP.z = (  cos( (grauXZ) * PI/180 ) * r + FOCAL.z);

    // std::cout << "VRP = ";
    // std::cout << glm::to_string(VRP) << std::endl;

    redoPipeline();

}

void Pipeline::VRP_RIGHT( )
{
    VRP_verticalSphereDefined = FOCAL_horizontalSphereDefined = FOCAL_verticalSphereDefined = false;

    glm::vec3 F = VRP - FOCAL;

    if( !VRP_horizontalSphereDefined )
    {
        grauXZ = round( atan( abs(F.x) / abs(F.z) ) * 180/PI );

        /// PRIMEIRO QUADRANTE é o próprio grau calculado

        /// GRAU XZ
        if( F.x > 0 && F.z < 0 ) /// SEGUNDO QUADRANTE
            grauXZ = 180 - grauXZ;
        else if( F.x < 0 && F.z < 0 ) /// TERCEIRO QUADRANTE
            grauXZ = 180 + grauXZ;
        else if( F.x < 0 && F.z >= 0 ) /// QUARTO QUADRANTE
            grauXZ = 360 - grauXZ;

        VRP_horizontalSphereDefined = true;
    }

    double r = sqrt( pow(F.x,2) + pow(F.z,2) );

    // std::cout << "MOVEU PARA DIRETA - GRAU = " << grauXZ << "\n";

    grauXZ++;

    VRP.x = (  sin( (grauXZ) * PI/180 ) * r + FOCAL.x);
    VRP.z = (  cos( (grauXZ) * PI/180 ) * r + FOCAL.z);

    // std::cout << "VRP = ";
    // std::cout << glm::to_string(VRP) << std::endl;

    redoPipeline();
}

void Pipeline::FOCAL_UP()
{
    FOCAL_horizontalSphereDefined = VRP_horizontalSphereDefined = VRP_verticalSphereDefined= false;

    // std::cout << "FOCAL = ";
    // std::cout << glm::to_string(FOCAL) << std::endl;

    if( !FOCAL_verticalSphereDefined )
    {
        glm::vec3 F = FOCAL - VRP;

        glm::vec3 TerceiroPontoPlano; /// Y = 0
        TerceiroPontoPlano = FOCAL;
        TerceiroPontoPlano.y = VRP.y;

        float sizeX = distance( VRP, TerceiroPontoPlano );
        float sizeY = distance( FOCAL, TerceiroPontoPlano );

        grauXY = round (atan( sizeY / sizeX ) * 180/PI );

        if( F.z < 0 && F.y > 0 )
            grauXY = 180 - grauXY;
        else if( F.z < 0 && F.y < 0 )
            grauXY = 180 + grauXY;
        else if( F.z > 0 && F.y < 0 )
            grauXY = 360 - grauXY;

        // std::cout << "GRAU CALCULADO = " << grauXY << "\n";

        raio = distance(VRP, FOCAL);

        a = raio;
        b = discoverB( a , F ) ;
        // std::cout << "B = " << b << "\n";

        // std::cout << "R = " << raio << "\n";

        FOCAL_verticalSphereDefined = true;
    }

    if(grauXY == 90)
    {
        return;
    }
    else if( grauXY > 270 || ( grauXY > 0 && grauXY < 90 ) )
    {
        grauXY++;
    }
    else
    {
        grauXY--;
    }

    if(grauXY == 90)
        return;

    // std::cout << "GRAU = " << grauXY << std::endl;


    if(grauXY == 360)
        grauXY = 0;

    FOCAL.y = (  sin( (grauXY) * PI/180 ) * raio + VRP.y );


    if( grauXY > 270 || ( grauXY > 0 && grauXY < 90 ) )
        FOCAL.z = ( b * sqrt( pow(a,2) - pow(FOCAL.y,2) + 2*FOCAL.y*VRP.y - pow(VRP.y,2) ) / a + VRP.z);
    else
        FOCAL.z = ( -b * sqrt( pow(a,2) - pow(FOCAL.y,2) + 2*FOCAL.y*VRP.y - pow(VRP.y,2) ) / a + VRP.z);

    FOCAL.x = esferaFOCAL(0 , FOCAL.y, FOCAL.z);

    // std::cout << "FOCAL = ";
    // std::cout << glm::to_string(FOCAL) << std::endl;


    redoPipeline();
}

void Pipeline::FOCAL_DOWN()
{

    FOCAL_horizontalSphereDefined = VRP_horizontalSphereDefined = VRP_verticalSphereDefined = false;

    // std::cout << "FOCAL = ";
    // std::cout << glm::to_string(FOCAL) << std::endl;

    if( !FOCAL_verticalSphereDefined )
    {
        glm::vec3 F = FOCAL - VRP;

        glm::vec3 TerceiroPontoPlano; /// Y = 0
        TerceiroPontoPlano = FOCAL;
        TerceiroPontoPlano.y =  VRP.y ;

        float sizeX = distance( VRP, TerceiroPontoPlano );
        float sizeY = distance( FOCAL, TerceiroPontoPlano );

        grauXY = round (atan( sizeY / sizeX ) * 180/PI );

        if( F.z < 0 && F.y > 0 )
            grauXY = 180 - grauXY;
        else if( F.z < 0 && F.y < 0 )
            grauXY = 180 + grauXY;
        else if( F.z > 0 && F.y < 0 )
            grauXY = 360 - grauXY;

        std::cout << "GRAU CALCULADO = " << grauXY << "\n";

        raio = distance(VRP, FOCAL);

        a = raio;
        b = discoverB( a , F ) ;
        std::cout << "B = " << b << "\n";

        std::cout << "R = " << raio << "\n";

        FOCAL_verticalSphereDefined = true;
    }

    if(grauXY == 270)
    {
        return;
    }
    else if( grauXY > 270 || ( grauXY > 0 && grauXY < 90 ) )
    {
        grauXY--;
    }
    else
    {
        grauXY++;
    }

    if(grauXY == 270)
        return;

    // std::cout << "GRAU = " << grauXY << std::endl;


    if(grauXY == 360)
        grauXY = 0;

    FOCAL.y = (  sin( (grauXY) * PI/180 ) * raio + VRP.y );


    if( grauXY > 270 || ( grauXY > 0 && grauXY < 90 ) )
        FOCAL.z = ( b * sqrt( pow(a,2) - pow(FOCAL.y,2) + 2*FOCAL.y*VRP.y - pow(VRP.y,2) ) / a + VRP.z);
    else
        FOCAL.z = ( -b * sqrt( pow(a,2) - pow(FOCAL.y,2) + 2*FOCAL.y*VRP.y - pow(VRP.y,2) ) / a + VRP.z);

    FOCAL.x = esferaFOCAL(0 , FOCAL.y, FOCAL.z) ;

    // std::cout << "FOCAL = ";
    // std::cout << glm::to_string(FOCAL) << std::endl;

    redoPipeline();
}

void Pipeline::FOCAL_LEFT()
{
    FOCAL_verticalSphereDefined = VRP_verticalSphereDefined = VRP_horizontalSphereDefined = false;

    glm::vec3 F = FOCAL - VRP;

    if( !FOCAL_horizontalSphereDefined )
    {
        grauXZ = round( atan( abs(F.x) / abs(F.z) ) * 180/PI );

        /// PRIMEIRO QUADRANTE é o próprio grau calculado

        /// GRAU XZ
        if( F.x > 0 && F.z < 0 ) /// SEGUNDO QUADRANTE
            grauXZ = 180 - grauXZ;
        else if( F.x < 0 && F.z < 0 ) /// TERCEIRO QUADRANTE
            grauXZ = 180 + grauXZ;
        else if( F.x < 0 && F.z >= 0 ) /// QUARTO QUADRANTE
            grauXZ = 360 - grauXZ;

        FOCAL_horizontalSphereDefined = true;
    }

    double r = sqrt( pow(F.x,2) + pow(F.z,2) );

    // std::cout << "TAMANHO = " << r << "\n";

    // std::cout << "MOVEU PARA ESQUERDA - GRAU = " << grauXZ << "\n";

    grauXZ--;

    FOCAL.x = (  sin( (grauXZ) * PI/180 ) * r + VRP.x);
    FOCAL.z = (  cos( (grauXZ) * PI/180 ) * r + VRP.z);

    // std::cout << "FOCAL = ";
    // std::cout << glm::to_string(FOCAL) << std::endl;

    redoPipeline();

}


void Pipeline::FOCAL_RIGHT()
{
    FOCAL_verticalSphereDefined = VRP_verticalSphereDefined = VRP_horizontalSphereDefined = false;

    glm::vec3 F = FOCAL - VRP;

    if( !FOCAL_horizontalSphereDefined )
    {
        grauXZ = round( atan( abs(F.x) / abs(F.z) ) * 180/PI );

        /// PRIMEIRO QUADRANTE é o próprio grau calculado

        /// GRAU XZ
        if( F.x > 0 && F.z < 0 ) /// SEGUNDO QUADRANTE
            grauXZ = 180 - grauXZ;
        else if( F.x < 0 && F.z < 0 ) /// TERCEIRO QUADRANTE
            grauXZ = 180 + grauXZ;
        else if( F.x < 0 && F.z >= 0 ) /// QUARTO QUADRANTE
            grauXZ = 360 - grauXZ;

        FOCAL_horizontalSphereDefined = true;
    }

    double r = sqrt( pow(F.x,2) + pow(F.z,2) );

    // std::cout << "TAMANHO = " << r << "\n";

    // std::cout << "MOVEU PARA ESQUERDA - GRAU = " << grauXZ << "\n";

    grauXZ++;

    FOCAL.x = (  sin( (grauXZ) * PI/180 ) * r + VRP.x);
    FOCAL.z = (  cos( (grauXZ) * PI/180 ) * r + VRP.z);

    // std::cout << "FOCAL = ";
    // std::cout << glm::to_string(FOCAL) << std::endl;

    redoPipeline();
}

void Pipeline::FOCAL_VRP_UP( )
{
    FOCAL_UP();
    VRP_UP();
}

void Pipeline::FOCAL_VRP_DOWN( )
{
    FOCAL_DOWN();
    VRP_DOWN();
}

void Pipeline::FOCAL_VRP_LEFT( )
{
    FOCAL_LEFT();
    VRP_LEFT();
}

void Pipeline::FOCAL_VRP_RIGHT( )
{
    FOCAL_RIGHT();
    VRP_RIGHT();
}

float Pipeline::esferaFOCAL( float x, float y, float z )
{
    if( x == 0 )
    {
        return FOCAL_bhaskaraX( 1, -VRP.x * 2, pow(VRP.x,2) + pow( (y - VRP.y),2 ) + pow( (z - VRP.z), 2) - pow(raio, 2) );
    }
}

float Pipeline::esferaVRP( float x, float y, float z )
{
    if( x == 0 )
    {
        return VRP_bhaskaraX( 1, -FOCAL.x * 2, pow(FOCAL.x,2) + pow( (y - FOCAL.y),2 ) + pow( (z - FOCAL.z), 2) - pow(raio, 2) );
    }
}

float Pipeline::VRP_bhaskaraX( float a, float b, float c )
{
    float x1, x2;

    x1 = ( -b+sqrt( (b*b) - (4*a*c) ) ) / (2*a);
    x2 = ( -b-sqrt( (b*b) - (4*a*c) ) ) / (2*a);

    float aux1 = abs( x1 - VRP.x );
    float aux2 = abs( x2 - VRP.x );

    // std::cout << "x1 = " << x1 << std::endl;
    // std::cout << "x2 = " << x2 << std::endl;

    if( isnan( x2 ) )
        return x1;
    else if( isnan( x1 ) )
        return x2;

    if( aux1 < aux2 )
        return x1;
    else
        return x2;

}

float Pipeline::FOCAL_bhaskaraX( float a, float b, float c )
{
    float x1, x2;

    x1 = ( -b+sqrt( (b*b) - (4*a*c) ) ) / (2*a);
    x2 = ( -b-sqrt( (b*b) - (4*a*c) ) ) / (2*a);

    float aux1 = abs( x1 - FOCAL.x );
    float aux2 = abs( x2 - FOCAL.x );

    // std::cout << "x1 = " << x1 << std::endl;
    // std::cout << "x2 = " << x2 << std::endl;

    if( isnan( x2 ) )
        return x1;
    else if( isnan( x1 ) )
        return x2;

    if( aux1 < aux2 )
        return x1;
    else
        return x2;

}

vec3f Pipeline::getn(){
    return vec3f(n.x, n.y, n.z);
}

vec3f Pipeline::getvrp(){
    return vec3f(VRP.x, VRP.y, VRP.z);
}

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

std::vector<vec3f> apply_pipeline(float **mat, std::vector<vec3f> &sru_points){
    std::vector<vec3f> ret;

    for(const auto &p : sru_points){
        float x, y, z;

        x = mat[0][0] * p.x + mat[0][1] * p.y + mat[0][2] * p.z + mat[0][3] * 1;
        y = mat[1][0] * p.x + mat[1][1] * p.y + mat[1][2] * p.z + mat[1][3] * 1;
        z = mat[2][0] * p.x + mat[2][1] * p.y + mat[2][2] * p.z + mat[2][3] * 1;

        ret.push_back(vec3f(x, y, z));
    }

    return ret;
}
