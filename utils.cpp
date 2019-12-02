#include "include/utils.hpp"

config::config(): mx(100),
                  my(100),
                  hmax(0.3),
                  vmode(WIREFRAME),
                  file_name(""),
                  file_name_texture(""),
                  file_set(false),
                  texture_set(false) { }

void config::add_button(std::string asset_name, sf::Vector2f at, std::function<void(void)> callb){
    interface.push_back(new button(asset_name, at, callb));
}

void config::click_button(sf::Vector2f at) const{
    for(const auto *b : interface){
        if(b->button_pressed(at)){
            return;
        }
    }
}

void config::draw_interface(sf::RenderTarget &win) const {
    for(const auto *b : interface){
        win.draw(*b);
    }
}

void config::load_font(std::string font_name){
    font.loadFromFile(font_name);
}

void config::add_overlay(sf::Vector2f at, std::function<std::string(void)> callb){
    overlays.push_back(new overlay(font, at, callb));
}

void config::draw_overlay(sf::RenderTarget &win) const {
    for(const auto t : overlays){
        t->update_text();
        win.draw(*t);
    }
}

void utils::save_mesh(config &conf, noise &heigth){
    std::FILE *f = std::fopen(conf.file_name.data(), "w");
    if(f == nullptr){
        conf.file_set = false;
        return;
    }

    std::size_t size = conf.terrain.half_vector.size();
    std::fwrite(&size, sizeof(size), 1, f);
    std::fwrite(conf.terrain.half_vector.data(), sizeof(half_edge), size, f);

    size = conf.terrain.vertex_vector.size();
    std::fwrite(&size, sizeof(size), 1, f);
    std::fwrite(conf.terrain.vertex_vector.data(), sizeof(index_t), size, f);

    size = conf.terrain.face_vector.size();
    std::fwrite(&size, sizeof(size), 1, f);
    std::fwrite(conf.terrain.face_vector.data(), sizeof(index_t), size, f);

    size = conf.terrain.edge_vector.size();
    std::fwrite(&size, sizeof(size), 1, f);
    std::fwrite(conf.terrain.edge_vector.data(), sizeof(index_t), size, f);

    // TODO: write the points as vec3f;

    size = conf.terrain.edge_index_map.size();
    std::fwrite(&size, sizeof(size), 1, f);
    for(auto &t : conf.terrain.edge_index_map){
        index_t i;
        index_t j;
        index_t k;

        i = t.first.first;
        j = t.first.second;
        k = t.second;
    }

    auto mapx = conf.my;
    auto mapy = conf.mx;

    std::fwrite(&mapx, sizeof(mapx), 1, f);
    std::fwrite(&mapy, sizeof(mapy), 1, f);

    for(uint64_t i = 0; i < conf.my; i++){
        for(uint64_t j = 0; j < conf.mx; j++){
            uint8_t n = heigth[i][j];
            std::fwrite(&n, sizeof(uint8_t), 1, f);
        }
    }


    std::fclose(f);
    conf.file_set = true;
}

void utils::load_mesh(config &conf, noise &height){
    std::FILE *f = std::fopen(conf.file_name.data(), "r");
    if(f == nullptr){
        conf.file_set = false;
        return;
    }
    conf.terrain.clear_mesh();

    std::size_t size;
    std::fread(&size, sizeof(std::size_t), 1, f);
    conf.terrain.half_vector.reserve(size);
    std::fread(conf.terrain.half_vector.data(), sizeof(half_edge), size, f);

    std::fread(&size, sizeof(std::size_t), 1, f);
    conf.terrain.vertex_vector.reserve(size);
    std::fread(conf.terrain.vertex_vector.data(), sizeof(index_t), size, f);

    std::fread(&size, sizeof(std::size_t), 1, f);
    conf.terrain.face_vector.reserve(size);
    std::fread(conf.terrain.face_vector.data(), sizeof(index_t), size, f);

    std::fread(&size, sizeof(std::size_t), 1, f);
    conf.terrain.edge_vector.reserve(size);
    std::fread(conf.terrain.edge_vector.data(), sizeof(index_t), size, f);

    // TODO: read the points as vec3f;

    std::fread(&size, sizeof(std::size_t), 1, f);
    for(uint64_t o = 0; o < size; o++){
        index_t i;
        index_t j;
        index_t k;

        std::fread(&i, sizeof(index_t), 1, f);
        std::fread(&j, sizeof(index_t), 1, f);
        std::fread(&k, sizeof(index_t), 1, f);

        conf.terrain.edge_index_map[std::make_pair(i, j)] = k;
    }

    uint64_t mapx;
    uint64_t mapy;

    std::fread(&mapx, sizeof(mapx), 1, f);
    std::fread(&mapy, sizeof(mapy), 1, f);

    // height.load();

    conf.mx = mapx;
    conf.my = mapy;

    std::fclose(f);
    conf.file_set = true;
}

void utils::load_texture(config &conf){
    conf.texture_set = conf.texture.loadFromFile(conf.file_name_texture);
}
