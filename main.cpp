#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <algorithm>

void runGame(sf::RenderWindow& window); 

enum class Menu {
    NewGame = 0,
    Options = 1,
    Quit = 2
};

int main() {
    sf::RenderWindow window(sf::VideoMode({ 960, 540 }), "Main Menu");
    window.setKeyRepeatEnabled(false);

    // Assets
    sf::Texture logoTex, indicatorTex;
    if (!logoTex.loadFromFile("assets/image/logo.png")) std::cerr << "logo load failed\n";
    if (!indicatorTex.loadFromFile("assets/image/indecator.png")) std::cerr << "indicator load failed\n";
    logoTex.setSmooth(true);

    sf::Font font;
    if (!font.openFromFile("assets/font/fixedsys.ttf")) std::cerr << "font load failed\n";

    // Title
    sf::Text title{ font, sf::String("PingPong"), 80 };
    title.setFillColor(sf::Color(240, 240, 240));
    title.setOutlineColor(sf::Color(20, 20, 20));
    title.setOutlineThickness(2.f);
    {
        const sf::FloatRect b = title.getLocalBounds();
        title.setOrigin(sf::Vector2f{ b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f }); // center origin [web:12]
    }

    // Title stays the same above...

// Logo (center origin once so positioning is by center)
    sf::Sprite logo{ logoTex };
    {
        const auto lb = logo.getLocalBounds();
        logo.setOrigin(sf::Vector2f{ lb.size.x * 0.5f, lb.size.y * 0.5f });
    }

    // Menu labels
    sf::Text lblNewGame{ font, sf::String("NewGame"), 24 };
    sf::Text lblOptions{ font, sf::String("Options"), 24 };
    sf::Text lblQuit{ font, sf::String("Quit"),    24 };
    sf::Text* labels[3] = { &lblNewGame, &lblOptions, &lblQuit };

    auto setDim = [&] {
        for (auto* t : labels) {
            t->setFillColor(sf::Color(160, 160, 160));
            t->setStyle(sf::Text::Regular);
        }
        };
    auto highlight = [&](int idx) {
        setDim();
        labels[idx]->setFillColor(sf::Color(240, 240, 240));
        };

    // Indicator
    sf::Sprite indicator{ indicatorTex };

    // Layout (no layoutLogo; just scale+position the logo directly)
    auto layout = [&] {
        const auto view = window.getView();
        const sf::Vector2f VC = view.getCenter();
        const sf::Vector2f VS = view.getSize();

        title.setPosition(sf::Vector2f{ VC.x, VC.y - VS.y * 0.32f });

        // Simple logo placement: fixed scale and position under the title
        logo.setScale(sf::Vector2f{ 0.45f, 0.45f });                 // shrink to ~45% [web:3]
        logo.setPosition(sf::Vector2f{ VC.x, VC.y - VS.y * 0.08f }); // centered under title [web:3]

        const float menuLeft = VC.x - VS.x * 0.40f;
        const float firstBaseline = VC.y - VS.y * 0.05f;
        const float line = 30.f;

        for (int i = 0; i < 3; ++i) {
            labels[i]->setPosition(sf::Vector2f{ menuLeft + 20.f, firstBaseline + i * line });
        }
        };

    int index = static_cast<int>(Menu::NewGame);
    setDim();
    highlight(index);
    layout();

    auto alignIndicator = [&] {
        const sf::Vector2f p = labels[index]->getPosition();
        indicator.setPosition(sf::Vector2f{ p.x - 10.f, p.y - 1.f });
        };
    alignIndicator();

    // Loop
    while (window.isOpen()) {
        while (const auto e = window.pollEvent()) { 
            if (e->is<sf::Event::Closed>()) window.close();
            else if (const auto* k = e->getIf<sf::Event::KeyPressed>()) {
                using S = sf::Keyboard::Scancode;
                if (k->scancode == S::Down || k->scancode == S::PageDown) { index = (index + 1) % 3; highlight(index); alignIndicator(); } 
                else if (k->scancode == S::Up || k->scancode == S::PageUp) { index = (index + 2) % 3; highlight(index); alignIndicator(); } 
                else if (k->scancode == S::Escape) window.close();
                else if (k->scancode == S::Enter || k->scancode == S::NumpadEnter) {
                    if (index == static_cast<int>(Menu::Quit)) {
                        window.close();
                    }
                    else if (index == static_cast<int>(Menu::NewGame)) {
                        // Run gameplay in same window; returns here when done
                        runGame(window); 
                        
                        layout();
                        alignIndicator();
                    }
                    else if (index == static_cast<int>(Menu::Options)) {
                        // Will Update this if no lazy hehe 
                    }
                }
            }
            else if (e->is<sf::Event::Resized>()) {
                const auto size = window.getSize();
                sf::View v(sf::FloatRect(sf::Vector2f{ 0.f,0.f },
                    sf::Vector2f{ static_cast<float>(size.x), static_cast<float>(size.y) }));
                window.setView(v);
                layout();
                alignIndicator();
            }
        }

        window.clear(sf::Color(10, 10, 12));
        window.draw(logo);
        window.draw(title);
        window.draw(indicator);
        for (auto* t : labels) window.draw(*t);
        window.display();
    }
    return 0;
}
