#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>

struct node {
    sf::Vector2f olddir, dir, pos, targetPos;
    sf::CircleShape shape;
    node() {
        olddir = dir = pos = targetPos = { 0, 0 };
        shape.setRadius(45.0f);
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin(sf::Vector2f(-7.5f, -7.5f));
    }
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(1080, 720), "Snake!", sf::Style::Close | sf::Style::Resize);
    window.setFramerateLimit(60);
    sf::RectangleShape rectangle[88];

    std::vector<node> snake;
    float speed = 200.0f; // Snake movement speed per second
    node head;
    head.targetPos = head.pos;
    snake.push_back(head);

    sf::Clock clock;
    for (int i = 0; i < 88; i++) {
        rectangle[i].setOrigin(sf::Vector2f(-5.0f, -5.0f));
        rectangle[i].setSize(sf::Vector2f(95.0f, 95.0f));
        rectangle[i].setFillColor(sf::Color(18, 18, 18, 255));
        rectangle[i].setPosition(sf::Vector2f((i % 11) * 100.0f, ((int)(i / 11)) * 100.0f));
    }

    sf::Vector2f newdir(0, 0);

    while (window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        sf::Event evnt;

        // Check for direction change with arrow keys
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && snake[0].dir.y != 1) newdir = sf::Vector2f(0, -1);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && snake[0].dir.y != -1) newdir = sf::Vector2f(0, 1);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && snake[0].dir.x != -1) newdir = sf::Vector2f(1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && snake[0].dir.x != 1) newdir = sf::Vector2f(-1, 0);

        while (window.pollEvent(evnt)) {
            if (evnt.type == sf::Event::Closed) window.close();
            else if (evnt.type == sf::Event::TextEntered) {
                if (evnt.text.unicode == 't' || evnt.text.unicode == 'T') {
                    node body;
                    body.pos = snake.back().pos - snake.back().dir * 100.0f;
                    body.targetPos = snake.back().pos;
                    body.dir = snake.back().dir;
                    snake.push_back(body);
                }
            }
        }

        // Set target position for head if it reached the previous target
        if (snake[0].pos == snake[0].targetPos) {
            for (int i = snake.size() - 1; i > 0; i--)
            {
                snake[i].olddir = snake[i].dir;
                snake[i].dir = snake[i - 1].dir;
                snake[i].targetPos = snake[i - 1].pos;
            }
            snake[0].olddir = snake[0].dir;
            snake[0].dir = newdir;
            snake[0].targetPos += snake[0].dir * 100.0f;
        }

        // Move each segment towards its target position smoothly
        for (size_t i = 0; i < snake.size(); ++i) {
            node& n = snake[i];

            // Move towards target position
            sf::Vector2f moveStep = speed * n.dir * elapsed.asSeconds();
            if (std::abs(n.targetPos.x - n.pos.x) < std::abs(moveStep.x)) moveStep.x = n.targetPos.x - n.pos.x;
            if (std::abs(n.targetPos.y - n.pos.y) < std::abs(moveStep.y)) moveStep.y = n.targetPos.y - n.pos.y;

            n.pos += moveStep;

            // Update target for each segment to follow the previous segment
            /*if (i > 0 && snake[i].pos == snake[i].targetPos) {
                snake[i].dir = snake[i - 1].olddir;
                snake[i].targetPos = snake[i - 1].pos;
            }*/
        }

        // Save current directions for all segments
        /*for (auto& n : snake) {
            n.olddir = n.dir;
        }*/

        window.clear();
        for (int i = 0; i < 88; i++) window.draw(rectangle[i]);

        // Draw all nodes (snake segments)
        for (auto& n : snake) {
            n.shape.setPosition(n.pos);
            window.draw(n.shape);
        }

        window.display();
    }

    return 0;
}
