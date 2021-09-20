#include "main.h"

int main() {
    Engine engine( "map.txt" );
    engine.draw_to_ppm( "framebuffer.ppm" );

    return 0;
}