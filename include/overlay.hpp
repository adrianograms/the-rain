#ifndef OVERLAY_H
#define OVERLAY_H


#include <functional>
#include <SFML/Graphics.hpp>

class overlay : public sf::Text {

    std::function<std::string(void)> callb;

public:

    overlay(const sf::Font &, sf::Vector2f, std::function<std::string(void)>);

    void update_text();

};

#endif
