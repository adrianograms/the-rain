#include "include/utils.hpp"

config::config(): mx(10),
                  my(10),
                  curr_view(WIREFRAME),
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
