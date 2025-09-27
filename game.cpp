#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <random>
#include <cmath>
#include <string>

// Runs the gameplay loop inside the existing window.
// Returns to the menu on Escape or if the window closes.
void runGame(sf::RenderWindow& window) {
    // Remember the menu's view and switch to a local one for gameplay.
    const sf::View savedView = window.getView();

    const auto winSize = window.getSize();
    sf::View view(sf::FloatRect({ 0.f, 0.f },
        { static_cast<float>(winSize.x), static_cast<float>(winSize.y) }));
    window.setView(view);

    // Virtual playfield
    const float W = 800.f;
    const float H = 600.f;
    const sf::Vector2f paddleSize(25.f, 100.f);
    const float ballR = 10.f;

    // Center camera on the virtual playfield
    view.setSize({ W, H });
    view.setCenter({ W * 0.5f, H * 0.5f });
    window.setView(view);

    // Audio
    sf::SoundBuffer bounceBuf;
    bounceBuf.loadFromFile("resources/ball.wav");
    sf::Sound bounce(bounceBuf);

    // Logo (shown on pause)
   

    // Paddles
    sf::RectangleShape leftPaddle;
    leftPaddle.setSize(paddleSize - sf::Vector2f(3.f, 3.f));
    leftPaddle.setOutlineThickness(3.f);
    leftPaddle.setOutlineColor(sf::Color::Black);
    leftPaddle.setFillColor(sf::Color(100, 100, 200));
    leftPaddle.setOrigin(paddleSize * 0.5f);

    sf::RectangleShape rightPaddle = leftPaddle;
    rightPaddle.setFillColor(sf::Color(200, 100, 100));

    // Ball
    sf::CircleShape ball;
    ball.setRadius(ballR - 3.f);
    ball.setOutlineThickness(2.f);
    ball.setOutlineColor(sf::Color::Black);
    ball.setFillColor(sf::Color::White);
    ball.setOrigin({ ballR * 0.5f, ballR * 0.5f });

    // Pause text
    sf::Font font;
    font.openFromFile("resources/tuffy.ttf");
    sf::Text pause(font);
    pause.setCharacterSize(40);
    pause.setPosition({ 170.f, 200.f });
    pause.setFillColor(sf::Color::White);
    pause.setString("Welcome to SFML PingPong!\n\nPress Space to start the game.");

    // Timing and motion
    std::random_device rd;
    std::mt19937 rng(rd());
    sf::Clock aiClock;
    const sf::Time aiTick = sf::seconds(0.1f);

    const float paddleSpeed = 400.f;
    float rightPaddleSpeed = 0.f;

    const float ballSpeed = 400.f;
    sf::Angle ballDir = sf::degrees(0); // direction of travel

    sf::Clock stepClock;
    bool playing = false;
    bool running = true;

    auto reset = [&] {
        playing = true;
        stepClock.restart();

        leftPaddle.setPosition({ 10.f + paddleSize.x * 0.5f, H * 0.5f });
        rightPaddle.setPosition({ W - 10.f - paddleSize.x * 0.5f, H * 0.5f });
        ball.setPosition({ W * 0.5f, H * 0.5f });

        // Randomize direction, avoid near-vertical angles
        std::uniform_real_distribution<float> deg(0.f, 360.f);
        do {
            ballDir = sf::degrees(deg(rng));
        } while (std::abs(std::cos(ballDir.asRadians())) < 0.7f); // requires radians for trig
        };

    while (running && window.isOpen()) {
        while (const std::optional ev = window.pollEvent()) { // SFML 3 uses std::optional here
            if (ev->is<sf::Event::Closed>()) {
                window.close();
                running = false;
                break;
            }
            else if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                using S = sf::Keyboard::Scancode;
                if (kp->scancode == S::Escape) {
                    running = false; // back to menu
                }
                else if (kp->scancode == S::Space) {
                    if (!playing) reset();
                }
            }
            else if (const auto* r = ev->getIf<sf::Event::Resized>()) {
                // Keep camera covering the virtual playfield
                sf::View v(sf::FloatRect({ 0.f, 0.f },
                    { static_cast<float>(r->size.x), static_cast<float>(r->size.y) }));
                v.setSize({ W, H });
                v.setCenter({ W * 0.5f, H * 0.5f });
                window.setView(v);
            }
        }
        if (!running || !window.isOpen()) break;

        if (playing) {
            const float dt = stepClock.restart().asSeconds();

            // Player paddle (smooth, polled input)
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) &&
                (leftPaddle.getPosition().y - paddleSize.y * 0.5f > 5.f)) {
                leftPaddle.move({ 0.f, -paddleSpeed * dt });
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) &&
                (leftPaddle.getPosition().y + paddleSize.y * 0.5f < H - 5.f)) {
                leftPaddle.move({ 0.f, paddleSpeed * dt });
            }

            // Simple AI
            if (aiClock.getElapsedTime() > aiTick) {
                aiClock.restart();
                if (ball.getPosition().y + ballR > rightPaddle.getPosition().y + paddleSize.y * 0.5f) rightPaddleSpeed = paddleSpeed;
                else if (ball.getPosition().y - ballR < rightPaddle.getPosition().y - paddleSize.y * 0.5f) rightPaddleSpeed = -paddleSpeed;
                else rightPaddleSpeed = 0.f;
            }
            const float rpTop = rightPaddle.getPosition().y - paddleSize.y * 0.5f;
            const float rpBot = rightPaddle.getPosition().y + paddleSize.y * 0.5f;
            if (((rightPaddleSpeed < 0.f) && (rpTop > 5.f)) ||
                ((rightPaddleSpeed > 0.f) && (rpBot < H - 5.f))) {
                rightPaddle.move({ 0.f, rightPaddleSpeed * dt });
            }

            // Move the ball using angle API
            const float dx = std::cos(ballDir.asRadians()) * ballSpeed * dt;
            const float dy = std::sin(ballDir.asRadians()) * ballSpeed * dt;
            ball.move({ dx, dy });

            const std::string prompt = "Press Space to restart or\nEscape to go back.";

            // Side walls (goal)
            if (ball.getPosition().x - ballR < 0.f) {
                playing = false;
                pause.setString("You Lost!\n\n" + prompt);
            }
            if (ball.getPosition().x + ballR > W) {
                playing = false;
                pause.setString("You Won!\n\n" + prompt);
            }

            // Top/bottom bounce
            if (ball.getPosition().y - ballR < 0.f) {
                bounce.play();
                ballDir = -ballDir;
                ball.setPosition({ ball.getPosition().x, ballR + 0.1f });
            }
            if (ball.getPosition().y + ballR > H) {
                bounce.play();
                ballDir = -ballDir;
                ball.setPosition({ ball.getPosition().x, H - ballR - 0.1f });
            }

            // Paddles
            std::uniform_real_distribution<float> jitter(0.f, 20.f);

            // Left paddle
            if (ball.getPosition().x - ballR < leftPaddle.getPosition().x + paddleSize.x * 0.5f &&
                ball.getPosition().x - ballR > leftPaddle.getPosition().x &&
                ball.getPosition().y + ballR >= leftPaddle.getPosition().y - paddleSize.y * 0.5f &&
                ball.getPosition().y - ballR <= leftPaddle.getPosition().y + paddleSize.y * 0.5f) {
                const sf::Angle d = sf::degrees(jitter(rng));
                ballDir = sf::degrees(180) - ballDir + (ball.getPosition().y > leftPaddle.getPosition().y ? d : -d);
                bounce.play();
                ball.setPosition({ leftPaddle.getPosition().x + ballR + paddleSize.x * 0.5f + 0.1f, ball.getPosition().y });
            }

            // Right paddle
            if (ball.getPosition().x + ballR > rightPaddle.getPosition().x - paddleSize.x * 0.5f &&
                ball.getPosition().x + ballR < rightPaddle.getPosition().x &&
                ball.getPosition().y + ballR >= rightPaddle.getPosition().y - paddleSize.y * 0.5f &&
                ball.getPosition().y - ballR <= rightPaddle.getPosition().y + paddleSize.y * 0.5f) {
                const sf::Angle d = sf::degrees(jitter(rng));
                ballDir = sf::degrees(180) - ballDir + (ball.getPosition().y > rightPaddle.getPosition().y ? d : -d);
                bounce.play();
                ball.setPosition({ rightPaddle.getPosition().x - ballR - paddleSize.x * 0.5f - 0.1f, ball.getPosition().y });
            }
        }

        window.clear(sf::Color(50, 50, 50));
        if (playing) {
            window.draw(leftPaddle);
            window.draw(rightPaddle);
            window.draw(ball);
        }
        else {
            window.draw(pause);
            
        }
        window.display();
    }

    // Hand control back to the menu
    window.setView(savedView);
}
