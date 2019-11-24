#include "include/overlay.hpp"

overlay::overlay(const sf::Font &font, sf::Vector2f at, std::function<std::string(void)> callb): callb(callb) {
    setFont(font);
    setFillColor(sf::Color::White);
    setOutlineColor(sf::Color::Black);
    setCharacterSize(16);
    setStyle(sf::Text::Regular);
    setPosition(at);
}

void overlay::update_text(){
    setString(callb());
}
