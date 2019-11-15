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

    buffer.create(WINDOW_X, WINDOW_Y);

    half_mesh terrain_mesh;

    uint64_t mx = 60;
    uint64_t my = 70;

    noise make_some_noise(WINDOW_X, WINDOW_Y);

    make_some_noise.gen(time(NULL));

    terrain_mesh.build_mesh(mx, my, [&make_some_noise, &mx](index_t index) -> std::tuple<float, float, float> {
            uint64_t i = index / mx;
            uint64_t j = index % mx;

            return std::make_tuple((float)i, (float)j, (float)make_some_noise[i][j]);
    });



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
                make_some_noise.smooth();
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

                    auto srt = terrain_mesh.points[dir.first];
                    auto end = terrain_mesh.points[dir.second];

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
        win.display();
    }

    for(std::size_t i = 0; i < WINDOW_Y; i++){
        delete[] noise_map[i];
    }
    delete[] noise_map;

    return 0;
}
