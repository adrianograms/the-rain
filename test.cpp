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

// --------------------------------------------------------------------------------------------------------------------
// Variaveis Globais
// --------------------------------------------------------------------------------------------------------------------

sf::Image imagem;
double Ila, Kd, ka, Ks, lpoint, n;
sf::Color background, line;
std::vector<vec3f> vertexes;
half_mesh terrain;
vec3f L, VRP;

// Aplica o brilho nas cores
void apply_light(sf::Color &cor, double Il)
{
    cor.r = cor.r * (Il/255);
    cor.b = cor.b * (Il/255);
    cor.g = cor.g * (Il/255);
}

//Pega a cor do vertex
sf::Color getColor(index_t vertex) { return imagem.getPixel(vertex%WINDOW_X, vertex/WINDOW_X);}

void scanline(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim);

Point **criar_vetor(std::vector<index_t> &pontos, index_t &index, bool &out_of_bounds, int &tam) {
    int maior, menor;
    maior = menor = vertexes[pontos[0]].y;
    // Descobre o maior entre os vertices
    for(int i =1; i< pontos.size(); i++)
    {
        if(vertexes[pontos[i]].y > maior)
            maior = vertexes[pontos[i]].y;
        else if(vertexes[pontos[i]].y < menor)
            menor = vertexes[pontos[i]].y;
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
        if(vertexes[pontos[0]].y >= WINDOW_Y)
        {
            out_of_bounds = true;
            index = tamanho-1;

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

// --------------------------------------------------------------------------------------------------------------------------
// Guro
// --------------------------------------------------------------------------------------------------------------------------


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
        vec3f lVertice = L - terrain.points[pontos[i]]; // Pontos no SRU
        lVertice.normlize();
        float product = normal.dot(lVertice);

        double Id = 0;
        double Is = 0;

        if(product > 0) 
        {
            Id = lpoint * Kd * product;

            vec3f rVertice = ((normal) * ((lVertice*2.0).dot(normal))) - lVertice;
            rVertice.normlize();

            vec3f sVertice = VRP - terrain.points[pontos[i]];
            sVertice.normlize();

            double product2 = rVertice.dot(sVertice);

            if(product2 > 0)
                Is = lpoint * Ks * pow(product2,n);
        }

        double It = Iambiente + Id + Is;

        result.push_back(It);
    }

    return result;
}

void scanline(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim)
{
    // Se fim não existe então é necessario apenas pintar um pixel
    if(fim == NULL)
    {
        //Porem o vertice pode estar fora das fronteiras
        if(0 <= inicio->x < WINDOW_X && 0 <= inicio->y < WINDOW_Y)
        {
            if(z_buffer[(int)inicio->x][(int)inicio->y].first > inicio->z)
            {
                z_buffer[(int)inicio->x][(int)inicio->y].first = inicio->z;

                z_buffer[(int)inicio->x][(int)inicio->y].second = inicio->color;
            }
        }
    }
    else
    {

        //Calcula delta
        double deltaX = fim->x - inicio->x;
        double deltaZ = fim->z - inicio->z;

        double Tz;
        Tz = 0;

        TxColor txColor = {0.0,0.0,0.0};

        if(deltaX != 0)
        {
            //Calcula taxa
            Tz = deltaZ/abs(deltaX);
            txColor.calc(fim->color, inicio->color, deltaX);
        }
        //Corta a scanline caso ela passe da janela
        if(inicio->x < 0)
        {
            //Se os dois estão fora tanto o inicio quanto o fim da scanline não faz nada
            if(fim->x < 0)
                return;
            else
            {
                double deslocamento = (- inicio->x);
                inicio->x = 0;
                inicio->z = inicio->z + Tz*deslocamento;
                inicio->color =  (txColor * deslocamento) + inicio->color ;
            }
            
        }
        // Se o fim do vetor estiver fora ele "recorta"
        if(fim->x >= WINDOW_X)
        {
            double deslocamento = fim->x - (WINDOW_X-1);
            fim->x = WINDOW_X;
            fim->z = fim->z - Tz*deslocamento;
            inicio->color =  (txColor * (-deslocamento)) + inicio->color ;
        }
        // Anda por todo os x's da scanline
        double condition = fim->x - inicio->x;

        //Seta os valores iniciais da scanline que são os valores do primeiro vertice da scanline
        double x_atual,y , z_atual;
        sf::Color color;
        x_atual = inicio->x;
        y = inicio->y;
        z_atual = inicio->z;
        color = inicio->color;

        for(int i =0; i<= floor(condition); i++)
        {
            // Se o Z da scanline for maior ele atualiza a cor dele
            if(z_buffer[(int)x_atual][(int)y].first > z_atual)
            {
                z_buffer[(int)x_atual][(int)y].first = z_atual;

                z_buffer[(int)x_atual][(int)y].second = color;
            }
            // Incrementa os valores para continuar andando pela scanline
            x_atual++;
            z_atual += Tz;
            color = txColor + color;
        }
        //deleta os point's para limpar memoria
        delete(inicio);
        delete(fim);
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
        // Calculo dos delta
        double deltaX = ponto2.x - ponto1.x;
        double deltaY = ponto2.y - ponto1.y;
        double deltaZ = ponto2.z - ponto1.z;

        double Tx, Tz;
        Tx = Tz = 0.0;

        TxColor txColor = {0.0,0.0,0.0};

        if(deltaY != 0) 
        {
            // Calcula as taixas, e para a cor calcula o delta e a taixa
            Tx = deltaX/std::abs(deltaY);
            Tz = deltaZ/std::abs(deltaY);
            txColor.calc(cor2, cor1, deltaY);
        }

        //Verifica se o vertice ta dentro da tela e seta out_of_bouns dependendo da conclusão
        if(0 <= ponto1.y < WINDOW_Y)
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
            Point *novo = new Point(index_vertexes[i],ponto1.x,ponto1.y,ponto1.z,cor1);
            if(vetor[position] == NULL)
                vetor[position] = novo;
            else
                vetor[position]->next = novo;
            
        }

        // Verifica se ta subindo ou descendo
        if(deltaY > 0)
            inc = UP;
        else
            inc = DOWN;

        //Seta os valores para o ponto logo abaixo/acima do vertice para sua primeira iteração
        int x_atual, y_atual, z_atual;
        sf::Color cor_atual;
        y_atual = round(ponto1.y) + inc;
        x_atual = ponto1.x + Tx;
        z_atual = ponto1.z + Tz;
        cor_atual = txColor + cor1;

        //Sempre verifica se o y está na fronteiras para atualizar o position
        if(0 <= y_atual < WINDOW_Y)
            position += inc;
        
        //Loop para a variação em y, ou seja, para cada scanline
        for(int j = 1; j < abs(round(deltaY)); j++)
        {
            /*Se o y estiver dentro das fronteiras ele atualiza para poder processa-lo
            /alem disso ele faz isso de forma que ele atualize e possa processar o proximo valor
            /que no caso sera a fronteira da janela
            */
            if(out_of_bounds == true)
                if(0 <= y_atual < WINDOW_Y)
                    out_of_bounds = false;
            
            if(out_of_bounds == false)
            {
                //Verifica se esta na janela
                if(y_atual < 0 || y_atual >= WINDOW_Y)
                {
                    out_of_bounds = true;
                    continue;
                }
                else
                {
                    //Adiciona um ponto no vetor
                    Point *novo = new Point(index_vertexes[i], x_atual,y_atual,z_atual,cor_atual);

                    if(vetor[position] == NULL)
                        vetor[position] = novo;
                    else
                        vetor[position]->next = novo;
            
                    //Só atualiza quando ele consegue se mover dentro pelo vetor
                    if(0 <= y_atual < WINDOW_Y)
                        position += inc;
                }
            }
            //Atualiza para toda iteração
            y_atual += inc;
            x_atual += Tx;
            z_atual += Tz;
            cor_atual = txColor + cor_atual;

        }
        
    }
    /*
    /Processa para todas as scanline de baixo para cima
    */
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

double calc_ilumination_flat(std::vector<index_t> &indexes, index_t face)
{
    double x_bigger, y_bigger, z_bigger, x_smaller, y_smaller, z_smaller;
    x_bigger = x_smaller = terrain.points[indexes[0]].x;
    y_bigger = y_smaller = terrain.points[indexes[0]].y;
    z_bigger = z_smaller = terrain.points[indexes[0]].z;
    for(int i = 1; i < indexes.size(); i++)
    {
        //Maior e menor x
        if(x_bigger < terrain.points[indexes[i]].x)
        {
            x_bigger = terrain.points[indexes[i]].x;
        }
        else if(x_smaller > terrain.points[indexes[i]].x)
        {
            x_smaller = terrain.points[indexes[i]].x;
        }

        //Maior e menor y
        if(y_bigger < terrain.points[indexes[i]].y)
        {
            y_bigger = terrain.points[indexes[i]].y;
        }
        else if(y_smaller > terrain.points[indexes[i]].y)
        {
            y_smaller = terrain.points[indexes[i]].y;
        }

        //Maior e menor z
        if(z_bigger < terrain.points[indexes[i]].z)
        {
            z_bigger = terrain.points[indexes[i]].z;
        }
        else if(z_smaller > terrain.points[indexes[i]].z)
        {
            z_smaller = terrain.points[indexes[i]].z;
        }
    }

    double x_avg = (x_bigger + x_smaller)/2;
    double y_avg = (y_bigger + y_smaller)/2;
    double z_avg = (z_bigger + z_smaller)/2;
    vec3f avg(x_avg,y_avg,z_avg);

    double IlAmbiente = Ila*ka;

    vec3f face_normal = terrain.get_face_normal(face);
    face_normal.normlize();

    vec3f l = l - avg;
    l.normlize();

    double product = face_normal.dot(l);

    double Id, Is;
    Id = Is = 0.0;
    if(product > 0)
    {
        Id = lpoint * Kd * product;

        vec3f rVertice = ((face_normal) * ((l*2.0).dot(face_normal))) - l;
        rVertice.normlize();

        vec3f sVertice = VRP - avg;
        sVertice.normlize();

        double product2 =  rVertice.dot(sVertice);

        if(product2 > 0)
        {
            Is = lpoint * Ks * pow(product2,n);
        }

        return IlAmbiente + Id + Is;
    }
}

void scanline_flat(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim)
{
    // Se fim não existe então é necessario apenas pintar um pixel
    if(fim == NULL)
    {
        //Porem o vertice pode estar fora das fronteiras
        if(0 <= inicio->x < WINDOW_X && 0 <= inicio->y < WINDOW_Y)
        {
            if(z_buffer[(int)inicio->x][(int)inicio->y].first > inicio->z)
            {
                z_buffer[(int)inicio->x][(int)inicio->y].first = inicio->z;

                z_buffer[(int)inicio->x][(int)inicio->y].second = inicio->color;
            }
        }
    }
    else
    {

        //Calcula delta
        double deltaX = fim->x - inicio->x;
        double deltaZ = fim->z - inicio->z;

        double Tz;
        Tz = 0;

        if(deltaX != 0)
        {
            //Calcula taxa
            Tz = deltaZ/abs(deltaX);
        }
        //Corta a scanline caso ela passe da janela
        if(inicio->x < 0)
        {
            //Se os dois estão fora tanto o inicio quanto o fim da scanline não faz nada
            if(fim->x < 0)
                return;
            else
            {
                double deslocamento = (- inicio->x);
                inicio->x = 0;
                inicio->z = inicio->z + Tz*deslocamento;
            }
            
        }
        // Se o fim do vetor estiver fora ele "recorta"
        if(fim->x >= WINDOW_X)
        {
            double deslocamento = fim->x - (WINDOW_X-1);
            fim->x = WINDOW_X;
            fim->z = fim->z - Tz*deslocamento;
        }
        // Anda por todo os x's da scanline
        double condition = fim->x - inicio->x;

        //Seta os valores iniciais da scanline que são os valores do primeiro vertice da scanline
        double x_atual,y , z_atual;
        sf::Color color;
        x_atual = inicio->x;
        y = inicio->y;
        z_atual = inicio->z;
        color = inicio->color;

        for(int i =0; i<= floor(condition); i++)
        {
            // Se o Z da scanline for maior ele atualiza a cor dele
            if(z_buffer[(int)x_atual][(int)y].first > z_atual)
            {
                z_buffer[(int)x_atual][(int)y].first = z_atual;

                z_buffer[(int)x_atual][(int)y].second = color;
            }
            // Incrementa os valores para continuar andando pela scanline
            x_atual++;
            z_atual += Tz;
        }
        //deleta os point's para limpar memoria
        delete(inicio);
        delete(fim);
    }
}

void flat_shading_face(std::pair <double, sf::Color> **z_buffer, index_t face)
{
    std::vector<index_t> index_vertexes = terrain.get_face_vertexes(face);
    index_t position = 0;
    int tamanho = 0;
    bool out_of_bounds = false;
    Point **vetor = criar_vetor(index_vertexes, position, out_of_bounds, tamanho);
    double light = calc_ilumination_flat(index_vertexes, face);

    sf::Color cor1, cor2, cor3, cor;
    cor1 = getColor(index_vertexes[0]);
    cor2 = getColor(index_vertexes[1]);
    cor3 = getColor(index_vertexes[2]);

    cor = avg_color(cor1, cor2, cor3);
    apply_light(cor, light);

    for(int i =0; i< index_vertexes.size(); i++)
    {
        vec3f ponto1 = vertexes[index_vertexes[i]];
        vec3f ponto2;

        int inc;
        // Se o vertice for o ultimo ele volta para o primeiro
        if(i == 2)
        {
            ponto2 = vertexes[index_vertexes[0]];
        }
        else 
        {
            ponto2 = vertexes[index_vertexes[i+1]];
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

        //Verifica se o vertice ta dentro da tela e seta out_of_bouns dependendo da conclusão
        if(0 <= ponto1.y < WINDOW_Y)
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
            Point *novo = new Point(index_vertexes[i],ponto1.x,ponto1.y,ponto1.z,cor);
            if(vetor[position] == NULL)
                vetor[position] = novo;
            else
                vetor[position]->next = novo;
            
        }

        // Verifica se ta subindo ou descendo
        if(deltaY > 0)
            inc = UP;
        else
            inc = DOWN;

        //Seta os valores para o ponto logo abaixo/acima do vertice para sua primeira iteração
        int x_atual, y_atual, z_atual;
        y_atual = round(ponto1.y) + inc;
        x_atual = ponto1.x + Tx;
        z_atual = ponto1.z + Tz;

        //Sempre verifica se o y está na fronteiras para atualizar o position
        if(0 <= y_atual < WINDOW_Y)
            position += inc;
        
        //Loop para a variação em y, ou seja, para cada scanline
        for(int j = 1; j < abs(round(deltaY)); j++)
        {
            /*Se o y estiver dentro das fronteiras ele atualiza para poder processa-lo
            /alem disso ele faz isso de forma que ele atualize e possa processar o proximo valor
            /que no caso sera a fronteira da janela
            */
            if(out_of_bounds == true)
                if(0 <= y_atual < WINDOW_Y)
                    out_of_bounds = false;
            
            if(out_of_bounds == false)
            {
                //Verifica se esta na janela
                if(y_atual < 0 || y_atual >= WINDOW_Y)
                {
                    out_of_bounds = true;
                    continue;
                }
                else
                {
                    //Adiciona um ponto no vetor
                    Point *novo = new Point(index_vertexes[i], x_atual,y_atual,z_atual,cor);

                    if(vetor[position] == NULL)
                        vetor[position] = novo;
                    else
                        vetor[position]->next = novo;
            
                    //Só atualiza quando ele consegue se mover dentro pelo vetor
                    if(0 <= y_atual < WINDOW_Y)
                        position += inc;
                }
            }
            //Atualiza para toda iteração
            y_atual += inc;
            x_atual += Tx;
            z_atual += Tz;
        }
        
    }
    for(int i =0; i< tamanho; i++)
    {
        if(vetor[i]->next == NULL || vetor[i]->x == vetor[i]->next->x)
            scanline(z_buffer,vetor[i], NULL);
        else
        {
            if(vetor[i]->x < vetor[i]->next->x)
                scanline_flat(z_buffer,vetor[i], vetor[i]->next);
            else
                scanline_flat(z_buffer,vetor[i]->next, vetor[i]);
        }
        
    }

    delete[] vetor;
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Wireframe
// -----------------------------------------------------------------------------------------------------------------------------------------------------

void scanline_wire(std::pair <double, sf::Color> **z_buffer, Point *inicio, Point *fim)
{
    // Se fim não existe então é necessario apenas pintar um pixel
    if(fim == NULL)
    {
        //Porem o vertice pode estar fora das fronteiras
        if(0 <= inicio->x < WINDOW_X && 0 <= inicio->y < WINDOW_Y)
        {
            if(z_buffer[(int)inicio->x][(int)inicio->y].first > inicio->z)
            {
                z_buffer[(int)inicio->x][(int)inicio->y].first = inicio->z;

                z_buffer[(int)inicio->x][(int)inicio->y].second = inicio->color;
            }
        }
    }
    else
    {

        //Calcula delta
        double deltaX = fim->x - inicio->x;
        double deltaZ = fim->z - inicio->z;

        double Tz;
        Tz = 0;

        if(deltaX != 0)
        {
            //Calcula taxa
            Tz = deltaZ/abs(deltaX);
        }
        //Corta a scanline caso ela passe da janela
        if(inicio->x < 0)
        {
            //Se os dois estão fora tanto o inicio quanto o fim da scanline não faz nada
            if(fim->x < 0)
                return;
            else
            {
                double deslocamento = (- inicio->x);
                inicio->x = 0;
                inicio->z = inicio->z + Tz*deslocamento;
            }
            
        }
        // Se o fim do vetor estiver fora ele "recorta"
        if(fim->x >= WINDOW_X)
        {
            double deslocamento = fim->x - (WINDOW_X-1);
            fim->x = WINDOW_X;
            fim->z = fim->z - Tz*deslocamento;
        }
        // Anda por todo os x's da scanline
        double condition = fim->x - inicio->x;

        if(z_buffer[(int)inicio->x][(int)inicio->y].first > inicio->z)
        {
            {
                z_buffer[(int)inicio->x][(int)inicio->y].first = inicio->z;
                z_buffer[(int)inicio->x][(int)inicio->y].second = line;
            }
        }

        if(z_buffer[(int)fim->x][(int)fim->y].first > fim->z)
        {
            {
                z_buffer[(int)fim->x][(int)fim->y].first = fim->z;
                z_buffer[(int)fim->x][(int)fim->y].second = line;
            }
        }

        //Seta os valores iniciais da scanline que são os valores do primeiro vertice da scanline
        double x_atual,y , z_atual;
        x_atual = inicio->x;
        y = inicio->y;
        z_atual = inicio->z;

        for(int i =1; i< floor(condition); i++)
        {
            // Se o Z da scanline for maior ele atualiza a cor dele
            if(z_buffer[(int)x_atual][(int)y].first > z_atual)
            {
                z_buffer[(int)x_atual][(int)y].first = z_atual;

                z_buffer[(int)x_atual][(int)y].second = background;
            }
            // Incrementa os valores para continuar andando pela scanline
            x_atual++;
            z_atual += Tz;
        }
        //deleta os point's para limpar memoria
        delete(inicio);
        delete(fim);
    }
}

void wire_frame_face(std::pair <double, sf::Color> **z_buffer, index_t face)
{
    std::vector<index_t> index_vertexes = terrain.get_face_vertexes(face);
    index_t position = 0;
    int tamanho = 0;
    bool out_of_bounds = false;
    Point **vetor = criar_vetor(index_vertexes, position, out_of_bounds, tamanho);

    for(int i =0; i< index_vertexes.size(); i++)
    {
        vec3f ponto1 = vertexes[index_vertexes[i]];
        vec3f ponto2;

        int inc;
        // Se o vertice for o ultimo ele volta para o primeiro
        if(i == 2)
        {
            ponto2 = vertexes[index_vertexes[0]];
        }
        else 
        {
            ponto2 = vertexes[index_vertexes[i+1]];
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

        //Verifica se o vertice ta dentro da tela e seta out_of_bouns dependendo da conclusão
        if(0 <= ponto1.y < WINDOW_Y)
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
            Point *novo = new Point(index_vertexes[i],ponto1.x,ponto1.y,ponto1.z,line);
            if(vetor[position] == NULL)
                vetor[position] = novo;
            else
                vetor[position]->next = novo;
            
        }

        // Verifica se ta subindo ou descendo
        if(deltaY > 0)
            inc = UP;
        else
            inc = DOWN;

        //Seta os valores para o ponto logo abaixo/acima do vertice para sua primeira iteração
        int x_atual, y_atual, z_atual;
        y_atual = round(ponto1.y) + inc;
        x_atual = ponto1.x + Tx;
        z_atual = ponto1.z + Tz;

        //Sempre verifica se o y está na fronteiras para atualizar o position
        if(0 <= y_atual < WINDOW_Y)
            position += inc;
        
        //Loop para a variação em y, ou seja, para cada scanline
        for(int j = 1; j < abs(round(deltaY)); j++)
        {
            /*Se o y estiver dentro das fronteiras ele atualiza para poder processa-lo
            /alem disso ele faz isso de forma que ele atualize e possa processar o proximo valor
            /que no caso sera a fronteira da janela
            */
            if(out_of_bounds == true)
                if(0 <= y_atual < WINDOW_Y)
                    out_of_bounds = false;
            
            if(out_of_bounds == false)
            {
                //Verifica se esta na janela
                if(y_atual < 0 || y_atual >= WINDOW_Y)
                {
                    out_of_bounds = true;
                    continue;
                }
                else
                {
                    //Adiciona um ponto no vetor
                    Point *novo = new Point(index_vertexes[i], x_atual,y_atual,z_atual,line);

                    if(vetor[position] == NULL)
                        vetor[position] = novo;
                    else
                        vetor[position]->next = novo;
            
                    //Só atualiza quando ele consegue se mover dentro pelo vetor
                    if(0 <= y_atual < WINDOW_Y)
                        position += inc;
                }
            }
            //Atualiza para toda iteração
            y_atual += inc;
            x_atual += Tx;
            z_atual += Tz;
        }
        
    }
    for(int i =0; i< tamanho; i++)
    {
        if(vetor[i]->next == NULL || vetor[i]->x == vetor[i]->next->x)
            scanline(z_buffer,vetor[i], NULL);
        else
        {
            if(vetor[i]->x < vetor[i]->next->x)
                scanline_flat(z_buffer,vetor[i], vetor[i]->next);
            else
                scanline_flat(z_buffer,vetor[i]->next, vetor[i]);
        }
        
    }

    delete[] vetor;
}


int main() {

}