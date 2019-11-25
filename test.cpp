#include <utility>      // std::pair, std::make_pair
#include <string> 
#include <iostream>
#include <ctime>
#include <set>
#include <SFML/Graphics.hpp>
#include <typeinfo>
#include <typeindex>
#include <cassert>
#include <tuple>
#include "include/button.hpp"
#include "include/noise.hpp"
#include "include/half_mesh.hpp"

#define WINDOW_Y 768
#define WINDOW_X 1337

#define UP 1
#define DOWN -1

typedef struct cor
{
    double r;
    double g;
    double b;
    cor() : r(0), g(0), b(0) {}
    cor(double r, double b, double g): r(r), b(b), g(g) {}
}Cor;

typedef struct point
{
    double x;
    double y;
    double z;
    double Ilr;
    double Ilg;
    double Ilb;
    struct point *next;
    point() : next (NULL) {}
    point(double x, double y, double z, double Ilr, double Ilb, double Ilg): x(x), y(y), z(z), Ilr(Ilr), Ilb(Ilb), Ilg(Ilg), next(NULL) {}

}Point;

typedef struct ilumination
{
    double IR;
    double IB;
    double IG;
    ilumination() : IR(0), IB(0), IG(0) {}
    ilumination(double r, double b, double g) : IR(r), IB(b), IG(g) {}

}Ilumination;

double IlaR, IlaB, IlaG, KdR, KdB, KdG, KaR, KaB, KaG, KsR, KsB, KsG;
std::vector<vec3f> vertexes;
half_mesh terrain;
vec3f L, VRP;

vec3f calc_normal_vertex(index_t)
{

}

double getColorRed(index_t) {}
double getColorBlue(index_t) {}
double getColorGreen(index_t) {}
void scanline(std::pair <double, Cor> **z_buffer, Point *inicio, Point *fim);


std::vector<Ilumination> calc_ilumination(std::vector<index_t> &pontos)
{
    double IaR = IlaR*KaR;
    double IaB = IlaB*KaB;
    double IaG = IlaG*KaG;
    std::vector<Ilumination> result;

    for(int i = 0; i < pontos.size(); i++)
    {
        std::vector<index_t> faces = terrain.get_vertex_faces(pontos[i]);
        vec3f normal(0.0,0.0,0.0);
        for(int j =0; j< faces.size(); j++)
        {
            normal += terrain.get_face_normal(faces[i]);
        }
        normal.normlize();
        vec3f lVertice = L - vertexes[pontos[i]];
        lVertice.normlize();
        float product = normal.dot(lVertice);

        double IdR = 0;
        double IdB = 0;
        double IdG = 0;

        if(product > 0)
        {
            IdR = getColorRed(pontos[i]) * KdR * product;
            IdB = getColorBlue(pontos[i]) * KdB * product;
            IdG = getColorGreen(pontos[i]) * KdG * product;
        }

        vec3f rVertice = ((normal) * ((lVertice*2.0).dot(normal))) - lVertice;
        rVertice.normlize();

        vec3f sVertice = VRP - vertexes[pontos[i]];
        sVertice.normlize();

        double product2 = rVertice.dot(sVertice);
        double IsR = 0;
        double IsB = 0;
        double IsG = 0;

        if(product2 > 0)
        {
            IsR = getColorRed(pontos[i]) * KsR * product;
            IsB = getColorBlue(pontos[i]) * KsB * product;
            IsG = getColorGreen(pontos[i]) * KsR * product;
        }

        double ItR = IaR + IdR + IsR;
        double ItB = IaB + IdB + IsB;
        double ItG = IaG + IdG + IsG;

        Ilumination vertice(ItR, ItB, ItG);
        result.push_back(vertice);
    }

    return result;
}

    Point **criar_vetor(std::vector<index_t> &pontos, index_t &index, bool &out_of_bounds, int &tam) {
    int maior, menor;
    maior = menor = vertexes[pontos[0]].y;
    for(int i =1; i< pontos.size(); i++)
    {
        if(vertexes[pontos[i]].y > maior)
            maior = vertexes[pontos[i]].y;
        else if(vertexes[pontos[i]].y < menor)
            menor = vertexes[pontos[i]].y;
    }
    if(menor < 0)
        menor = 0;
    if(maior > WINDOW_Y )
        maior = WINDOW_Y;

    int tamanho = maior - menor;

    if(tamanho <= 0)
        return (Point **)NULL;
    else 
    {
        Point **vetor = new Point*[tamanho+1];
        for(int i = 0; i <= tamanho; i++)
            vetor[i] = NULL;
        if(vertexes[pontos[0]].y > WINDOW_Y)
        {
            out_of_bounds = true;
            index = WINDOW_Y;

        }
        else if(vertexes[pontos[0]].y < 0)
        {
            out_of_bounds = true;
            index = 0;
        }
        else
        {
            out_of_bounds = false;
            index = vertexes[pontos[0]].y - menor;
        }
        tam = tamanho;
        return vetor;
    }
}

void guro_face(std::pair <double, Cor> **z_buffer, index_t face) 
{
    std::vector<index_t> index_vertexes = terrain.get_face_vertexes(face);
    index_t position = 0;
    int tamanho = 0;
    bool out_of_bounds = false;
    Point **vetor = criar_vetor(index_vertexes, position, out_of_bounds, tamanho);
    std::vector<Ilumination> iluminacao = calc_ilumination(index_vertexes);

    for(int i =0; i< index_vertexes.size(); i++)
    {
        vec3f ponto1 = vertexes[index_vertexes[i]];
        vec3f ponto2;

        int inc;

        double deltaIR, deltaIB, deltaIG;
        // Se o vertice for o ultimo ele volta para o primeiro
        if(i == 2)
        {
            ponto2 = vertexes[index_vertexes[0]];
            deltaIR = iluminacao[i].IR - iluminacao[0].IR;
            deltaIB = iluminacao[i].IB - iluminacao[0].IB;
            deltaIG = iluminacao[i].IG - iluminacao[0].IG;
        }
        else 
        {
            ponto2 = vertexes[index_vertexes[i+1]];
            deltaIR = iluminacao[i].IR - iluminacao[i+1].IR;
            deltaIB = iluminacao[i].IB - iluminacao[i+1].IB;
            deltaIG = iluminacao[i].IG - iluminacao[i+1].IG;
        }
        double deltaX = ponto1.x - ponto2.x;
        double deltaY = ponto1.y - ponto2.y;
        double deltaZ = ponto1.z - ponto2.z;

        double Tx, Tz, TiR, TiB, TiG;
        Tx = Tz = TiR = TiB = TiG = 0.0;
        if(deltaY != 0) 
        {
            Tx = deltaX/deltaY;
            Tz = deltaZ/deltaY;
            TiR = deltaIR/deltaY;
            TiB = deltaIB/deltaY;
            TiG = deltaIG/deltaY;
        }

        if(0 <= ponto1.y <= WINDOW_Y)
            out_of_bounds = false;
        else
            out_of_bounds = true;
        
        if(out_of_bounds == false)
        {
            Point *novo = new Point(ponto1.x,ponto1.y,ponto1.z,iluminacao[i].IR,iluminacao[i].IB,iluminacao[i].IG);
            if(vetor[position] == NULL)
                vetor[position] = novo;
            else
                vetor[position]->next = novo;
            
        }

        if(deltaY > 0)
            inc = DOWN;
        else
            inc = UP;

        int x_atual, y_atual, z_atual, IR, IB, IG;
        y_atual = round(ponto1.y) + inc;
        x_atual = ponto1.x + Tx;
        z_atual = ponto1.z + Tz;
        IR = iluminacao[i].IR + TiR;
        IB = iluminacao[i].IB + TiB;
        IG = iluminacao[i].IG + TiG;

        if(0 <= y_atual <= WINDOW_Y)
            position += inc;
        
        for(int j = 1; j < abs(round(deltaY)); j++)
        {
            if(out_of_bounds == true)
                if(0 <= y_atual <= WINDOW_Y)
                    out_of_bounds = false;
            
            if(out_of_bounds == false)
            {
                if(y_atual < 0 || y_atual > WINDOW_Y)
                {
                    out_of_bounds = true;
                    continue;
                }

                Point *novo = new Point(x_atual,y_atual,z_atual,IR,IB,IG);
                if(vetor[position] == NULL)
                    vetor[position] = novo;
                else
                    vetor[position]->next = novo;
                
                y_atual += inc;
                x_atual += Tx;
                z_atual += Tz;
                IR += TiR;
                IB += TiB;
                IG += TiG;
                 if(0 <= y_atual <= WINDOW_Y)
                    position += inc;
            }

        }
        
    }
    for(int i =0; i< tamanho; i++)
    {
        if(vetor[i]->next == NULL || vetor[i]->x == vetor[i]->next->x)
            scanline(z_buffer,vetor[i], NULL);
        else
        {
            if(vetor[i]->x < vetor[i]->next->x)
                scanline(z_buffer,vetor[i], vetor[i]->next);
            else
                scanline(z_buffer,vetor[i]->next, vetor[i]);
            
        }
        
    }
}

void scanline(std::pair <double, Cor> **z_buffer, Point *inicio, Point *fim)
{
    if(fim == NULL)
        if(0 <= inicio->x <= WINDOW_X && 0 <= inicio->y <= WINDOW_Y)
        {
            if(z_buffer[(int)inicio->x][(int)inicio->y].first > inicio->z)
            {
                z_buffer[(int)inicio->x][(int)inicio->y].first = inicio->z;

                Cor cor(inicio->Ilr,inicio->Ilb,inicio->Ilg);

                z_buffer[(int)inicio->x][(int)inicio->y].second = cor;
            }
        }
    else
    {
        double deltaX = fim->x - inicio->x;
        double deltaZ = fim->z - inicio->z;
        double deltaIR = fim->Ilr - inicio->Ilr;
        double deltaIB = fim->Ilb - inicio->Ilb;
        double deltaIG = fim->Ilg - inicio->Ilg;

        double Tz, TiR, TiB, TiG;
        Tz = TiR = TiB = TiG = 0;

        if(deltaX != 0)
        {
            Tz = deltaZ/deltaX;
            TiR = deltaIR/deltaX;
            TiB = deltaIB/deltaX;
            TiG = deltaIG/deltaX;
        }
        if(inicio->x < 0)
        {
            if(fim->x < 0)
                return;
            else
            {
                double deslocamento = - inicio->x;
                inicio->x = 0;
                inicio->z = inicio->z + Tz*deslocamento;
                inicio->Ilr = inicio->Ilr + TiR*deslocamento;
                inicio->Ilb = inicio->Ilb + TiB*deslocamento;
                inicio->Ilg = inicio->Ilg + TiG*deslocamento;
            }
            
        }
        if(fim->x > WINDOW_X)
        {
            double deslocamento = fim->x - WINDOW_X;
            fim->x = WINDOW_X;
            fim->z = fim->z - Tz*deslocamento;
            fim->Ilr = fim->Ilr - TiR*deslocamento;
            fim->Ilb = fim->Ilb - TiB*deslocamento;
            fim->Ilg = fim->Ilg - TiG*deslocamento;
        }
        double condition = fim->x - inicio->x;

        double x_atual,y , z_atual, IR, IB, IG;
        x_atual = inicio->x;
        y = inicio->y;
        z_atual = inicio->z;
        IR = inicio->Ilr;
        IB = inicio->Ilb;
        IG = inicio->Ilg;

        for(int i =0; i<= condition; i++)
        {
            if(z_buffer[(int)x_atual][(int)y].first > z_atual)
            {
                z_buffer[(int)x_atual][(int)y].first = z_atual;

                Cor cor(IR,IB,IG);

                z_buffer[(int)x_atual][(int)y].second = cor;
            }
        }
        delete(inicio);
        delete(fim);
    }
    

}


int main() {

}