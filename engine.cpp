#include "engine.h"

Engine::Engine() {
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
}

void Engine::draw_to_ppm( std::string path ) {
    std::fstream f;
    f.open( path, std::ios::out );

    f << "P3\n" << WINDOW_WIDTH << " " << WINDOW_HEIGHT << "\n255\n";
    for ( int i = 0; i < FRAMEBUFFER_LENGTH; i++ ) {
        auto c = framebuffer[ i ];
        uint8_t r, g, b, a;
        c.get_components( r, g, b, a );

        f << unsigned(r) << " " << unsigned(g) << " " << unsigned(b) << "\n";
    }

    f.close();
}
