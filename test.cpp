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

typedef struct txColor
{
    double r;
    double g;
    double b;
    txColor() : r(0), g(0), b(0) {}
    txColor(double r, double b, double g): r(r), b(b), g(g) {}
    void calc(sf::Color &fim, sf::Color &inicio, double deltaY)
    {
        r = (fim.r - inicio.r)/std::abs(deltaY);
        b = (fim.b - inicio.b)/std::abs(deltaY);
        g = (fim.g - inicio.g)/std::abs(deltaY);
    }
    sf::Color operator + (const sf::Color &color)
    {
        return sf::Color(r + color.r,b + color.b,g + color.g);
    }
    txColor operator * (const double &v)
    {
        return txColor(r*v,b*v,g*v);
    }

}TxColor;

typedef struct Point
{
    index_t vertex;
    double x;
    double y;
    double z;
    sf::Color color;
    struct Point *next;
    Point() : next (NULL) {}
    Point(index_t vertex ,double x, double y, double z, sf::Color color): vertex(vertex) ,x(x), y(y), z(z), color(color), next(NULL) {}

};

double Ila, Kd, ka, Ks, lpoint;
std::vector<vec3f> vertexes;
half_mesh terrain;
vec3f L, VRP;

void apply_light(sf::Color &cor, double Il)
{
    cor.r = cor.r * Il;
    cor.b = cor.b * Il;
    cor.g = cor.g * Il;
}

sf::Color getColor(index_t) {}

void scanline(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim);

std::vector<double> calc_ilumination(std::vector<index_t> &pontos)
{
    double Iambiente = Ila*ka;
    std::vector<double> result;

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

        double Id = 0;
        double Is = 0;

        if(product > 0) 
        {
            Id = lpoint * Kd * product;

            vec3f rVertice = ((normal) * ((lVertice*2.0).dot(normal))) - lVertice;
            rVertice.normlize();

            vec3f sVertice = VRP - vertexes[pontos[i]];
            sVertice.normlize();

            double product2 = rVertice.dot(sVertice);

            if(product2 > 0)
                Is = lpoint * Ks * product;
        }

        double It = Iambiente + Id + Is;

        result.push_back(It);
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

void guro_face(std::pair <double, sf::Color> **z_buffer, index_t face) 
{
    std::vector<index_t> index_vertexes = terrain.get_face_vertexes(face);
    index_t position = 0;
    int tamanho = 0;
    bool out_of_bounds = false;
    Point **vetor = criar_vetor(index_vertexes, position, out_of_bounds, tamanho);
    std::vector<double> iluminacao = calc_ilumination(index_vertexes);

    for(int i =0; i< index_vertexes.size(); i++)
    {
        vec3f ponto1 = vertexes[index_vertexes[i]];
        vec3f ponto2;

        int inc;

        sf::Color cor1, cor2;
        // Se o vertice for o ultimo ele volta para o primeiro
        if(i == 2)
        {
            ponto2 = vertexes[index_vertexes[0]];
            cor1 = getColor(index_vertexes[i]);
            apply_light(cor1, iluminacao[i]);
            cor2 = getColor(index_vertexes[0]);
            apply_light(cor2, iluminacao[0]);
        }
        else 
        {
            ponto2 = vertexes[index_vertexes[i+1]];
            cor1 = getColor(index_vertexes[i]);
            apply_light(cor1, iluminacao[i]);
            cor2 = getColor(index_vertexes[i+1]);
            apply_light(cor2, iluminacao[i+1]);
        }

        double deltaX = ponto2.x - ponto1.x;
        double deltaY = ponto2.y - ponto1.y;
        double deltaZ = ponto2.z - ponto1.z;

        double Tx, Tz;
        Tx = Tz = 0.0;

        TxColor txColor = {0.0,0.0,0.0};

        if(deltaY != 0) 
        {
            Tx = deltaX/std::abs(deltaY);
            Tz = deltaZ/std::abs(deltaY);
            txColor.calc(cor2, cor1, deltaY);
        }

        if(0 <= ponto1.y <= WINDOW_Y)
            out_of_bounds = false;
        else
            out_of_bounds = true;
        
        if(out_of_bounds == false)
        {
            Point *novo = new Point(index_vertexes[i],ponto1.x,ponto1.y,ponto1.z,cor1);
            if(vetor[position] == NULL)
                vetor[position] = novo;
            else
                vetor[position]->next = novo;
            
        }

        if(deltaY > 0)
            inc = UP;
        else
            inc = DOWN;

        int x_atual, y_atual, z_atual;
        sf::Color cor_atual;
        y_atual = round(ponto1.y) + inc;
        x_atual = ponto1.x + Tx;
        z_atual = ponto1.z + Tz;
        cor_atual = txColor + cor1;

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

                Point *novo = new Point(index_vertexes[i], x_atual,y_atual,z_atual,cor_atual);
                if(vetor[position] == NULL)
                    vetor[position] = novo;
                else
                    vetor[position]->next = novo;
                
                y_atual += inc;
                x_atual += Tx;
                z_atual += Tz;
                cor_atual = txColor + cor_atual;
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

void scanline(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim)
{
    if(fim == NULL)
        if(0 <= inicio->x <= WINDOW_X && 0 <= inicio->y <= WINDOW_Y)
        {
            if(z_buffer[(int)inicio->x][(int)inicio->y].first > inicio->z)
            {
                z_buffer[(int)inicio->x][(int)inicio->y].first = inicio->z;

                z_buffer[(int)inicio->x][(int)inicio->y].second = inicio->color;
            }
        }
    else
    {
        double deltaX = fim->x - inicio->x;
        double deltaZ = fim->z - inicio->z;

        double Tz;
        Tz = 0;

        TxColor txColor = {0.0,0.0,0.0};

        if(deltaX != 0)
        {
            Tz = deltaZ/abs(deltaX);
            txColor.calc(fim->color, inicio->color, deltaX);
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
                inicio->color =  (txColor * deslocamento) + inicio->color ;
            }
            
        }
        if(fim->x > WINDOW_X)
        {
            double deslocamento = fim->x - WINDOW_X;
            fim->x = WINDOW_X;
            fim->z = fim->z - Tz*deslocamento;
            inicio->color =  (txColor * (-deslocamento)) + inicio->color ;
        }
        double condition = fim->x - inicio->x;

        double x_atual,y , z_atual;
        sf::Color color;
        x_atual = inicio->x;
        y = inicio->y;
        z_atual = inicio->z;
        color = inicio->color;

        for(int i =0; i<= floor(condition); i++)
        {
            if(z_buffer[(int)x_atual][(int)y].first > z_atual)
            {
                z_buffer[(int)x_atual][(int)y].first = z_atual;

                z_buffer[(int)x_atual][(int)y].second = color;
            }
            x_atual++;
            z_atual += Tz;
            color = txColor + color;
        }
        delete(inicio);
        delete(fim);
    }
    

}


int main() {

}