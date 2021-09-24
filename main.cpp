#include "main.h"

int main() {
    Engine engine( "map.txt", "walltext.png", "monsters.png" );

    sf::RenderWindow window(
        sf::VideoMode( WINDOW_WIDTH, WINDOW_HEIGHT ),
        "raycaster prototype" );

    sf::Texture render_texture;
    render_texture.create( WINDOW_WIDTH, WINDOW_HEIGHT );
    sf::Sprite s( render_texture );
    sf::Clock delta_clock;

    while ( window.isOpen() ) {
        auto delta_time = delta_clock.restart().asSeconds();

        // input
        sf::Event event;
        while ( window.pollEvent( event ) ) {
            if ( event.type == sf::Event::Closed ) window.close();
        }

        // update
        engine.update( delta_time );

        // draw
        window.clear();

        engine.render();

        uint8_t render_buffer[ FRAMEBUFFER_LENGTH * 4 ];
        engine.get_framebuffer( render_buffer );
        render_texture.update( render_buffer );
        window.draw( s );

        window.display();
    }

    return 0;
}