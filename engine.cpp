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

    player = Player {
        Vec2 { 2.0, 7.0 },
        1.0,
        M_PI / 3.0
    };

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
                float hit_x = cx - floor( cx + .5 );
                float hit_y = cy - floor( cy + .5 );
                int x_texcoord = hit_x * wall_tex_size;
                if ( abs( hit_y ) > std::abs( hit_x ) ) { // check if vertical or horizontal wall
                    x_texcoord = hit_y * wall_tex_size;
                }

                if ( x_texcoord < 0 ) x_texcoord += wall_tex_size;

                auto column = get_tex_column( column_height, tile, x_texcoord );
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

std::vector<Color> Engine::get_tex_column( size_t col_height, int tex_index, int tex_x ) {
    size_t img_w, img_h;
    img_w = wall_tex_size * wall_tex_count;
    img_h = wall_tex_size;
    std::vector<Color> column( col_height );
    for ( int y = 0; y < col_height; y++ ) {
        size_t pixel_x, pixel_y;
        pixel_x = tex_index * wall_tex_size + tex_x;
        pixel_y = (y * wall_tex_size) / col_height;
        column[ y ] = wall_textures[ pixel_x + pixel_y * img_w ];
    }

    return column;
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
