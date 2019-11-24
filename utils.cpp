#include "include/utils.hpp"

config::config(): mx(10),
                  my(10),
                  vmode(WIREFRAME),
                  file_name(""),
                  file_name_texture("") { }

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
