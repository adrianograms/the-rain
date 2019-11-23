#ifndef BUTTON_H
#define BUTTON_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>

class button : public sf::Sprite {
    std::function<void(void)> callback;

    sf::Texture t;
public:

    button(std::string filename, sf::Vector2f at, std::function<void(void)> c): sf::Sprite(),
                                                                                callback(c){

        this->t.loadFromFile(filename);
        this->setTexture(this->t);
        setPosition(at);
    }

    void button_pressed(sf::Vector2f mouse) const {
        if (getGlobalBounds().contains(mouse)){
            callback();
        }
    }
};

#endif
