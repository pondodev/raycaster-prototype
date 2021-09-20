#ifndef ENGINE_H
#define ENGINE_H

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define FRAMEBUFFER_LENGTH WINDOW_WIDTH * WINDOW_HEIGHT

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

#include "color.h"

class Engine {
public:
    Engine();
    void draw_to_ppm( std::string path );

private:
    Color framebuffer[ FRAMEBUFFER_LENGTH ];
};

#endif
