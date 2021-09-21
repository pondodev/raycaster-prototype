#include "engine.h"

Engine::Engine( std::string map_path ) {
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
            Color col;
            switch ( map[ x + y * map_width ] ) {
                case Floor:
                    col = Color( 0xBBBBBBFF );
                    break;

                case Wall:
                    col = Color( 0x222222FF );
                    break;

                default:
                    col = Color( 0x00FFFFFF );
                    break;
            }

            int rect_x, rect_y;
            rect_x = x * rect_w;
            rect_y = y * rect_h;
            draw_rect( rect_x, rect_y, rect_w, rect_h, col );
        }
    }

    // draw player on map
    player = Player {
        Vec2 { 2.0, 7.0 },
        1.0,
        M_PI / 3.0
    };
    draw_rect( player.position.x * rect_w, player.position.y * rect_h, 5, 5, Color( 0xFFFFFFFF ) );

    // draw view cone
    for ( float i = 0; i < WINDOW_WIDTH; i++ ) {
        float angle = player.view_angle - player.fov / 2 + player.fov * i / float(WINDOW_WIDTH);
        for ( float t = 0; t < 20; t += .05 ) {
            float cx = player.position.x + t * cos( angle );
            float cy = player.position.y + t * sin( angle );
            if ( map[ int(cx) + int(cy) * map_width ] == Wall ) break;

            int pixel_x, pixel_y;
            pixel_x = cx * rect_w;
            pixel_y = cy * rect_h;
            framebuffer[ pixel_x + pixel_y * WINDOW_WIDTH ] = Color( 0xDD4444FF );
        }
    }
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
