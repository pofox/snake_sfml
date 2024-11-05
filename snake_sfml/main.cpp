#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>

struct node {
    sf::Vector2f olddir, dir, pos, targetPos;
    sf::CircleShape shape;
    node(unsigned int cellsize) {
        olddir = dir = pos = targetPos = { 0, 0 };
        shape.setRadius(0.45 * cellsize);
        shape.setFillColor(sf::Color::Green);
        shape.setOrigin(sf::Vector2f(-0.075 * cellsize, -0.075 * cellsize));
    }
};

sf::Vector2f scaleToFit(const sf::Vector2f& in, const sf::Vector2f& clip)
{
    sf::Vector2f ret(in);
    if ((clip.y * in.x) / in.y >= clip.x)
    {
        ret.y = (clip.x * in.y) / in.x;
        ret.x = clip.x;
    }
    else if ((clip.x * in.y) / in.x >= clip.y)
    {
        ret.x = (clip.y * in.x) / in.y;
        ret.y = clip.y;
    }
    else
        ret = clip;
    return ret;
}

sf::Vector2f findPlayer();
void fileToVec(std::string file);

std::vector<std::vector<char> > level;

int main()
{
    fileToVec("assets/levels/1.txt");
    unsigned int levelw = 0;
    for (int i = 0; i < level.size(); i++) if (levelw < level[i].size()) levelw = level[i].size();
    unsigned int levelh = level.size();
    
    unsigned int screenw = 1080;
    unsigned int screenh = 720;

    unsigned int cellsize = fmin(screenw / levelw, screenh / levelh);
    
    sf::RenderWindow window(sf::VideoMode(screenw, screenh), "Snake!", sf::Style::Close | sf::Style::Resize);
    window.setFramerateLimit(60);
    
    sf::FloatRect visibleArea(((int)levelw * (int)cellsize - (int)screenw) / 2, (((int)levelh * (int)cellsize) - (int)screenh) / 2, screenw, screenh);
    window.setView(sf::View(visibleArea));
    
    std::vector<node> snake;
    float speed = 200.0f; // Snake movement speed per second
    node head(cellsize);
    head.pos = findPlayer() * (float)cellsize;
    head.targetPos = head.pos;
    snake.push_back(head);

    sf::Clock clock;

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
                    node body(cellsize);
                    body.pos = snake.back().pos - snake.back().dir * (float)cellsize;
                    body.targetPos = snake.back().pos;
                    body.dir = snake.back().dir;
                    snake.push_back(body);
                }
            }else if (evnt.type == sf::Event::Resized)
            {
                // update the view to the new size of the window
                unsigned int windoww = evnt.size.width;
                unsigned int windowh = evnt.size.height;
                int left = 0;
                int top = 0;
                sf::Vector2f r = scaleToFit(sf::Vector2f(levelw * cellsize, levelh * cellsize), sf::Vector2f(windoww, windowh));
                left = (1 - windoww / r.x) * levelw * cellsize / 2;
                top = (1 - windowh / r.y) * levelh * cellsize / 2;
                sf::FloatRect visibleArea(left, top, (levelw * cellsize * windoww) / r.x, (levelh * cellsize * windowh) / r.y);
                window.setView(sf::View(visibleArea));
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
            snake[0].targetPos += snake[0].dir * (float)cellsize;
        }

        // Move each segment towards its target position smoothly
        for (size_t i = 0; i < snake.size(); ++i) {
            node& n = snake[i];

            // Move towards target position
            sf::Vector2f moveStep = speed * n.dir * elapsed.asSeconds();
            if (std::abs(n.targetPos.x - n.pos.x) < std::abs(moveStep.x)) moveStep.x = n.targetPos.x - n.pos.x;
            if (std::abs(n.targetPos.y - n.pos.y) < std::abs(moveStep.y)) moveStep.y = n.targetPos.y - n.pos.y;

            n.pos += moveStep;
        }

        window.clear();
        for (int i = 0; i < level.size(); i++)
        {
            for (int j = 0; j < (level[i]).size(); j++) {
                sf::RectangleShape rectangle;
                rectangle.setOrigin(sf::Vector2f(-0.05 * cellsize, -0.05 * cellsize));
                rectangle.setSize(sf::Vector2f(0.95 * cellsize, 0.95 * cellsize));
                rectangle.setFillColor(sf::Color(18, 18, 18, 255));
                if (level[i][j] == '#')
                {
                    rectangle.setFillColor(sf::Color(100, 18, 180, 255));
                }
                rectangle.setPosition(sf::Vector2f(j * cellsize, i * cellsize));
                window.draw(rectangle);
            }
        }

        // Draw all nodes (snake segments)
        for (auto& n : snake) {
            n.shape.setPosition(n.pos);
            window.draw(n.shape);
        }

        window.display();
    }

    return 0;
}

// player is '@'
sf::Vector2f findPlayer()
{
    int i, j;
    for (i = 0; i < level.size(); i++) {
        for (j = 0; j < level[i].size(); j++) {
            if (level[i][j] == '@')
            {
                return sf::Vector2f(j, i);
            }
        }
    }

}

void fileToVec(std::string file)
{
    std::string line;
    std::ifstream infile(file);

    while (std::getline(infile, line)) {
        std::vector<char> row;

        for (char& c : line) {
            if (c != ',') {
                row.push_back(c);
            }
        }

        level.push_back(row);
    }

}
