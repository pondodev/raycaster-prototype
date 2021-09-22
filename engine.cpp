#include "engine.h"

Engine::Engine( std::string map_path, std::string wall_tex_path, std::string enemy_tex_path )
    : wall_textures( wall_tex_path ), enemy_textures( enemy_tex_path ) {
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
                map.push_back( Wall1 );
                break;

            case '$':
                map.push_back( Wall2 );
                break;

            case '%':
                map.push_back( Wall3 );
                break;

            default:
                map.push_back( Floor );
                std::cerr << "unrecognised tile: " << tile << std::endl;
                break;
        }
    }

    f.close();

    player = Player {
        Vec2 { 2.0, 7.0 },
        1.0,
        M_PI / 3.0
    };

    enemies = {
        Enemy { Vec2 { 2.5, 9.0 }, 0 },
        Enemy { Vec2 { 5.0, 10.0 }, 0 },
        Enemy { Vec2 { 5.5, 9.0 }, 0 }
    };

    // draw map
    int rect_w, rect_h;
    rect_w = WINDOW_WIDTH / (map_width * 2);
    rect_h = WINDOW_HEIGHT / map_height;
    for ( int y = 0; y < map_height; y++ ) {
        for ( int x = 0; x < map_width; x++ ) {
            Color col;
            auto tile = map[ x + y * map_width ];
            switch ( tile ) {
                case Floor:
                    col = Color( 0xBBBBBBFF );
                    break;

                case Wall1:
                case Wall2:
                case Wall3:
                    // get top left corner of texture
                    col = wall_textures.get_column( wall_textures.get_size(), tile, 0 )[ 0 ];
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

    // clear the 3d view
    draw_rect( WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT, Color( 0xBBBBBBFF ) );

    // draw view cone and 3d view
    std::vector<float> depth_buffer( WINDOW_WIDTH / 2, 1e3 );
    for ( float i = 0; i < WINDOW_WIDTH / 2; i++ ) {
        float angle = player.view_angle - player.fov / 2 + player.fov * i / float(WINDOW_WIDTH / 2);
        for ( float ray_dist = 0; ray_dist < 20; ray_dist += .01 ) {
            float cx = player.position.x + ray_dist * cos( angle );
            float cy = player.position.y + ray_dist * sin( angle );

            // the 3d magic!
            auto tile = map[ int(cx) + int(cy) * map_width ];
            if ( tile != Floor ) {
                int column_height = WINDOW_HEIGHT / (ray_dist * cos(angle - player.view_angle));
                float hit_x = cx - floor( cx + .5 );
                float hit_y = cy - floor( cy + .5 );
                int x_texcoord = hit_x * wall_textures.get_size();
                if ( std::abs( hit_y ) > std::abs( hit_x ) ) { // check if vertical or horizontal wall
                    x_texcoord = hit_y * wall_textures.get_size();
                }

                if ( x_texcoord < 0 ) x_texcoord += wall_textures.get_size();

                depth_buffer[ i ] = ray_dist * cos( angle - player.view_angle );
                auto column = wall_textures.get_column( column_height, tile, x_texcoord );
                int pixel_x = WINDOW_WIDTH / 2 + i;
                for ( int j = 0; j < column_height; j++ ) {
                    int pixel_y = j + WINDOW_HEIGHT / 2 - column_height / 2;
                    if ( pixel_y < 0 || pixel_y >= WINDOW_HEIGHT ) continue;
                    framebuffer[ pixel_x + pixel_y * WINDOW_WIDTH ] = column[ j ];
                }

                break;
            }

            int pixel_x, pixel_y;
            pixel_x = cx * rect_w;
            pixel_y = cy * rect_h;
            framebuffer[ pixel_x + pixel_y * WINDOW_WIDTH ] = Color( 0x5555DDFF );
        }
    }

    // draw the enemies
    for ( auto e : enemies ) {
        draw_rect( e.position.x * rect_w, e.position.y * rect_h, 5, 5, Color( 0xFF0000FF ) );
        draw_sprite( e, depth_buffer );
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

void Engine::draw_sprite( Enemy enemy, std::vector<float> depth_buffer ) {
    float dir = atan2( enemy.position.y - player.position.y, enemy.position.x - player.position.x );
    while ( dir - player.view_angle > M_PI ) dir -= 2 * M_PI;
    while ( dir - player.view_angle < -M_PI ) dir += 2 * M_PI;

    float dist = sqrt( pow( player.position.x - enemy.position.x, 2 ) ) + pow( player.position.y - enemy.position.y, 2 );
    size_t size = std::min( 2000, static_cast<int>( WINDOW_HEIGHT / dist ) );
    int h_offset = (dir - player.view_angle) * (WINDOW_WIDTH / 2) / player.fov + (WINDOW_WIDTH / 2) / 2 - size / 2;
    int v_offset = WINDOW_HEIGHT / 2 - size / 2;

    for ( int i = 0; i < size; i++ ) {
        if ( h_offset + i < 0 || h_offset + i >= WINDOW_WIDTH / 2 ) continue;
        if ( depth_buffer[ h_offset + i ] < dist ) continue; // occlude sprite
        for ( int j = 0; j < size; j++ ) {
            if ( v_offset + j < 0 || v_offset + j >= WINDOW_HEIGHT ) continue;
            int x, y;
            x = WINDOW_WIDTH / 2 + h_offset + i;
            y = v_offset + j;
            framebuffer[ x + y * WINDOW_WIDTH ] = Color( 0x000000FF );
        }
    }
}
