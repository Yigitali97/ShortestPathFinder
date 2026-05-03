#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include "GraphTypes.h"
#include "SplashScreen.h"
#include "LoginScreen.h"
#include "GraphCanvas.h"

static sf::Font g_font;

static bool loadFont() {
    const std::vector<std::string> paths = {
#if defined(_WIN32)
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
#else
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
#endif
        "assets/font.ttf"
    };
    for (const auto& p : paths)
        if (g_font.openFromFile(p)) return true;
    return false;
}

int main() {
    sf::RenderWindow window(
        sf::VideoMode({1280u, 800u}),
        "Shortest Path Finder -- Dijkstra's Algorithm",
        sf::Style::Close | sf::Style::Titlebar
    );
    window.setFramerateLimit(60);

    if (!loadFont())
        std::cerr << "[Warning] No font found. Place a TTF at assets/font.ttf.\n";

    AppState state = AppState::SPLASH;

    SplashScreen splash(window, g_font);
    LoginScreen  login (window, g_font);
    GraphCanvas  canvas(window, g_font);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (const std::optional<sf::Event> ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>())
                window.close();

            switch (state) {
            case AppState::LOGIN:
                login.handleEvent(*ev, state);
                break;
            case AppState::GRAPH_EDIT:
            case AppState::WEIGHT_INPUT:
            case AppState::ALGO_RUNNING:
            case AppState::SHOW_RESULT:
            case AppState::EXIT_CONFIRM:
                canvas.handleEvent(*ev, state);
                break;
            default:
                break;
            }
        }

        if (!window.isOpen()) break; // window closed during event handling — skip render

        switch (state) {
        case AppState::SPLASH:
            if (splash.update(dt)) state = AppState::LOGIN;
            break;
        case AppState::ALGO_RUNNING:
            canvas.update(dt, state);
            break;
        default:
            break;
        }

        window.clear(sf::Color(18, 18, 30));
        switch (state) {
        case AppState::SPLASH: splash.render(); break;
        case AppState::LOGIN:  login.render();  break;
        default:               canvas.render(state); break;
        }
        window.display();
    }

    return 0;
}
