#include "engine.h"

Engine::Engine( std::string map_path ) {
    // temp coord space drawing
    for ( int y = 0; y < WINDOW_HEIGHT; y++ ) {
        for ( int x = 0; x < WINDOW_WIDTH; x++ ) {
            uint8_t r, g, b, a;
            r = 255 * x / float(WINDOW_WIDTH);
            g = 255 * y / float(WINDOW_HEIGHT);
            b = 0;
            a = 0;

            framebuffer[ x + y * WINDOW_WIDTH ] = Color( r, g, b, a );
        }
    }

    // read in map
    std::fstream f;
    f.open( map_path, std::ios::in );
    std::string width_str, height_str;
    std::getline( f, width_str );
    std::getline( f, height_str );
    map_width = std::stoi( width_str );
    map_height = std::stoi( height_str );
    char tile;
    while ( f >> std::noskipws >> tile ) {
        switch ( tile ) {
            case '\n': // ignore
                break;

            case '_':
                map.push_back( Floor );
                break;

            case '#':
                map.push_back( Wall );
                break;

            default:
                std::cerr << "unrecognised tile: " << tile << std::endl;
                break;
        }
    }

    f.close();

    // draw map
    int rect_w, rect_h;
    rect_w = WINDOW_WIDTH / map_width;
    rect_h = WINDOW_HEIGHT / map_height;
    for ( int y = 0; y < map_height; y++ ) {
        for ( int x = 0; x < map_width; x++ ) {
            if ( map[ x + y * map_width ] == Floor ) continue;

            int rect_x, rect_y;
            rect_x = x * rect_w;
            rect_y = y * rect_h;
            draw_rect( rect_x, rect_y, rect_w, rect_h, Color( 0x000000FF ) );
        }
    }

    // draw player on map
    player.position = Vec2 { 2.0, 7.0 };
    draw_rect( player.position.x * rect_w, player.position.y * rect_h, 5, 5, Color( 0xFFFFFFFF ) );
}

void Engine::draw_to_ppm( std::string path ) {
    std::fstream f;
    f.open( path, std::ios::out );

    // header information
    f << "P3\n" << WINDOW_WIDTH << " " << WINDOW_HEIGHT << "\n255\n";

    for ( int i = 0; i < FRAMEBUFFER_LENGTH; i++ ) {
        auto c = framebuffer[ i ];
        uint8_t r, g, b, a;
        c.get_components( r, g, b, a );

        f << unsigned(r) << " " << unsigned(g) << " " << unsigned(b) << "\n";
    }

    f.close();
}

void Engine::draw_rect( int x, int y, int w, int h, Color color ) {
    for ( int i = 0; i < w; i++ ) {
        for ( int j = 0; j < h; j++ ) {
            int cx = x + i;
            int cy = y + j;
            framebuffer[ cx + cy * WINDOW_WIDTH ] = color;
        }
    }
}
