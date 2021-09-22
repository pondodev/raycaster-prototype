#include "main.h"

int main() {
    Engine engine( "map.txt", "walltext.png", "monsters.png" );
    engine.draw_to_ppm( "framebuffer.ppm" );

    return 0;
}