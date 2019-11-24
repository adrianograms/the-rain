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


sf::RenderWindow win(sf::VideoMode(WINDOW_X, WINDOW_Y), ":(", sf::Style::Close | sf::Style::Titlebar);

int main(){
    std::vector<button*> display;

    view_mode curr_view = WIREFRAME;

    // save button
    display.push_back(new button("assets/button_save.png", {20, 20}, [](){
        std::cout << "save pressed" << std::endl;
    }));
    // load button
    display.push_back(new button("assets/button_load.png", {20, 60}, [](){
        std::cout << "load pressed" << std::endl;
    }));
    // load texture button
    display.push_back(new button("assets/button_texture.png", {20, 100}, [](){
        std::cout << "rexture pressed" << std::endl;
    }));
    // wireframe button
    display.push_back(new button("assets/button_wire.png", {20, WINDOW_Y - 50}, [&curr_view](){
        curr_view = WIREFRAME;
    }));
    // flat button
    display.push_back(new button("assets/button_flat.png", {20, WINDOW_Y - 90}, [&curr_view](){
        curr_view = FLAT;
    }));
    // gouraud
    display.push_back(new button("assets/button_gouraud.png", {20, WINDOW_Y - 130}, [&curr_view](){
        curr_view = GOURAUD;
    }));
    // ka_pluss
    display.push_back(new button("assets/button_plus.png", {WINDOW_X - 50, 20}, [](){
        std::cout << "Ka plus" << std::endl;
    }));
    // ka_minus
    display.push_back(new button("assets/button_minus.png", {WINDOW_X - 90, 20}, [](){
        std::cout << "Ka minus" << std::endl;
    }));
    // smoth terrain button
    display.push_back(new button("assets/button_smoth.png", {WINDOW_X - 50, WINDOW_Y - 50}, [](){
        std::cout << "smoth" << std::endl;
    }));
    // generate terrain button
    button generate("assets/button_generate.png", {WINDOW_X - 50, WINDOW_Y - 90}, [](){
        std::cout << "gen terrain" << std::endl;
    });

    std::vector<button*> select;
    select.push_back(new button("assets/selected.png", {130, WINDOW_Y - 40}, [](){
    }));
    select.push_back(new button("assets/selected.png", {130, WINDOW_Y - 80}, [](){
    }));
    select.push_back(new button("assets/selected.png", {130, WINDOW_Y - 120}, [](){
    }));

    half_mesh terrain;

    uint64_t mx = 20;
    uint64_t my = 20;

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
                    // for(auto face : terrain.face_vector){
                    //     std::cout << "face -> " << "half_edge ->  "<< face << std::endl;
                    //     auto vertexes = terrain.get_face_vertexes(face);

                    //     for(auto vertex : vertexes){
                    //         auto p = terrain.points[vertex];
                    //         std::cout << "\t" << p.x << "," << p.y << "," << p.z << std::endl;
                    //     }
                    // }
                    // // normal da face
                    // for(index_t i = 0; i < (index_t)terrain.face_vector.size(); i++){
                    //     vec3f normal = terrain.get_face_normal(i);
                    //     std::cout << "normal da face: " << i << " "
                    //               << "(" << normal.x << ","
                    //               << normal.y << ","
                    //               << normal.z << ")" << std::endl;
                    // }
                    break;
                }
                break;
            case sf::Event::MouseButtonPressed:
                switch(ev.mouseButton.button){
                case sf::Mouse::Left:
                    for(const button *b : display){
                        b->button_pressed({(float)ev.mouseButton.x, (float)ev.mouseButton.y});
                    }
                    break;
                default:
                    break;
                }
                break;
            default:
                win.clear();
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

        auto visible_faces = filter_normal(terrain, vec3f(0,0,0));

        // std::vector<vec3f> srt_points = apply_transf(terrain, visible_faces, cam.get_proj_mat(curr_view));

        // void draw(terrain, srt_points, visible_faces);

        // ignorem isso, vou mudar
        for(const button *b : display){
            win.draw(*b);
        }
        win.draw(*select[curr_view]);
        win.display();
    }

    return 0;
}
