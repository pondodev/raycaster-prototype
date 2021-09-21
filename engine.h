#ifndef ENGINE_H
#define ENGINE_H

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define FRAMEBUFFER_LENGTH WINDOW_WIDTH * WINDOW_HEIGHT

#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <cmath>

#include "color.h"
#include "player.h"

enum MapTile {
    Floor,
    Wall
};

class Engine {
public:
    Engine( std::string map_path );
    void draw_to_ppm( std::string path );

private:
    Color framebuffer[ FRAMEBUFFER_LENGTH ];
    std::vector<MapTile> map;
    unsigned int map_width;
    unsigned int map_height;
    Player player;

    void draw_rect( int x, int y, int w, int h, Color color );
};

#endif
