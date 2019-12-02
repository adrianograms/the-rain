#ifndef SHADING_H
#define SHADING_H

#include <utility>      // std::pair, std::make_pair
#include <string>
#include <iostream>
#include <set>
#include <SFML/Graphics.hpp>
#include <cassert>
#include <tuple>

#include "utils.hpp"
#include "button.hpp"
#include "noise.hpp"
#include "half_mesh.hpp"

#define WINDOW_Y 768
#define WINDOW_X 1337

#define UP 1
#define DOWN -1

struct txColor
{
    double r;
    double g;
    double b;
    txColor() : r(0), g(0), b(0) {}
    txColor(double r, double g, double b): r(r), g(g), b(b) {}
    void calc(sf::Color &fim, sf::Color &inicio, double deltaY)
    {
        r = (fim.r - inicio.r)/std::abs(deltaY);
        b = (fim.b - inicio.b)/std::abs(deltaY);
        g = (fim.g - inicio.g)/std::abs(deltaY);
    }
    sf::Color operator + (const sf::Color &color)
    {
        int r_new = r + color.r;
        if(r_new > 255)
            r_new = 255;
        else if(r_new < 0)
            r_new = 0;

        int g_new = g + color.g;
        if(g_new > 255)
            g_new = 255;
        else if(g_new < 0)
            g_new = 0;

        int b_new = b + color.b;
        if(b_new > 255)
            b_new = 255;
        else if(b_new < 0)
            b_new = 0;

        return sf::Color(r_new,g_new,b_new);
    }
    txColor operator * (const double &v)
    {
        return txColor(r*v,g*v,b*v);
    }
};

struct Point
{
    index_t vertex;
    double x;
    double y;
    double z;
    sf::Color color;
    struct Point *next;
    Point() : next (NULL) {}
    Point(index_t vertex ,double x, double y, double z, sf::Color color): vertex(vertex),
                                                                          x(x),
                                                                          y(y),
                                                                          z(z),
                                                                          color(color),
                                                                          next(NULL) { }
};

struct light{
    std::string x;
    std::string y;
    std::string z;

    vec3f pos;

    double N;
    double LPOINT;

    double LA;

    double KA;
    double KD;
    double KS;

    light(): x("0"), y("0"), z("0"), N(1.63), LPOINT(155), LA(80), KA(0.277), KD(0.467), KS(0.369){ }

    void parse(){
        float x = std::stof(this->x);
        x = x == std::nan("1") ? 0.0 : x;
        this->x = std::to_string(x);

        float y = std::stof(this->y);
        y = y == std::nan("1") ? 0.0 : y;
        this->y = std::to_string(y);

        float z = std::stof(this->z);
        z = z == std::nan("1") ? 0.0 : z;
        this->z = std::to_string(z);

        pos = vec3f(x, y, z);
    }
};

// --------------------------------------------------------------------------------------------------------------------
// Variaveis Globais
// --------------------------------------------------------------------------------------------------------------------


//std::vector<vec3f> vertexes;

// Aplica o brilho nas cores
void apply_light(sf::Color &cor, double Il)
{
    cor.r = cor.r * (Il/255);
    cor.b = cor.b * (Il/255);
    cor.g = cor.g * (Il/255);
}

std::pair<int, int> get_maior_menor(const std::vector<index_t> &indexes,
                                    const std::vector<vec3f> &src_points)
{
    std::pair<int, int> maior_menor;
    double maior, menor;
    maior = src_points[indexes[0]].y;
    menor = src_points[indexes[0]].y;

    for(auto index:indexes)
    {
        if(maior < src_points[index].y)
            maior = src_points[index].y;
        else if(menor > src_points[index].y)
            menor = src_points[index].y;
    }
    maior_menor.first = (int)round(maior);
    maior_menor.second = (int)round(menor);
    if(maior_menor.first >= WINDOW_Y)
        maior_menor.first = WINDOW_Y - 1;
    if(maior_menor.second < 0)
        maior_menor.second = 0;
    return maior_menor;
}

//Pega a cor do vertex
sf::Color getColor(index_t vertex, const config &conf){
    auto v = conf.texture.getSize();
    return conf.texture.getPixel((vertex % conf.mx) % v.x,
                                 (vertex / conf.mx) % v.y);
    //return sf::Color::White;
}

void scanline(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim);

Point **criar_vetor(std::vector<index_t> &pontos,
                    index_t &index,
                    bool &out_of_bounds,
                    int &tam,
                    std::vector<vec3f> const &src_points) {
    int maior, menor;
    maior = menor = src_points[pontos[0]].y;
    // Descobre o maior entre os vertices
    for(int64_t i = 1; i < (int64_t)pontos.size(); i++)
    {
        if(round(src_points[pontos[i]].y) > maior)
            maior = round(src_points[pontos[i]].y);
        else if(round(src_points[pontos[i]].y) < menor)
            menor = round(src_points[pontos[i]].y);
    }
    //Verifica se o menor ou maior estão dentro da janela
    if(menor < 0)
        menor = 0;
    if(maior >= WINDOW_Y )
        maior = WINDOW_Y;

    int tamanho = maior - menor;

    // Se tamanho negativo retorna NULL
    if(tamanho <= 0)
        return (Point **)NULL;
    else
    {
        Point **vetor = new Point*[tamanho];
        //Inicia todo mundo como NULL
        for(int i = 0; i < tamanho; i++)
            vetor[i] = NULL;
        //Verifica se o vertice incial está dentro da janela se não ele seta o index/position
        //para a fronteira baixa/alta da janela
        if(round(src_points[pontos[0]].y) >= WINDOW_Y)
        {
            out_of_bounds = true;
            index = tamanho-1;
        }
        else if(round(src_points[pontos[0]].y) < 0)
        {
            out_of_bounds = true;
            index = 0;
        }
        else
        {
            out_of_bounds = false;
            if(round(src_points[0].y) >= maior)
                index = tamanho - 1;
            else
                index = round(src_points[pontos[0]].y) - menor;
        }
        tam = tamanho;
        return vetor;
    }
}

// --------------------------------------------------------------------------------------------------------------------------
// Guro
// --------------------------------------------------------------------------------------------------------------------------


std::vector<double> calc_ilumination(std::vector<index_t> &pontos,
                                     const config &conf,
                                     const light &lp,
                                     const vec3f VRP)
{
    double Iambiente = lp.LA * lp.KA;
    std::vector<double> result;

    for(int64_t i = 0; i < (int64_t)pontos.size(); i++)
    {
        std::vector<index_t> faces = conf.terrain.get_vertex_faces(pontos[i]);
        vec3f normal(0.0,0.0,0.0);
        for(int64_t j = 0; j < (int64_t)faces.size(); j++)
        {
            //std::cout <<  faces[j] << std::endl;
            normal += conf.terrain.get_face_normal(faces[j]);
        }
        normal.normlize();
        vec3f lVertice = lp.pos - conf.terrain.points[pontos[i]]; // Pontos no SRU
        lVertice.normlize();

        double product = normal.dot(lVertice);

        double Id = 0;
        double Is = 0;

        if(product > 0)
        {
            Id = lp.LPOINT * lp.KD * product;

            vec3f rVertice = ((normal) * ((lVertice*2.0).dot(normal))) - lVertice;
            rVertice.normlize();

            vec3f sVertice = VRP - conf.terrain.points[pontos[i]];
            sVertice.normlize();

            double product2 = rVertice.dot(sVertice);

            if(product2 > 0)
                Is = lp.LPOINT * lp.KS * pow(product2, lp.N);
        }

        double It = Iambiente + Id + Is;

        result.push_back(It);
    }

    return result;
}

void scanline(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim)
{
    int x_inicial, x_final, y;
    x_inicial = (int)round(inicio->x);
    y = (int)round(inicio->y);
    if(fim == NULL)
    {
        //Porem o vertice pode estar fora das fronteiras
        if(0 <= x_inicial && x_inicial < WINDOW_X && 0 <= y && y < WINDOW_Y)
        {
            if(z_buffer[y][x_inicial].first < inicio->z)
            {
                z_buffer[y][x_inicial].first = inicio->z;

                z_buffer[y][x_inicial].second = inicio->color;
            }
        }
        delete inicio;
    }
    else
    {
        x_final = (int)round(fim->x);

        //Calcula delta
        double deltaX = fim->x - inicio->x;
        double deltaZ = fim->z - inicio->z;

        double Tz;
        Tz = 0;

        txColor txColor = {0.0,0.0,0.0};

        if(deltaX != 0)
        {
            //Calcula taxa
            Tz = deltaZ/std::abs(deltaX);
            txColor.calc(fim->color, inicio->color, deltaX);
        }
        //Corta a scanline caso ela passe da janela
        if(x_inicial >= WINDOW_X)
        {
            delete inicio;
            delete fim;
            return;
        }
        if(x_inicial < 0)
        {
            //Se os dois estão fora tanto o inicio quanto o fim da scanline não faz nada
            if(x_final < 0)
            {
                delete inicio;
                delete fim;
                return;
            }
            else
            {
                double deslocamento = (- inicio->x);
                x_inicial = 0;
                inicio->z = inicio->z + Tz*deslocamento;
                inicio->color =  (txColor * deslocamento) + inicio->color ;
            }

        }
        // Se o fim do vetor estiver fora ele "recorta"
        if(x_final >= WINDOW_X)
        {
            x_final = WINDOW_X - 1;
            //fim->z = fim->z - Tz*deslocamento;
            //fim->color =  (txColor * (-deslocamento)) + fim->color ;
        }
        // Anda por todo os x's da scanline
        //Seta os valores iniciais da scanline que são os valores do primeiro vertice da scanline
        double z_atual;
        sf::Color color;
        z_atual = inicio->z;
        color = inicio->color;

        //std::cout << "Inicio: " << x_inicial << std::endl;
        //std::cout << "Fim: " << x_final << std::endl;

        for(int i = x_inicial; i<= x_final; i++)
        {
            // Se o Z da scanline for maior ele atualiza a cor dele
            if(z_buffer[y][i].first < z_atual)
            {
                z_buffer[y][i].first = z_atual;
                int diff = (i - x_inicial);
                z_buffer[y][i].second = (txColor * diff) + color;
            }
            // Incrementa os valores para continuar andando pela scanline
            z_atual += Tz;
            //color = txColor + color;
        }
        //deleta os point's para limpar memoria
        delete inicio;
        delete fim;
    }

}

void guro_face(std::pair <double, sf::Color> **z_buffer,
               index_t face,
               const config &conf,
               const light  &lp,
               const vec3f  &vrp,
               const std::vector<vec3f> &src_points)
{
    std::vector<index_t> index_vertexes = conf.terrain.get_face_vertexes(face);
    std::pair<int,int> maior_menor = get_maior_menor(index_vertexes,src_points);
    bool out_of_bounds = false;

    Point **vetor = new Point*[WINDOW_Y];
    for(int i =0; i< WINDOW_Y; i++)
        vetor[i] = NULL;

    std::vector<double> iluminacao = calc_ilumination(index_vertexes, conf, lp, vrp);

    for(int64_t i =0; i < (int64_t)index_vertexes.size(); i++)
    {
        vec3f ponto1 = src_points[index_vertexes[i]];
        vec3f ponto2;

        int inc;

        sf::Color cor1, cor2;
        // Se o vertice for o ultimo ele volta para o primeiro
        if(i == (int64_t)index_vertexes.size() - 1)
        {
            ponto2 = src_points[index_vertexes[0]];
            cor1 = getColor(index_vertexes[i], conf);
            apply_light(cor1, iluminacao[i]);
            cor2 = getColor(index_vertexes[0], conf);
            apply_light(cor2, iluminacao[0]);
        }
        else
        {
            ponto2 = src_points[index_vertexes[i+1]];
            cor1 = getColor(index_vertexes[i], conf);
            apply_light(cor1, iluminacao[i]);
            cor2 = getColor(index_vertexes[i+1], conf);
            apply_light(cor2, iluminacao[i+1]);
        }
        // Calculo dos delta
        double deltaX = ponto2.x - ponto1.x;
        double deltaY = ponto2.y - ponto1.y;
        double deltaZ = ponto2.z - ponto1.z;

        double Tx, Tz;
        Tx = Tz = 0.0;

        txColor txColor = {0.0,0.0,0.0};

        if(deltaY != 0)
        {
            // Calcula as taixas, e para a cor calcula o delta e a taixa
            Tx = deltaX/std::abs(deltaY);
            Tz = deltaZ/std::abs(deltaY);
            txColor.calc(cor2, cor1, deltaY);
        }


        int y = (int)round(ponto1.y);
        //Verifica se o vertice ta dentro da tela e seta out_of_bouns dependendo da conclusão
        if(0 <= y && y < WINDOW_Y)
            out_of_bounds = false;
        else
            out_of_bounds = true;

        /*
        /Se o ponto está dentro das fronteiras, o vertice é adicionado na lista
        /de forma a garantir que o ponto esteja na lista, e por questões de consistencia
        / do index position
        */
        if(out_of_bounds == false)
        {
            Point *novo = new Point(index_vertexes[i],ponto1.x,y,ponto1.z,cor1);
            if(vetor[y] == NULL)
                vetor[y] = novo;
            else
                vetor[y]->next = novo;
        }

        // Verifica se ta subindo ou descendo
        if(deltaY > 0)
            inc = UP;
        else
            inc = DOWN;

        //Seta os valores para o ponto logo abaixo/acima do vertice para sua primeira iteração
        int y_atual, y_final;
        double x_atual, z_atual;
        sf::Color cor_atual;
        y_atual = y + inc;
        y_final = (int)round(ponto2.y);
        x_atual = ponto1.x + Tx;
        z_atual = ponto1.z + Tz;

        //int deltaY_int = (int)std::abs(round(deltaY));

        //Loop para a variação em y, ou seja, para cada scanline
        for(int j = 1; y_atual*inc < y_final*inc; j++)
        {
            /*Se o y estiver dentro das fronteiras ele atualiza para poder processa-lo
            /alem disso ele faz isso de forma que ele atualize e possa processar o proximo valor
            /que no caso sera a fronteira da janela
            */
            if(out_of_bounds == true)
                if(0 <= y_atual && y_atual < WINDOW_Y)
                    out_of_bounds = false;

            if(out_of_bounds == false)
            {
                //Verifica se esta na janela
                if(y_atual < 0 || y_atual >= WINDOW_Y)
                {
                    out_of_bounds = true;
                }
                else
                {
                    //std::cout << "Y: " << y_atual << std::endl;
                    //Adiciona um ponto no vetor
                    //int diff =  y_atual*inc - y*inc;
                    cor_atual = (txColor * j) + cor1;
                    Point *novo = new Point(index_vertexes[i], x_atual,y_atual,z_atual,cor_atual);

                    if(vetor[y_atual] == NULL)
                        vetor[y_atual] = novo;
                    else
                        vetor[y_atual]->next = novo;
                }
            }
            //Atualiza para toda iteração
            y_atual += inc;
            x_atual += Tx;
            z_atual += Tz;
            //cor_atual = txColor + cor_atual;
        }

    }
    /*
    /Processa para todas as scanline de baixo para cima
    */
    //std::cout << "Comecei" << std::endl;
    for(int i = maior_menor.second; i<= maior_menor.first; i++)
    {
        if(vetor[i] == NULL)
            continue;
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
    //std::cout << "Terminei" << std::endl;

    delete[] vetor;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------------------
// flat shading
// ---------------------------------------------------------------------------------------------------------------------------------------------------------------

sf::Color avg_color(sf::Color cor1, sf::Color cor2, sf::Color cor3)
{
    int r = (cor1.r + cor2.r + cor3.r)/3;
    int b = (cor1.b + cor2.b + cor3.b)/3;
    int g = (cor1.g + cor2.g + cor3.g)/3;

    sf::Color avg(r,g,b);
    return avg;
}

double calc_ilumination_flat(std::vector<index_t> &indexes,
                             index_t face,
                             const config &conf,
                             const light &lp,
                             const vec3f &vrp)
{
    double x_bigger, y_bigger, z_bigger, x_smaller, y_smaller, z_smaller;
    x_bigger = x_smaller = conf.terrain.points[indexes[0]].x;
    y_bigger = y_smaller = conf.terrain.points[indexes[0]].y;
    z_bigger = z_smaller = conf.terrain.points[indexes[0]].z;
    for(int64_t i = 1; i < (int64_t)indexes.size(); i++)
    {
        //Maior e menor x
        if(x_bigger < conf.terrain.points[indexes[i]].x)
        {
            x_bigger = conf.terrain.points[indexes[i]].x;
        }
        if(x_smaller > conf.terrain.points[indexes[i]].x)
        {
            x_smaller = conf.terrain.points[indexes[i]].x;
        }

        //Maior e menor y
        if(y_bigger < conf.terrain.points[indexes[i]].y)
        {
            y_bigger = conf.terrain.points[indexes[i]].y;
        }
        if(y_smaller > conf.terrain.points[indexes[i]].y)
        {
            y_smaller = conf.terrain.points[indexes[i]].y;
        }

        //Maior e menor z
        if(z_bigger < conf.terrain.points[indexes[i]].z)
        {
            z_bigger = conf.terrain.points[indexes[i]].z;
        }
        if(z_smaller > conf.terrain.points[indexes[i]].z)
        {
            z_smaller = conf.terrain.points[indexes[i]].z;
        }
    }
    double x_avg = (x_bigger + x_smaller)/2;
    double y_avg = (y_bigger + y_smaller)/2;
    double z_avg = (z_bigger + z_smaller)/2;
    vec3f avg(x_avg,y_avg,z_avg);

    double IlAmbiente = lp.LA * lp.KA;

    vec3f face_normal = conf.terrain.get_face_normal(face);
    face_normal.normlize();

    vec3f l(0, 0, 0);
    l = lp.pos - avg;
    l.normlize();

    double product = face_normal.dot(l);

    double Id, Is;
    Id = Is = 0.0;
    if(product > 0)
    {
        Id = lp.LPOINT * lp.KD * product;

        vec3f rVertice = ((face_normal) * ((l*2.0).dot(face_normal))) - l;
        rVertice.normlize();

        vec3f sVertice = vrp - avg;
        sVertice.normlize();

        double product2 =  rVertice.dot(sVertice);

        if(product2 > 0)
        {
            Is = lp.LPOINT * lp.KS * pow(product2, lp.N);
        }

        double Iltotal = IlAmbiente + Id + Is;
        if(Iltotal >= 0)
            return Iltotal;
        else
            return 0;

    }
    return IlAmbiente;
    // FIXME: nao retorna nada se product for <= 0
}

void scanline_flat(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim)
{
    // Se fim não existe então é necessario apenas pintar um pixel
    //std::cout << "Cheguei" << std::endl;
    int x_inicial, x_final, y;
    x_inicial = (int)round(inicio->x);
    y = (int)round(inicio->y);
    if(fim == NULL)
    {
        //Porem o vertice pode estar fora das fronteiras
        if(0 <= x_inicial && x_inicial < WINDOW_X && 0 <= y && y < WINDOW_Y)
        {
            if(z_buffer[y][x_inicial].first < inicio->z)
            {
                z_buffer[y][x_inicial].first = inicio->z;

                z_buffer[y][x_inicial].second = inicio->color;
            }
        }
        delete inicio;
    }
    else
    {
        x_final = (int)round(fim->x);
        //Calcula delta
        double deltaX = fim->x - inicio->x;
        double deltaZ = fim->z - inicio->z;

        double Tz;
        Tz = 0;

        if(deltaX != 0)
        {
            //Calcula taxa
            Tz = deltaZ/std::abs(deltaX);
        }

        //Corta a scanline caso ela passe da janela
        if(x_inicial >= WINDOW_X)
        {
            delete inicio;
            delete fim;
            return;
        }
        if(x_inicial < 0)
        {
            //Se os dois estão fora tanto o inicio quanto o fim da scanline não faz nada
            if(x_final < 0)
            {
                delete inicio;
                delete fim;
                return;
            }
            else
            {
                double deslocamento = (- inicio->x);
                x_inicial = 0;
                inicio->z = inicio->z + Tz*deslocamento;
            }

        }
        // Se o fim do vetor estiver fora ele "recorta"
        if(x_final >= WINDOW_X)
        {
            x_final = WINDOW_X - 1;
            //fim->z = fim->z - Tz*deslocamento;
        }
        //Seta os valores iniciais da scanline que são os valores do primeiro vertice da scanline
        double z_atual;
        sf::Color color;
        z_atual = inicio->z;
        color = inicio->color;

        for(int i = x_inicial; i<= x_final; i++)
        {
            // Se o Z da scanline for maior ele atualiza a cor dele
            //std::cout << "y: " << y << std::endl;
            //std::cout << "x: " << i << std::endl;
            if(z_buffer[y][i].first < z_atual)
            {
                z_buffer[y][i].first = z_atual;

                z_buffer[y][i].second = color;
            }
            // Incrementa os valores para continuar andando pela scanline
            z_atual += Tz;
        }
        //std::cout << "Fim" << std::endl;
        //deleta os point's para limpar memoria
        delete inicio;
        delete fim;
    }
}

void flat_shading_face(std::pair <double, sf::Color> **z_buffer,
                       index_t face,
                       const config &conf,
                       const light  &lp,
                       const vec3f  &vrp,
                       const std::vector<vec3f> &src_points)
{
    std::vector<index_t> index_vertexes = conf.terrain.get_face_vertexes(face);
    std::pair <int, int> maior_menor = get_maior_menor(index_vertexes, src_points);
    bool out_of_bounds = false;
    //Point **vetor = criar_vetor(index_vertexes, position, out_of_bounds, tamanho, src_points);
    Point **vetor = new Point*[WINDOW_Y];
    for(int i =0; i< WINDOW_Y; i++)
        vetor[i] = NULL;

    double light = calc_ilumination_flat(index_vertexes, face, conf, lp, vrp);
    //std::cout << "Luz: " << light << std::endl;

    sf::Color cor1, cor2, cor3, cor;
    cor1 = getColor(index_vertexes[0], conf);
    cor2 = getColor(index_vertexes[1], conf);
    cor3 = getColor(index_vertexes[2], conf);

    cor = avg_color(cor1, cor2, cor3);
    apply_light(cor, light);

    for(int64_t i = 0; i < (int64_t)index_vertexes.size(); i++)
    {
        vec3f ponto1 = src_points[index_vertexes[i]];
        vec3f ponto2;

        int inc;
        // Se o vertice for o ultimo ele volta para o primeiro
        if(i == (int64_t)index_vertexes.size() - 1)
        {
            ponto2 = src_points[index_vertexes[0]];
        }
        else
        {
            ponto2 = src_points[index_vertexes[i+1]];
        }
        //std::cout << "Ponto 2: " << (int)ponto2.x << std::endl;
        // Calculo dos delta
        double deltaX = ponto2.x - ponto1.x;
        double deltaY = ponto2.y - ponto1.y;
        double deltaZ = ponto2.z - ponto1.z;

        double Tx, Tz;
        Tx = Tz = 0.0;


        //std::cout << "deltaY: " << deltaY << std::endl;
        if(deltaY != 0)
        {
            // Calcula as taixas, e para a cor calcula o delta e a taixa
            Tx = deltaX/std::abs(deltaY);
            Tz = deltaZ/std::abs(deltaY);
        }

        //Verifica se o vertice ta dentro da tela e seta out_of_bouns dependendo da conclusão
        int y = (int)round(ponto1.y);
        if(0 <= y && y < WINDOW_Y)
            out_of_bounds = false;
        else
            out_of_bounds = true;

        /*
        /Se o ponto está dentro das fronteiras, o vertice é adicionado na lista
        /de forma a garantir que o ponto esteja na lista, e por questões de consistencia
        / do index position
        */
        if(out_of_bounds == false)
        {
            Point *novo = new Point(index_vertexes[i],ponto1.x,y,ponto1.z,cor);
            if(vetor[y] == NULL)
                vetor[y] = novo;
            else if(vetor[y]->next == NULL)
                vetor[y]->next = novo;
            else
            {
                delete vetor[y]->next;
                vetor[y]->next = novo;
            }
        }


        // Verifica se ta subindo ou descendo
        if(deltaY > 0)
            inc = UP;
        else
            inc = DOWN;

        //Seta os valores para o ponto logo abaixo/acima do vertice para sua primeira iteração
        int y_atual, y_final;
        double x_atual, z_atual;
        y_atual = y + inc;
        y_final = (int)round(ponto2.y);
        x_atual = ponto1.x + Tx;
        z_atual = ponto1.z + Tz;

        //std::cout << "Posicao: " << position<< std::endl;

        //Loop para a variação em y, ou seja, para cada scanline
        for(; y_atual*inc < y_final*inc;)
        {
            /*Se o y estiver dentro das fronteiras ele atualiza para poder processa-lo
            /alem disso ele faz isso de forma que ele atualize e possa processar o proximo valor
            /que no caso sera a fronteira da janela
            */
            if(out_of_bounds == true)
                if(0 <= y_atual && y_atual < WINDOW_Y)
                    out_of_bounds = false;

            if(out_of_bounds == false)
            {
                //Verifica se esta na janela
                if(y_atual < 0 || y_atual >= WINDOW_Y)
                {
                    out_of_bounds = true;
                }
                else
                {
                    //std::cout << "Posicao Interna: " << y_atual<< std::endl;
                    //Adiciona um ponto no vetor
                    Point *novo = new Point(index_vertexes[i], x_atual,y_atual,z_atual,cor);

                    if(vetor[y_atual] == NULL)
                        vetor[y_atual] = novo;
                    else if(vetor[y_atual]->next == NULL)
                        vetor[y_atual]->next = novo;
                    else
                    {
                        delete vetor[y_atual]->next;
                        vetor[y_atual]->next = novo;
                    }

                    //Só atualiza quando ele consegue se mover dentro pelo vetor
                    //std::cout << "Ponto X: " << vetor[y_atual]->x << std::endl;
                }
            }
            //Atualiza para toda iteração
            y_atual += inc;
            x_atual += Tx;
            z_atual += Tz;
        }

    }
    //std::cout << maior_menor.first << ", " << maior_menor.second << std::endl;
    //std::cout << "Comecei" << std::endl;
    for(int i = maior_menor.second; i<= maior_menor.first; i++)
    {
        if(vetor[i] == NULL)
            continue;
        if(vetor[i]->next == NULL)
            scanline_flat(z_buffer,vetor[i], NULL);
        else
        {
            if(vetor[i]->x < vetor[i]->next->x)
                scanline_flat(z_buffer,vetor[i], vetor[i]->next);
            else
                scanline_flat(z_buffer,vetor[i]->next, vetor[i]);
        }
    }
    //std::cout << "Finalizei" << std::endl;
    delete[] vetor;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Wireframe
// -----------------------------------------------------------------------------------------------------------------------------------------------------

void scanline_wire(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim)
{
    // Se fim não existe então é necessario apenas pintar um pixel
    if(inicio == NULL)
        return;
    int x_inicial, x_final, y;
    x_inicial = (int)round(inicio->x);
    y = (int)round(inicio->y);
    if(fim == NULL)
    {
        //Porem o vertice pode estar fora das fronteiras
        if(0 <= x_inicial && x_inicial < WINDOW_X && 0 <= y && y < WINDOW_Y)
        {
            if(z_buffer[y][x_inicial].first < inicio->z)
            {
                z_buffer[y][x_inicial].first = inicio->z;

                z_buffer[y][x_inicial].second = inicio->color;
            }

        }
        delete inicio;
    }
    else
        {

        x_final = (int)round(fim->x);

        //Calcula delta
        double deltaX = fim->x - inicio->x;
        double deltaZ = fim->z - inicio->z;

        double Tz;
        Tz = 0;

        if(deltaX != 0)
        {
            //Calcula taxa
            Tz = deltaZ/std::abs(deltaX);
        }
        //Corta a scanline caso ela passe da janela
        if(x_inicial >= WINDOW_X){
            delete inicio;
            delete fim;
            return;
        }
        if(x_inicial < 0)
        {
            //Se os dois estão fora tanto o inicio quanto o fim da scanline não faz nada
            if(x_final < 0){
                delete inicio;
                delete fim;
                return;
            }
            else
            {
                double deslocamento = (- inicio->x);
                x_inicial = 0;
                inicio->z = inicio->z + Tz*deslocamento;
            }

        }
        // Se o fim do vetor estiver fora ele "recorta"
        if(x_final >= WINDOW_X)
        {
            double deslocamento = fim->x - (WINDOW_X-1);
            x_final = WINDOW_X - 1;
            fim->z = fim->z - Tz*deslocamento;
        }
        // Anda por todo os x's da scanline

        if(z_buffer[y][x_inicial].first < inicio->z)
        {
            {
                z_buffer[y][x_inicial].first = inicio->z;
                z_buffer[y][x_inicial].second = sf::Color::Green;
            }
        }

        if(z_buffer[y][x_final].first < fim->z)
        {
            {
                z_buffer[y][x_final].first = fim->z;
                z_buffer[y][x_final].second = sf::Color::Green;
            }
        }

        //Seta os valores iniciais da scanline que são os valores do primeiro vertice da scanline
        double z_atual;
        z_atual = inicio->z;

        for(int i = x_inicial+1; i< x_final; i++)
        {
            // Se o Z da scanline for maior ele atualiza a cor dele
            if(z_buffer[y][i].first < z_atual)
            {
                z_buffer[y][i].first = z_atual;

                z_buffer[y][i].second = sf::Color::Black;
            }
            // Incrementa os valores para continuar andando pela scanline
            z_atual += Tz;
        }
        //deleta os point's para limpar memoria
        delete(inicio);
        delete(fim);
    }
}

void wire_frame_face(std::pair <double, sf::Color> **z_buffer,
                     index_t face,
                     const config &conf,
                     const std::vector<vec3f> &src_points)
{
    std::vector<index_t> index_vertexes = conf.terrain.get_face_vertexes(face);
    std::pair<int,int> maior_menor = get_maior_menor(index_vertexes, src_points);
    bool out_of_bounds = false;
    //Point **vetor = criar_vetor(index_vertexes, position, out_of_bounds, tamanho, src_points);
    Point **vetor = new Point*[WINDOW_Y];
    for(int i =0; i< WINDOW_Y; i++)
        vetor[i] = NULL;

    for(int64_t i = 0; i < (int64_t)index_vertexes.size(); i++)
    {
        vec3f ponto1 = src_points[index_vertexes[i]];
        vec3f ponto2;

        int inc;
        // Se o vertice for o ultimo ele volta para o primeiro
        if(i == (int64_t)index_vertexes.size() - 1)
        {
            ponto2 = src_points[index_vertexes[0]];
        }
        else
        {
            ponto2 = src_points[index_vertexes[i+1]];
        }
        // Calculo dos delta
        double deltaX = ponto2.x - ponto1.x;
        double deltaY = ponto2.y - ponto1.y;
        double deltaZ = ponto2.z - ponto1.z;

        double Tx, Tz;
        Tx = Tz = 0.0;

        if(deltaY != 0)
        {
            // Calcula as taixas, e para a cor calcula o delta e a taixa
            Tx = deltaX/std::abs(deltaY);
            Tz = deltaZ/std::abs(deltaY);
        }

        int y = (int)round(ponto1.y);

        //Verifica se o vertice ta dentro da tela e seta out_of_bouns dependendo da conclusão
        if(0 <= y && y < WINDOW_Y)
            out_of_bounds = false;
        else
            out_of_bounds = true;

        /*
        /Se o ponto está dentro das fronteiras, o vertice é adicionado na lista
        /de forma a garantir que o ponto esteja na lista, e por questões de consistencia
        / do index position
        */
        if(out_of_bounds == false)
        {
            Point *novo = new Point(index_vertexes[i],ponto1.x,y,ponto1.z,sf::Color::Green);
            if(vetor[y] == NULL)
                vetor[y] = novo;
            else
                vetor[y]->next = novo;

        }

        // Verifica se ta subindo ou descendo
        if(deltaY > 0)
            inc = UP;
        else
            inc = DOWN;

        //Seta os valores para o ponto logo abaixo/acima do vertice para sua primeira iteração
        int y_atual, y_final;
        double x_atual, z_atual;
        y_final = (int)round(ponto2.y);
        y_atual = y + inc;
        x_atual = ponto1.x + Tx;
        z_atual = ponto1.z + Tz;

        //Loop para a variação em y, ou seja, para cada scanline
        for(; y_atual*inc < y_final*inc;)
        {
            /*Se o y estiver dentro das fronteiras ele atualiza para poder processa-lo
            /alem disso ele faz isso de forma que ele atualize e possa processar o proximo valor
            /que no caso sera a fronteira da janela
            */
            if(out_of_bounds == true)
                if(0 <= y_atual && y_atual < WINDOW_Y)
                    out_of_bounds = false;

            if(out_of_bounds == false)
            {
                //Verifica se esta na janela
                if(y_atual < 0 || y_atual >= WINDOW_Y)
                {
                    out_of_bounds = true;
                }
                else
                {
                    //Adiciona um ponto no vetor
                    Point *novo = new Point(index_vertexes[i], x_atual,y_atual,z_atual,sf::Color::Green);

                    if(vetor[y_atual] == NULL)
                        vetor[y_atual] = novo;
                    else
                        vetor[y_atual]->next = novo;
                }
            }
            //Atualiza para toda iteração
            y_atual += inc;
            x_atual += Tx;
            z_atual += Tz;
        }

    }
    for(int i = maior_menor.second; i<= maior_menor.first; i++)
    {
        if(vetor[i] == NULL)
            continue;
        if(vetor[i]->next == NULL || vetor[i]->x == vetor[i]->next->x)
            scanline_wire(z_buffer,vetor[i], NULL);
        else
        {
            if(vetor[i]->x < vetor[i]->next->x)
                scanline_wire(z_buffer,vetor[i], vetor[i]->next);
            else
                scanline_wire(z_buffer,vetor[i]->next, vetor[i]);
        }

    }

    delete[] vetor;
}

#endif
