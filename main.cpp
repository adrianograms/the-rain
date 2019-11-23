#include <iostream>
#include <ctime>

#include <SFML/Graphics.hpp>

#include "include/button.hpp"
#include "include/noise.hpp"
#include "include/half_mesh.hpp"
#include "include/vec3f.hpp"

#define WINDOW_Y 768
#define WINDOW_X 1337

// TODO list:
// menus
// save to file
// load from file
// camera
// pipeline


sf::RenderWindow win(sf::VideoMode(WINDOW_X, WINDOW_Y), ":(", sf::Style::Close | sf::Style::Titlebar);

int main(){
    std::vector<button> menu;

    view_mode curr_view;

    button save("assets/bs.png", {30, 60}, [](){
        std::cout << "save pressed" << std::endl;
    });

    button load("assets/bs.png", {90, 60}, [](){
        std::cout << "load pressed" << std::endl;
    });
    // menu.push_back(button("assets/bs.png", {30, 60}, [](){
    //     std::cout << "save pressed" << std::endl;
    // }));
    // menu.push_back(button("assets/sad.png", {60, 60}, [](){
    //     std::cout << "load pressed" << std::endl;
    // }));
    // menu.push_back(button("assets/bs.png", {90, 90}, [](){
    //     std::cout << "texture pressed" << std::endl;
    // }));
    //     menu.push_back(button("assets/bs.png", {120, 120}, []{
    //      std::cout << "gen pressed" << std::endl;
    // }));
    // menu.push_back(button("assets/bs.png", {150, 150}, []{
    //     std::cout << "smoth pressed" << std::endl;
    // }));
    // menu.push_back(button("assest/bs.png", {300, 300}, [&curr_view](){
    //     curr_view = WIREFRAME;
    // }));
    // menu.push_back(button("assest/bs.png", {320, 300}, [&curr_view](){
    //     curr_view = FLAT;
    // }));
    // menu.push_back(button("assest/bs.png", {320, 300}, [&curr_view](){
    //     curr_view = GOURAUD;
    // }));

    half_mesh terrain;

    uint64_t mx = 2;
    uint64_t my = 2;

    noise height(WINDOW_X, WINDOW_Y);

    height.gen(time(NULL));

    terrain.build_mesh(mx, my, [&height, &mx](index_t index) -> vec3f {
            uint64_t i = index / mx;
            uint64_t j = index % mx;

            return vec3f((float)i, (float)j, (float)height[i][j]);
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
                    // referência de como fazer algumas coisas
                    // pontos de uma face
                    for(auto face : terrain.face_vector){
                        std::cout << "face -> " << "half_edge ->  "<< face << std::endl;
                        auto vertexes = terrain.get_face_vertexes(face);

                        for(auto vertex : vertexes){
                            auto p = terrain.points[vertex];
                            std::cout << "\t" << p.x << "," << p.y << "," << p.z << std::endl;
                        }
                    }
                    // normal da face
                    for(index_t i = 0; i < (index_t)terrain.face_vector.size(); i++){
                        vec3f normal = terrain.get_face_normal(i);
                        std::cout << "normal da face: " << i << " "
                                  << "(" << normal.x << ","
                                  << normal.y << ","
                                  << normal.z << ")" << std::endl;
                    }
                    break;
                }
                break;
            case sf::Event::MouseButtonPressed:
                switch(ev.mouseButton.button){
                case sf::Mouse::Left:
                    save.button_pressed({(float)ev.mouseButton.x, (float)ev.mouseButton.y});
                    load.button_pressed({(float)ev.mouseButton.x, (float)ev.mouseButton.y});
                    break;
                default:
                    break;
                }
                break;
            default:
                for(index_t i : terrain.edge_vector){
                    sf::VertexArray lines(sf::Lines, 2);
                    std::pair<index_t, index_t> dir = terrain.half_direction(i);

                    auto srt = terrain.points[dir.first];
                    auto end = terrain.points[dir.second];

                    // just for testing
                    lines[0].position.x = (srt.x + 10) * 10;
                    lines[0].position.y = (srt.y + 10) * 10;
                    lines[0].color = sf::Color::Green;

                    lines[1].position.x = (end.x + 10) * 10;
                    lines[1].position.y = (end.y + 10) * 10;
                    lines[1].color = sf::Color::Blue;

                    win.draw(lines);
                }
                break;
            }
        }

        // ultar face
        // pipeline
        // aqui pinta

        // win.draw(save);
        // win.draw(load);
        win.display();
    }
    return 0;
}
