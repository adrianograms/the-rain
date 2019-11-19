#include <iostream>
#include <ctime>

#include <SFML/Graphics.hpp>

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

bool button_pressed(sf::FloatRect sprite, sf::Vector2f mouse){
    if (sprite.contains(mouse)){
        return true;
    }
    return false;
}

sf::RenderWindow win(sf::VideoMode(WINDOW_X, WINDOW_Y), ":(", sf::Style::Close | sf::Style::Titlebar);

int main(){
    sf::Texture bs_texture;
    if(!bs_texture.loadFromFile("assets/bs.png")){
        return 0;
    }
    sf::Sprite  bs_sprite(bs_texture);
    bs_sprite.setPosition(300, 300);

    half_mesh terrain_mesh;

    uint64_t mx = 60;
    uint64_t my = 70;

    noise make_some_noise(WINDOW_X, WINDOW_Y);

    make_some_noise.gen(time(NULL));

    terrain_mesh.build_mesh(mx, my, [&make_some_noise, &mx](index_t index) -> point {
            uint64_t i = index / mx;
            uint64_t j = index % mx;

            return point((float)i, (float)j, (float)make_some_noise[i][j]);
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
                    if(button_pressed(bs_sprite.getGlobalBounds(), {(float)ev.mouseButton.x, (float)ev.mouseButton.y})){
                        std::cout << ":(" << std::endl;
                    }
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
        win.draw(bs_sprite);
        win.display();
    }

    return 0;
}
