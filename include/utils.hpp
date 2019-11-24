#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <functional>
#include <SFML/Graphics.hpp>

#include "half_types.hpp"
#include "button.hpp"
#include "overlay.hpp"

struct config{
    uint64_t mx;
    uint64_t my;

    view_mode vmode;

    std::string file_name;
    std::string file_name_texture;

    std::vector<button*> interface;
    std::vector<overlay*> overlays;

    sf::Font font;

    config();

    void add_button(std::string, sf::Vector2f, std::function<void(void)>);
    void click_button(sf::Vector2f) const;
    void draw_interface(sf::RenderTarget &) const;

    void load_font(std::string);
    void add_overlay(sf::Vector2f, std::function<std::string(void)>);
    void draw_overlay(sf::RenderTarget &) const;

};

#endif