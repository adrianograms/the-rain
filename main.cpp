#include <iostream>
#include <ctime>

#include <SFML/Graphics.hpp>

#include "include/button.hpp"
#include "include/noise.hpp"
#include "include/half_mesh.hpp"
#include "include/pipeline.hpp"
#include "include/vec3f.hpp"
#include "include/utils.hpp"

#define WIN_Y 768
#define WIN_X 1337

#define DOLOG
#ifdef DOLOG
#define LOG(s) do{                                                  \
        auto t = time(NULL);                                        \
        std::cout << std::ctime(&t) << "::" << (s) << std::endl;    \
            }while(false);
#else
#define LOG(s)
#endif

// TODO list:
// menus
// save to file
// load from file
// camera
// pipeline

sf::RenderWindow win(sf::VideoMode(WIN_X, WIN_Y), ":(", sf::Style::Close | sf::Style::Titlebar);

int main(){
    win.setVerticalSyncEnabled(true);

    config    conf;
    noise     height(conf.my, conf.mx);
    half_mesh terrain;

    auto zmap = [&height, &conf](index_t index) -> vec3f {
            uint64_t i = index / conf.mx;
            uint64_t j = index % conf.mx;

            return vec3f((float)i, (float)j, (float)height[i][j]);
    };

    auto terrain_update_size = [&terrain, &height, &conf, &zmap](uint64_t x, uint64_t y){
        conf.mx = x;
        conf.my = y;
        height.update(conf.mx, conf.my);
        height.gen(std::time(NULL));
        terrain.clear_mesh();
        terrain.build_mesh(conf.mx, conf.my, zmap);
    };

    conf.load_font("assets/OpenSans-Regular.ttf");

    // buttons
    conf.add_button("assets/button_save.png", {20, 20}, [](){
        LOG("Save pressed");
    });
    conf.add_button("assets/button_load.png", {20, 60}, [](){
        LOG("Load pressed");
    });
    conf.add_button("assets/button_texture.png", {20, 100}, [](){
        LOG("Load texture pressed");
    });
    conf.add_button("assets/button_wire.png", {20, WIN_Y - 50}, [&conf](){
        conf.vmode = WIREFRAME;
        LOG("Wireframe mode");
    });
    conf.add_button("assets/button_flat.png", {20, WIN_Y - 90}, [&conf](){
        conf.vmode = FLAT;
        LOG("Flat shading mode");
    });
    conf.add_button("assets/button_gouraud.png", {20, WIN_Y - 130}, [&conf](){
        conf.vmode = GOURAUD;
        LOG("Gouraud shading mode");
    });
    conf.add_button("assets/button_plus.png", {WIN_X - 265, WIN_Y - 90}, [&terrain_update_size, &conf](){
        terrain_update_size(conf.mx, ++conf.my);
        LOG("Terrain mx++")
    });
    conf.add_button("assets/button_minus.png", {WIN_X - 300, WIN_Y - 90}, [&terrain_update_size, &conf](){
        terrain_update_size(conf.mx, conf.my == 2 ? 2 : --conf.my);
        LOG("Terrain mx--")
    });
    conf.add_button("assets/button_plus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
        LOG("Terrain my++")
    });
    conf.add_button("assets/button_minus.png", {WIN_X - 300, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(conf.mx == 2 ? 2 : --conf.mx, conf.my);
        LOG("Terrain my--")
    });
    conf.add_button("assets/button_smoth.png", {WIN_X - 120, WIN_Y - 50}, [&height](){
        height.smooth();
        LOG("Height smoth");
    });
    conf.add_button("assets/button_generate.png", {WIN_X - 120, WIN_Y - 90}, [&height](){
        height.gen(std::time(NULL));
        LOG("Height genenete");
    });
    // overlays
    conf.add_overlay({WIN_X - 230, WIN_Y - 85}, [&conf]() -> std::string {
        return "X: " + std::to_string(conf.my);
    });
    conf.add_overlay({WIN_X - 230, WIN_Y - 45}, [&conf]() -> std::string {
        return "Y: " + std::to_string(conf.mx);
    });
    conf.add_overlay({WIN_X - 300, WIN_Y - 110}, []() -> std::string {
        return "Terrain size:";
    });
    conf.add_overlay({WIN_X - 120, WIN_Y - 110}, []() -> std::string {
        return "Height map:";
    });
    conf.add_overlay({20, WIN_Y - 150}, [&conf]() -> std::string {
        static std::string curr_mode[3] = {"Wireframe", "Flat shade", "Gouraud shade"};
        return "> " + curr_mode[conf.vmode];
    });
    conf.add_overlay({125, 25}, [&conf]() -> std::string {
        return conf.file_name == "" ? "> No save file" : conf.file_name;
    });
    conf.add_overlay({125, 105}, [&conf]() -> std::string {
        return conf.file_name_texture == "" ? "> No texture" : conf.file_name_texture;
    });


    terrain.build_mesh(conf.mx, conf.my, zmap);
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
                    conf.click_button({(float)ev.mouseButton.x, (float)ev.mouseButton.y});
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        auto visible_faces = filter_normal(terrain, vec3f(0,0,0));

        // std::vector<vec3f> srt_points = apply_transf(terrain, visible_faces, cam.get_proj_mat(curr_view));

        // void draw(terrain, srt_points, visible_faces);

        // ignorem isso, vou mudar
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

        conf.draw_interface(win);
        conf.draw_overlay(win);
        win.display();
    }

    return 0;
}
