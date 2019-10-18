#include <iostream>
#include <ctime>

#include <SFML/Graphics.hpp>

#include "include/noise.hpp"
#include "include/half_mesh.hpp"

#define K_SIZE 7
#define WINDOW_Y 768
#define WINDOW_X 1337

//teste 1

sf::RenderWindow win(sf::VideoMode(WINDOW_X, WINDOW_Y), "make some moise");

sf::Image   buffer;
sf::Texture texture;
sf::Sprite  sprite;

int main(){
    std::srand(std::time(NULL));

    uint16_t **k = new uint16_t*[K_SIZE];
    for(std::size_t i = 0; i < K_SIZE; i++){
        k[i] = new uint16_t[K_SIZE];
    }
    // // 7 x 7 kernel
    k[0][0]=1;  k[0][1]=6;   k[0][2]=15;  k[0][3]=20;  k[0][4]=15;  k[0][5]=6;   k[0][6]=1;
    k[1][0]=6;  k[1][1]=36;  k[1][2]=90;  k[1][3]=120; k[1][4]=90;  k[1][5]=36;  k[1][6]=6;
    k[2][0]=15; k[2][1]=90;  k[2][2]=225; k[2][3]=300; k[2][4]=225; k[2][5]=90;  k[2][6]=15;
    k[3][0]=20; k[3][1]=120; k[3][2]=300; k[3][3]=400; k[3][4]=300; k[3][5]=120; k[3][6]=20;
    k[4][0]=15; k[4][1]=90;  k[4][2]=225; k[4][3]=300; k[4][4]=225; k[4][5]=90;  k[4][6]=15;
    k[5][0]=6;  k[5][1]=36;  k[5][2]=90;  k[5][3]=120; k[5][4]=90;  k[5][5]=36;  k[5][6]=6;
    k[6][0]=1;  k[6][1]=6;   k[6][2]=15;  k[6][3]=20;  k[6][4]=15;  k[6][5]=6;   k[6][6]=1;

    half_mesh terrain_mesh;

    uint64_t mx = 60;
    uint64_t my = 70;

    noise make_some_noise(WINDOW_X, WINDOW_Y);

    make_some_noise.gen(time(NULL));

    terrain_mesh.build_mesh(mx, my, [&make_some_noise, &mx](index_t index) -> std::tuple<float, float, float>{
            uint64_t i = index / mx;
            uint64_t j = index % mx;

            return std::make_tuple((float)i, (float)j, (float)make_some_noise.at(i, j));
    });

    buffer.create(WINDOW_X, WINDOW_Y);

    auto draw_image = [&](){
        texture.loadFromImage(buffer);
        sprite.setTexture(texture);
        win.draw(sprite);
        win.display();
    };

    // allocate noise_map matrix
    uint8_t **noise_map = new uint8_t*[WINDOW_Y];
    for(std::size_t i = 0; i < WINDOW_Y; i++){
        noise_map[i] = new uint8_t[WINDOW_X];
    }



    while(win.isOpen()){
        sf::Event ev;
        while(win.pollEvent(ev)){
            switch(ev.type){
            case sf::Event::Closed:
                win.close();
                break;
            case sf::Event::MouseButtonPressed:
                std::cout << "smoothing" << std::endl;
                make_some_noise.smooth(k, K_SIZE);
            default:
// #pragma omp parallel for schedule(dynamic, 1) collapse(2)
                // for(std::size_t i = 0; i < WINDOW_Y; i++){
                //     for(std::size_t j = 0; j < WINDOW_X; j++){
                //         uint8_t rgb = make_some_noise.at(j, i);
                //         buffer.setPixel(j, i, sf::Color(rgb, rgb, 0));
                //     }
                // }
                for(index_t i : terrain_mesh.edge_vector){
                    sf::VertexArray lines(sf::Lines, 2);
                    std::pair<index_t, index_t> dir = terrain_mesh.half_direction(i);

                    auto srt = terrain_mesh.vertexes[dir.first];
                    auto end = terrain_mesh.vertexes[dir.second];

                    // just for testing
                    lines[0].position.x = (std::get<0>(srt) + 10) * 10;
                    lines[0].position.y = (std::get<1>(srt) + 10) * 10;
                    lines[0].color = sf::Color::Green;
                    lines[1].position.x = (std::get<0>(end) + 10) * 10;
                    lines[1].position.y = (std::get<1>(end) + 10) * 10;
                    lines[1].color = sf::Color::Blue;

                    win.draw(lines);

                }

                break;
            }
        }
        // draw_image();
        win.display();
    }

    for(std::size_t i = 0; i < K_SIZE; i++){
          delete[] k[i];
     }
     delete[] k;

     for(std::size_t i = 0; i < WINDOW_Y; i++){
          delete[] noise_map[i];
     }
     delete[] noise_map;

    return 0;
}
