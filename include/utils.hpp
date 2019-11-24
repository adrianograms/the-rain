#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>

#include "half_types.hpp"
#include "button.hpp"

struct config{
    uint64_t mx;
    uint64_t my;

    view_mode curr_view;

    std::string file_name;
    std::string file_name_texture;

    std::vector<button*> interface;

    config();

    void add_button(std::string, sf::Vector2f, std::function<void(void)>);
    void click_button(sf::Vector2f) const;
    void draw_interface(sf::RenderTarget &) const;

};



#endif
