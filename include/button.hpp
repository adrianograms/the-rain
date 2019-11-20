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

        t.loadFromFile(filename);
        setTexture(t);
        setPosition(at);
    }

    void button_pressed(sf::Vector2f mouse){
        if (getGlobalBounds().contains(mouse)){
            callback();
        }
    }
};

#endif
