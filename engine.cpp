#include "engine.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Engine::Engine( std::string map_path, std::string wall_tex_path ) {
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

    if ( !load_image( wall_tex_path ) ) {
        throw "zoinks scoobs!";
    }

    // draw map
    int rect_w, rect_h;
    rect_w = WINDOW_WIDTH / (map_width * 2);
    rect_h = WINDOW_HEIGHT / map_height;
    for ( int y = 0; y < map_height; y++ ) {
        for ( int x = 0; x < map_width; x++ ) {
            Color col;
            switch ( map[ x + y * map_width ] ) {
                case Floor:
                    col = Color( 0xBBBBBBFF );
                    break;

                case Wall1:
                case Wall2:
                case Wall3:
                    col = wall_textures[ wall_tex_size * map[ x + y * map_width ] ];
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

    // clear the 3d view
    draw_rect( WINDOW_WIDTH / 2, 0, WINDOW_WIDTH / 2, WINDOW_HEIGHT, Color( 0xBBBBBBFF ) );

    // draw view cone and 3d view
    for ( float i = 0; i < WINDOW_WIDTH / 2; i++ ) {
        float angle = player.view_angle - player.fov / 2 + player.fov * i / float(WINDOW_WIDTH / 2);
        for ( float ray_dist = 0; ray_dist < 20; ray_dist += .01 ) {
            float cx = player.position.x + ray_dist * cos( angle );
            float cy = player.position.y + ray_dist * sin( angle );

            // the 3d magic!
            auto tile = map[ int(cx) + int(cy) * map_width ];
            if ( tile != Floor ) {
                auto color = wall_textures[ wall_tex_size * tile ];
                int column_height = WINDOW_HEIGHT / (ray_dist * cos(angle - player.view_angle));
                int col_x, col_y;
                col_x = WINDOW_WIDTH / 2 + i;
                col_y = WINDOW_HEIGHT / 2 - column_height / 2;
                draw_rect( col_x, col_y, 1, column_height, color );
                break;
            }

            int pixel_x, pixel_y;
            pixel_x = cx * rect_w;
            pixel_y = cy * rect_h;
            framebuffer[ pixel_x + pixel_y * WINDOW_WIDTH ] = Color( 0xDD5555FF );
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

bool Engine::load_image( std::string tex_file_path ) {
    int nchannels = -1, w, h;
    unsigned char* pixmap = stbi_load( tex_file_path.c_str(), &w, &h, &nchannels, 0 );
    if ( !pixmap ) {
        std::cerr << "failed to load texture" << std::endl;
        return false;
    }

    if ( nchannels != 4 ) {
        std::cerr << "texture must be a 32 bit image" << std::endl;
        stbi_image_free( pixmap );
        return false;
    }

    wall_tex_count = w / h;
    wall_tex_size = w / wall_tex_count;
    if ( w != h * int(wall_tex_count) ) {
        std::cerr << "the texture file must contain N square textures packed horizontally" << std::endl;
        return false;
    }

    wall_textures = std::vector<Color>( w * h );
    for ( int i = 0; i < wall_textures.size(); i++ ) {
        int root_index = i * 4;
        int r, g, b, a;
        r = pixmap[ root_index ];
        g = pixmap[ root_index + 1 ];
        b = pixmap[ root_index + 2 ];
        a = pixmap[ root_index + 3 ];

        wall_textures[ i ] = Color( r, g, b, a );
    }

    stbi_image_free( pixmap );
    return true;
}
