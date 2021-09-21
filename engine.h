#ifndef ENGINE_H
#define ENGINE_H

#define WINDOW_WIDTH 1024
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
    Floor = -1,
    Wall1 = 0,
    Wall2 = 1,
    Wall3 = 2,
    Wall4 = 3
};

class Engine {
public:
    Engine( std::string map_path, std::string wall_tex_path );
    void draw_to_ppm( std::string path );

private:
    Color framebuffer[ FRAMEBUFFER_LENGTH ];
    std::vector<MapTile> map;
    unsigned int map_width;
    unsigned int map_height;
    Player player;
    std::vector<Color> wall_textures;
    size_t wall_tex_size;
    size_t wall_tex_count;

    void draw_rect( int x, int y, int w, int h, Color color );
    bool load_image( std::string tex_file_path );
};

#endif
