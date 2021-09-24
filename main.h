#ifndef MAIN_H
#define MAIN_H

#include <SFML/Graphics.hpp>

#include "engine.h"

void input();
void update();
void draw();
void handle_key_down( sf::Keyboard::Key key );

#endif
