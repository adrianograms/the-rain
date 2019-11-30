#include "include/button.hpp"
#include "include/noise.hpp"
#include "include/half_mesh.hpp"
#include "include/pipeline.hpp"
#include "include/vec3f.hpp"
#include "include/utils.hpp"
#include "include/shading.hpp"

#include <limits>

#define WIN_Y 768
#define WIN_X 1337

const std::vector<sf::Color> palett = {{ 30,   0, 255},
                                       { 12,  60, 232},
                                       {  0, 142, 255},
                                       { 12, 206, 232},
                                       {  0, 255, 196},
                                       {  0, 255, 110},
                                       { 12, 232,  28},
                                       { 89, 255,   0},
                                       {186, 232,  12},
                                       {255, 236,   0},
                                       {255, 199,   0},
                                       {232, 149,  12},
                                       {255, 108,   0},
                                       {232,  54,  12},
                                       {255,   0,  29}};

sf::RenderWindow win(sf::VideoMode(WIN_X, WIN_Y), "8===========D~~~", sf::Style::Close | sf::Style::Titlebar);

int main(){
    win.setVerticalSyncEnabled(true);

    config    conf;
    noise     height(conf.mx, conf.my);
    light     lpoint;
    Pipeline  camera;

    camera.setMatrixJP(-100.0, 100.0, -100.0, 100.0, 0, WIN_X, 0, WIN_Y);
    camera.setMatrixSRC({0, 30, 30}, {50, 0, 50});

    auto zmap = [&height, &conf](index_t index) -> vec3f {
        uint64_t i = index / conf.mx;
        uint64_t j = index % conf.mx;

        return vec3f((float)i, (float)height[i][j]/(float)conf.hmax, (float)j);
    };

    auto hmax_update = [&conf, &zmap](uint64_t i){
        conf.hmax = i;
        conf.terrain.update_mesh_z(zmap);
    };

    auto terrain_update_size = [&height, &conf, &zmap](uint64_t x, uint64_t y){
        conf.mx = x;
        conf.my = y;
        height.update(conf.mx, conf.my);
        height.gen(std::time(NULL));
        conf.terrain.clear_mesh();
        conf.terrain.build_mesh(conf.mx, conf.my, zmap);
    };

    auto draw_ui = [&conf](){
        conf.draw_interface(win);
        conf.draw_overlay(win);
        win.display();
    };

    auto draw_height_map = [&height, &conf](){
        sf::Texture t;
        sf::Sprite  s;
        sf::Image   b;
        b.create(conf.my, conf.mx);

        for(uint64_t i = 0; i < conf.my; i++){
            for(uint64_t j = 0; j < conf.mx; j++){
                auto c = palett[(uint64_t)((height[i][j] / 256.0) * palett.size())];
                b.setPixel(i, j, c);
            }
        }

        t.loadFromImage(b);
        s.setTexture(t);
        s.setPosition({(float)WIN_X - conf.my, 0});

        win.draw(s);
    };

    conf.load_font("assets/OpenSans-Regular.ttf");

    auto read_input = [&draw_ui](std::string &to) {
        to = "";
        while(win.isOpen()){

            sf::Event e;

            while(win.pollEvent(e)){
                switch(e.type){
                case sf::Event::Closed:
                    win.close();
                    break;
                case sf::Event::TextEntered:
                    if(e.text.unicode < 128){
                        char a = static_cast<char>(e.text.unicode);
                        if(a == 13){
                            return;
                        }
                        if(a == 8 && to.size() != 0){
                            to.pop_back();
                        }
                        else{
                            to += a;
                        }

                    }
                default:
                    break;
                }
            }
            win.clear();
            draw_ui();
        }
    };

    auto read_input_number = [&draw_ui](std::string &to){
        to = "";
        while(win.isOpen()){

            sf::Event e;

            while(win.pollEvent(e)){
                switch(e.type){
                case sf::Event::Closed:
                    win.close();
                    break;
                case sf::Event::TextEntered:
                    if(e.text.unicode < 128){
                        char a = static_cast<char>(e.text.unicode);
                        if(a == 13){
                            return;
                        }
                        if(a == 8 && to.size() != 0){
                            to.pop_back();
                        }
                        else if(a == 46 || (a >= 48 && a <= 57)){
                            to += a;
                        }

                    }
                default:
                    break;
                }
            }
            win.clear();
            draw_ui();
        }
    };

    // buttons
    conf.add_button("assets/button_save.png", {20, 20}, [&read_input, &conf](){
        read_input(conf.file_name);
    });
    conf.add_button("assets/button_texture.png", {20, 60}, [&read_input, &conf](){
        read_input(conf.file_name_texture);
        utils::load_texture(conf);
        if(!conf.texture_set){
            conf.message_log = "Fail to load texture: " + conf.file_name_texture;
            conf.file_name_texture = "";
        }
    });
    conf.add_button("assets/button_load.png", {20, 100}, [&read_input, &conf](){
        read_input(conf.file_name);
        utils::load_mesh(conf);
        if(!conf.file_set){
            conf.message_log = "Fail to load file: " + conf.file_name;
            conf.file_name_texture = "";
        }
    });
    conf.add_button("assets/button_wire.png", {20, WIN_Y - 50}, [&conf](){
        conf.vmode = WIREFRAME;
    });
    conf.add_button("assets/button_flat.png", {20, WIN_Y - 90}, [&conf](){
        conf.vmode = FLAT;
    });
    conf.add_button("assets/button_gouraud.png", {20, WIN_Y - 130}, [&conf](){
        conf.vmode = GOURAUD;
    });
    conf.add_button("assets/button_plus.png", {WIN_X - 265, WIN_Y - 90}, [&terrain_update_size, &conf](){
        terrain_update_size(conf.mx, ++conf.my);
    });
    conf.add_button("assets/button_minus.png", {WIN_X - 300, WIN_Y - 90}, [&terrain_update_size, &conf](){
        terrain_update_size(conf.mx, conf.my == 2 ? 2 : --conf.my);
    });
    conf.add_button("assets/button_plus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
    });
    conf.add_button("assets/button_minus.png", {WIN_X - 300, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(conf.mx == 2 ? 2 : --conf.mx, conf.my);
    });
    conf.add_button("assets/button_smoth.png", {WIN_X - 120, WIN_Y - 50}, [&height, &conf, &zmap](){
        height.smooth();
        conf.terrain.update_mesh_z(zmap);

    });
    conf.add_button("assets/button_minus.png", {WIN_X - 435, WIN_Y - 50}, [&hmax_update, &conf](){
        hmax_update(conf.hmax == 1 ? 1 : --conf.hmax);
    });
    conf.add_button("assets/button_plus.png", {WIN_X - 400, WIN_Y - 50}, [&hmax_update, &conf](){
        hmax_update(conf.hmax == 255 ? 255 : ++conf.hmax);
    });
    conf.add_button("assets/button_generate.png", {WIN_X - 120, WIN_Y - 90}, [&height, &conf, &zmap](){
        height.gen(std::time(NULL));
        conf.terrain.update_mesh_z(zmap);
    });
    conf.add_button("assets/button_x.png", {140, WIN_Y - 130}, [&read_input_number, &lpoint](){
        read_input_number(lpoint.x);
        if(lpoint.x == ""){
            lpoint.x = "0";
        }
    });
    conf.add_button("assets/button_y.png", {140, WIN_Y - 90}, [&read_input_number, &lpoint](){
        read_input_number(lpoint.y);
        if(lpoint.y == ""){
            lpoint.y = "0";
        }
    });
    conf.add_button("assets/button_z.png", {140, WIN_Y - 50}, [&read_input_number, &lpoint](){
        read_input_number(lpoint.z);
        if(lpoint.z == ""){
            lpoint.z = "0";
        }
    });

    conf.add_button("assets/button_plus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
    });
    conf.add_button("assets/button_minus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
    });
    conf.add_button("assets/button_plus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
    });
    conf.add_button("assets/button_minus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
    });
    conf.add_button("assets/button_plus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
    });
    conf.add_button("assets/button_minus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
    });
    conf.add_button("assets/button_plus.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
    });
    conf.add_button("assets/button_minux.png", {WIN_X - 265, WIN_Y - 50}, [&terrain_update_size, &conf](){
        terrain_update_size(++conf.mx, conf.my);
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
        static std::string curr_mode[3] = {"Wireframe", "Flat", "Gouraud"};
        return "-> " + curr_mode[conf.vmode];
    });
    conf.add_overlay({125, 25}, [&conf]() -> std::string {
        return conf.file_name == "" ? "-> No save file" : conf.file_name;
    });
    conf.add_overlay({125, 65}, [&conf]() -> std::string {
        return conf.file_name_texture == "" ? "-> No texture" : conf.file_name_texture;
    });
    conf.add_overlay({175, WIN_Y - 125}, [&lpoint]() -> std::string {
        return lpoint.x;
    });
    conf.add_overlay({175, WIN_Y - 85}, [&lpoint]() -> std::string {
        return lpoint.y;
    });
    conf.add_overlay({175, WIN_Y - 45}, [&lpoint]() -> std::string {
        return lpoint.z;
    });
    conf.add_overlay({140, WIN_Y - 150}, []() -> std::string {
        return "Light position:";
    });
    conf.add_overlay({20, WIN_Y / 2}, [&conf]() -> std::string {
        return conf.message_log;
    });
    conf.add_overlay({400, WIN_Y - 30}, [&lpoint]() -> std::string {
        return "LA: " + std::to_string(lpoint.LA);
    });
    conf.add_overlay({400, WIN_Y - 50}, [&lpoint]() -> std::string {
        return "KA: " + std::to_string(lpoint.KA);
    });
    conf.add_overlay({400, WIN_Y - 70}, [&lpoint]() -> std::string {
        return "KD: " + std::to_string(lpoint.KD);
    });
    conf.add_overlay({400, WIN_Y - 90}, [&lpoint]() -> std::string {
        return "KS: " + std::to_string(lpoint.KS);
    });
    conf.add_overlay({WIN_X - 365, WIN_Y - 45}, [&conf]() -> std::string {
        return std::to_string(conf.hmax);
    });

    conf.terrain.build_mesh(conf.mx, conf.my, zmap);
    while(win.isOpen()){

        sf::Event event;

        while(win.pollEvent(event)){
            switch(event.type){
            case sf::Event::Closed:
                win.close();
                break;
            case sf::Event::KeyPressed:
                switch(event.key.code){
                case sf::Keyboard::A:
                    // pan esquerda
                    camera.FOCAL_VRP_LEFT();
                    break;
                case sf::Keyboard::S:
                    // pan direita
                    camera.FOCAL_VRP_RIGHT();
                    break;
                case sf::Keyboard::Q:
                    // pan cima
                    camera.FOCAL_VRP_UP();
                    break;
                case sf::Keyboard::W:
                    // pan baixo
                    camera.FOCAL_VRP_DOWN();
                    break;
                case sf::Keyboard::Z:
                    // aumenta o fov
                    break;
                case sf::Keyboard::X:
                    // diminui o fov
                    break;
                case sf::Keyboard::D:
                    // ponto focal esquerda
                    camera.FOCAL_LEFT();
                    break;
                case sf::Keyboard::F:
                    // ponto focal direita
                    camera.FOCAL_RIGHT();
                    break;
                case sf::Keyboard::E:
                    // ponto focal para baixo
                    camera.FOCAL_DOWN();
                    break;
                case sf::Keyboard::R:
                    // ponto focal para cima
                    camera.FOCAL_UP();
                    break;
                case sf::Keyboard::G:
                    // camera para a esquerda
                    camera.VRP_LEFT();
                    break;
                case sf::Keyboard::H:
                    // camera para a direita
                    camera.VRP_RIGHT();
                    break;
                case sf::Keyboard::T:
                    // camera para baixo
                    camera.VRP_DOWN();
                    break;
                case sf::Keyboard::Y:
                    // camera para cima
                    camera.VRP_UP();
                    break;
                default:
                    break;
                }
                break;
            case sf::Event::MouseButtonPressed:
                switch(event.mouseButton.button){
                case sf::Mouse::Left:
                    conf.click_button({(float)event.mouseButton.x, (float)event.mouseButton.y});
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }

        auto visible_faces = filter_normal(conf.terrain, camera.getn());
        auto src_points = apply_pipeline(camera.getMatrix(), conf.terrain.points);

        win.clear();

        {
            // isso é so pra desenhar a image na tela
            sf::Texture t;
            sf::Sprite  s;
            sf::Image   im;

            lpoint.pos = vec3f(50, 50, -50);

            std::pair<double, sf::Color> **zbuff;

            zbuff = new std::pair<double, sf::Color>*[WIN_X];

            for(uint64_t i = 0; i < WIN_Y; i++){
                zbuff[i] = new std::pair<double, sf::Color>[WIN_X];
            }

            for(uint64_t i = 0; i < WIN_Y; i++){
                for(uint64_t j = 0; j < WIN_X; j++){
                    zbuff[i][j].first = std::numeric_limits<double>::max();
                    zbuff[i][j].second = sf::Color::Black;
                }
            }

            for(auto f : visible_faces){
                flat_shading_face(zbuff, f, conf, lpoint, camera.getvrp());
            }

            im.create(WIN_X, WIN_Y);
            for(uint64_t i = 0; i < WIN_Y; i++){
                for(uint64_t j = 0; j < WIN_X; j++){
                    im.setPixel(i, j, zbuff[j][i].second);
                }
            }


            t.loadFromImage(im);
            s.setTexture(t);
            win.draw(s);

            for(uint64_t i = 0; i < WIN_Y; i++){
                delete [] zbuff[i];
            }
            delete [] zbuff;
        }

        // for(index_t i : terrain.edge_vector){
        //     sf::VertexArray lines(sf::Lines, 2);
        //     std::pair<index_t, index_t> dir = terrain.half_direction(i);

        //     auto srt = src_points[dir.first];
        //     auto end = src_points[dir.second];

        //     lines[0].position.x = srt.x;
        //     lines[0].position.y = srt.y;
        //     lines[0].color = sf::Color::Green;

        //     lines[1].position.x = end.x;
        //     lines[1].position.y = end.y;
        //     lines[1].color = sf::Color::Blue;

        //     win.draw(lines);
        // }

        if(conf.texture_set){
            sf::Texture t;
            sf::Sprite  s;

            t.loadFromImage(conf.texture);
            s.setTexture(t);

            s.setPosition({500, 500});
            win.draw(s);
        }

        draw_height_map();
        draw_ui();
    }

    return 0;
}
