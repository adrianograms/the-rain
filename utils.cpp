#include "include/utils.hpp"

config::config(): mx(100),
                  my(100),
                  hmax(32),
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

void utils::save_mesh(config &conf){
    // TODO: finish
}

void utils::load_mesh(config &conf){
    std::FILE *f = std::fopen(conf.file_name.data(), "r");
    if(f == nullptr){
        conf.file_set = false;
        return;
    }
    // TODO: fisnish
    std::fclose(f);
    conf.file_set = true;
}

void utils::load_texture(config &conf){
    conf.texture_set = conf.texture.loadFromFile(conf.file_name_texture);
}
