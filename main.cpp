#include <iostream>
#include <ctime>

#include <SFML/Graphics.hpp>

#include "include/button.hpp"
#include "include/noise.hpp"
#include "include/half_mesh.hpp"

#define WINDOW_Y 768
#define WINDOW_X 1337

// TODO list:
// menus
// save to file
// load from file
// camera
// pipeline

sf::RenderWindow win(sf::VideoMode(WINDOW_X, WINDOW_Y), ":(", sf::Style::Close | sf::Style::Titlebar);

std::vector<button> menu;

int main(){
    button save("assets/save_button.png", {30, 60}, [](){
        std::cout << "save pressed" << std::endl;
    });

    button load("assets/load_button.png", {60, 60}, [](){
        std::cout << "load pressed" << std::endl;
    });

    button texture("assets/texture_button.png", {90, 90}, [](){
        std::cout << "texture pressed" << std::endl;
    });

    button gen("assets/gen_button.png", {120, 120}, []{
        std::cout << "gen pressed" << std::endl;
    });

    button smoth("assets/smoth_button.png", {150, 150}, []{
        std::cout << "smoth pressed" << std::endl;
    });


    half_mesh terrain_mesh;

    uint64_t mx = 60;
    uint64_t my = 70;

    noise height(WINDOW_X, WINDOW_Y);

    height.gen(time(NULL));

    terrain_mesh.build_mesh(mx, my, [&height, &mx](index_t index) -> point {
            uint64_t i = index / mx;
            uint64_t j = index % mx;

            return point((float)i, (float)j, (float)height[i][j]);
    });

    while(win.isOpen()){

        sf::Event ev;

        while(win.pollEvent(ev)){
            switch(ev.type){
            case sf::Event::Closed:
                win.close();
                break;
            case sf::Event::KeyPressed:
                switch(ev.key.code){
                case sf::Keyboard::A:
                    // pan esquerda
                    break;
                case sf::Keyboard::S:
                    // pan direita
                    break;
                case sf::Keyboard::Q:
                    // pan cima
                    break;
                case sf::Keyboard::W:
                    // pan baixo
                    break;
                case sf::Keyboard::Z:
                    // aumenta o fov
                    break;
                case sf::Keyboard::X:
                    // diminui o fov
                    break;
                case sf::Keyboard::D:
                    // ponto focal esquerda
                    break;
                case sf::Keyboard::F:
                    // ponto focal direita
                    break;
                case sf::Keyboard::E:
                    // ponto focal para baixo
                    break;
                case sf::Keyboard::R:
                    // ponto focal para cima
                    break;
                case sf::Keyboard::G:
                    // camera para a esquerda
                    break;
                case sf::Keyboard::H:
                    // camera para a direita
                    break;
                case sf::Keyboard::T:
                    // camera para baixo
                    break;
                case sf::Keyboard::Y:
                    // camera para cima
                    break;
                default:
                    break;
                }
                break;
            case sf::Event::MouseButtonPressed:
                switch(ev.mouseButton.button){
                case sf::Mouse::Left:
                    save.button_pressed({(float)ev.mouseButton.x, (float)ev.mouseButton.y});
                    break;
                default:
                    break;
                }

                break;
            default:
                for(index_t i : terrain_mesh.edge_vector){
                    sf::VertexArray lines(sf::Lines, 2);
                    std::pair<index_t, index_t> dir = terrain_mesh.half_direction(i);

                    auto srt = terrain_mesh.points[dir.first];
                    auto end = terrain_mesh.points[dir.second];

                    // just for testing
                    lines[0].position.x = (srt.x() + 10) * 10;
                    lines[0].position.y = (srt.y() + 10) * 10;
                    lines[0].color = sf::Color::Green;
                    lines[1].position.x = (end.x() + 10) * 10;
                    lines[1].position.y = (end.y() + 10) * 10;
                    lines[1].color = sf::Color::Blue;

                    win.draw(lines);

                }

                break;
            }
        }
        win.draw(save);
        win.display();
    }

    return 0;
}
