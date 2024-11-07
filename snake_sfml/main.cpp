#define _USE_MATH_DEFINES
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

enum collectableenum
{
    Health,
    SPEED,
    Stoptime
};

enum enemyenum
{
    slow,
    thief
};

struct collectable {
    sf::Vector2f pos;
    sf::RectangleShape shape;
    collectableenum kind;
    collectable(collectableenum type, unsigned int cellsize) {
        pos = { 0,0 };
        kind = type;
        shape.setSize(sf::Vector2f(0.75 * cellsize, 0.75 * cellsize));
        switch (type)
        {
        case Health:
            shape.setFillColor(sf::Color::Red);
            break;
        case SPEED:
            shape.setFillColor(sf::Color::Blue);
            break;
        case Stoptime:
            shape.setFillColor(sf::Color::White);
            break;
        default:
            break;
        }
        shape.setOrigin(sf::Vector2f(-0.125 * cellsize, -0.125 * cellsize));
    }
};

struct enemy
{
    sf::Vector2f pos, dir, targetpos;
    sf::ConvexShape shape;
    enemyenum kind;
    enemy(enemyenum type, unsigned int cellsize)
    {
        pos = dir = targetpos = { 0,0 };
        kind = type;
        shape.setPointCount(3);
        for (int i = 0; i < 3; i++)
        {
            shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3) + 1) * (float)cellsize / 2,-((float)cos(i * 2 * M_PI / 3) + 1) * (float)cellsize / 2 });
        }
        switch (type)
        {
        case slow:
            shape.setFillColor(sf::Color::Color(50, 100, 50, 255));
            break;
        case thief:
            shape.setFillColor(sf::Color::Yellow);
            break;
        default:
            break;
        }
    }
};

std::vector<collectable> collectables;

std::vector<enemy*> enemys;

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
bool wallCollision(sf::Vector2f gridpos);
bool soCollision(sf::Vector2f gridpos);
sf::Vector2f findcpos(std::vector<node>* snake, int levelw, int levelh, int cellsize);
void snakegrow(std::vector<node>* snake, int cellsize);
void selfcollision(std::vector<node>* snake);
void enemycollision(std::vector<node>* snake, enemy e);

std::vector<std::vector<char>> level;

int main()
{
    bool moving = true;
    srand(time(0));
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
    float enemyspeed = 100.0f;
    node head(cellsize);
    head.pos = findPlayer() * (float)cellsize;
    head.targetPos = head.pos;
    snake.push_back(head);

    sf::Clock clock;
    sf::Time stime = sf::Time::Zero;
    sf::Time BonasTime = sf::Time::Zero;

    sf::Vector2f newdir(0, 0);

    collectable h(Health,cellsize);
    h.pos = findcpos(&snake, levelw, levelh, cellsize);
    collectables.push_back(h);

    enemy r1(slow, cellsize);
    r1.pos = r1.targetpos = findcpos(&snake, levelw, levelh, cellsize);
    enemys.push_back(&r1);

    enemy t1(thief, cellsize);
    t1.pos = t1.targetpos = findcpos(&snake, levelw, levelh, cellsize);
    enemys.push_back(&t1);

    while (window.isOpen())
    {
        sf::Time elapsed = clock.restart();
        BonasTime += elapsed;
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
                    snakegrow(&snake,cellsize);
                    moving = !moving;
                }
                else if (evnt.text.unicode == 'd' || evnt.text.unicode == 'D')
                {
                    snake.pop_back();
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

        for (enemy* e: enemys)
        {
            if (e->pos == e->targetpos) {
                if (e->kind == slow)
                {
                    int y = 1;
                    for (int i = 0; i < 4; i++)
                    {
                        int x = rand() % 4 + 2;
                        for (int i = 0; i < 4; i++)
                        {
                            if (e->pos.x == 0) {
                                y *= 2;
                                if (level[(int)e->pos.y / (int)cellsize + 1][(int)e->pos.x / (int)cellsize] != ' ') y *= 5;
                                if (e->pos.y != 0) {
                                    if (level[(int)e->pos.y / (int)cellsize][(int)e->pos.x / (int)cellsize - 1] != ' ') y *= 4;
                                }
                                if (e->pos.y != levelh - 1) {
                                    if (level[(int)e->pos.y / (int)cellsize][(int)e->pos.x / (int)cellsize + 1] != ' ') y *= 3;
                                }
                            }
                            if (e->pos.x == levelw - 1) {
                                y *= 5;
                                if (level[(int)e->pos.y / (int)cellsize - 1][(int)e->pos.x / (int)cellsize] != ' ') y *= 2;
                                if (e->pos.y != 0) {
                                    if (level[(int)e->pos.y / (int)cellsize][(int)e->pos.x / (int)cellsize - 1] != ' ') y *= 4;
                                }
                                if (e->pos.y != levelh - 1) {
                                    if (level[(int)e->pos.y / (int)cellsize][(int)e->pos.x / (int)cellsize + 1] != ' ') y *= 3;
                                }
                            }
                            if (e->pos.y == 0) {
                                y *= 4;
                                if (level[(int)e->pos.y / (int)cellsize][(int)e->pos.x / (int)cellsize + 1] != ' ') y *= 3;
                                if (e->pos.x != 0) {
                                    if (level[(int)e->pos.y / (int)cellsize - 1][(int)e->pos.x / (int)cellsize] != ' ') y *= 2;
                                }
                                if (e->pos.x != levelw - 1) {
                                    if (level[(int)e->pos.y / (int)cellsize + 1][(int)e->pos.x / (int)cellsize] != ' ') y *= 5;
                                }
                            }
                            if (e->pos.y == levelh - 1) {
                                y *= 3;if (level[(int)e->pos.y / (int)cellsize][(int)e->pos.x / (int)cellsize - 1] != ' ') y *= 4;
                                if (e->pos.x != 0) {
                                    if (level[(int)e->pos.y / (int)cellsize - 1][(int)e->pos.x / (int)cellsize] != ' ') y *= 2;
                                }
                                if (e->pos.x != levelw - 1) {
                                    if (level[(int)e->pos.y / (int)cellsize + 1][(int)e->pos.x / (int)cellsize] != ' ') y *= 5;
                                }
                            }
                        }
                        if (x == 2 && ((float)y / (float)x) - (int)(y / x) != 0)
                        {
                            e->dir = { 0,1 };
                            for (int i = 0; i < 3; i++)
                            {
                                e->shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3) + 1) * (float)cellsize / 2,((float)cos(i * 2 * M_PI / 3) + 1) * (float)cellsize / 2 });
                            }
                        }
                        else if (x == 3 && ((float)y / (float)x) - (int)(y / x) != 0)
                        {
                            e->dir = { 1,0 };
                            for (int i = 0; i < 3; i++)
                            {
                                e->shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3 + M_PI / 2) + 1) * (float)cellsize / 2,-((float)cos(i * 2 * M_PI / 3 + M_PI / 2) + 1) * (float)cellsize / 2 });
                            }
                        }
                        else if (x == 4 && ((float)y / (float)x) - (int)(y / x) != 0)
                        {
                            e->dir = { -1,0 };
                            for (int i = 0; i < 3; i++)
                            {
                                e->shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3 + 3 * M_PI / 2) + 1) * (float)cellsize / 2,-((float)cos(i * 2 * M_PI / 3 + 3 * M_PI / 2) + 1) * (float)cellsize / 2 });
                            }
                        }
                        else if (x == 4 && ((float)y / (float)x) - (int)(y / x) != 0)
                        {
                            e->dir = { 0,-1 };
                            for (int i = 0; i < 3; i++)
                            {
                                e->shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3 + M_PI) + 1) * (float)cellsize / 2,((float)cos(i * 2 * M_PI / 3 + M_PI) + 1) * (float)cellsize / 2 });
                            }
                        }
                        else
                        {
                            e->dir = { 0,0 };
                        }
                        if (level[(int)(e->pos.y + e->dir.y * (float)cellsize) / (int)cellsize][(int)(e->pos.x + e->dir.x * (float)cellsize) / (int)cellsize] == ' ') break;
                        y *= x;
                    }
                }
                else if (e->kind == thief)
                {
                    int y = 1;
                    for (int i = 0; i < 2; i++)
                    {
                        sf::Vector2f dir1 = collectables[0].pos - e->pos;
                        int x = rand() % 2;
                        if (x == 0)
                        {
                            dir1 = { dir1.x,0 };
                            if (dir1.x > 0)
                            {
                                x = 3;
                            }
                            else
                            {
                                x = 4;
                            }
                        }
                        else
                        {
                            dir1 = { 0,dir1.y };
                            if (dir1.y > 0)
                            {
                                x = 2;
                            }
                            else
                            {
                                x = 5;
                            }
                        }
                        if (x == 2 && ((float)y / (float)x) - (int)(y / x) != 0)
                        {
                            e->dir = { 0,1 };
                            for (int i = 0; i < 3; i++)
                            {
                                e->shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3) + 1) * (float)cellsize / 2,((float)cos(i * 2 * M_PI / 3)+1) * (float)cellsize / 2 });
                            }
                        }
                        else if (x == 3 && ((float)y / (float)x) - (int)(y / x) != 0)
                        {
                            e->dir = { 1,0 };
                            for (int i = 0; i < 3; i++)
                            {
                                e->shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3 + M_PI / 2) + 1) * (float)cellsize / 2,-((float)cos(i * 2 * M_PI / 3 + M_PI / 2)+1) * (float)cellsize / 2 });
                            }
                        }
                        else if (x == 4 && ((float)y / (float)x) - (int)(y / x) != 0)
                        {
                            e->dir = { -1,0 };
                            for (int i = 0; i < 3; i++)
                            {
                                e->shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3 + 3 * M_PI / 2) + 1) * (float)cellsize / 2,-((float)cos(i * 2 * M_PI / 3 + 3 * M_PI / 2) + 1) * (float)cellsize / 2 });
                            }
                        }
                        else
                        {
                            e->dir = { 0,-1 };
                            for (int i = 0; i < 3; i++)
                            {
                                e->shape.setPoint(i, { ((float)sin(i * 2 * M_PI / 3 + M_PI) + 1) * (float)cellsize / 2,((float)cos(i * 2 * M_PI / 3 + M_PI) + 1) * (float)cellsize / 2 });
                            }
                        }
                        if (level[(int)(e->pos.y + e->dir.y*(int)cellsize) / (int)cellsize][(int)(e->pos.x + e->dir.x*(int)cellsize) / (int)cellsize] == ' ') break;
                        y *= x;
                    }
                }
                e->targetpos += e->dir * (float)cellsize;
            }
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

        for (size_t i = 0; i < enemys.size(); ++i) {
            enemy* n = enemys[i];

            // Move towards target position
            sf::Vector2f moveStep = moving * enemyspeed * n->dir * elapsed.asSeconds();
            if (std::abs(n->targetpos.x - n->pos.x) < std::abs(moveStep.x)) moveStep.x = n->targetpos.x - n->pos.x;
            if (std::abs(n->targetpos.y - n->pos.y) < std::abs(moveStep.y)) moveStep.y = n->targetpos.y - n->pos.y;

            n->pos += moveStep;
        }

        if (stime.asSeconds() != 0)
        {
            stime += elapsed;
        }
        if (stime.asSeconds() > 5)
        {
            moving = true;
        }

        int gridx = (((int)snake[0].pos.x /*+ (int)snake[0].dir.x * (int)cellsize/4*/) / ( (int)cellsize));
        int gridy = (((int)snake[0].pos.y /*+ (int)snake[0].dir.y * (int)cellsize/4*/) / ( (int)cellsize));

        if (soCollision(sf::Vector2f(gridx, gridy)))
        {
            snake.pop_back();
            if (snake.size() == 0) break;
        }

        if (wallCollision(sf::Vector2f(gridx, gridy)))
        {
            if (snake.size() >= 1) break;
            while (snake.size() != 1)
            {
                snake.pop_back();
            }
            moving = false;
        }

        selfcollision(&snake);

        /*for (enemy* e: enemys)
        {
            for (int i = 1; i < snake.size(); i++)
            {
                int x = (int)(snake)[i].pos.x - (int)e->pos.x;
                int y = (int)(snake)[i].pos.y - (int)e->pos.y;
                if (x > 3.5 && y > 3.5)
                {
                    snake.pop_back();
                    if (snake.size() == 1)return 0;
                }
            }
        }*/

        if (BonasTime.asSeconds() > 5 && collectables.size() < 5)
        {
            BonasTime = sf::Time::Zero;
            int x = rand() % 2;
            if (x == 0)
            {
                collectable sh(SPEED, cellsize);
                sh.pos = findcpos(&snake, levelw, levelh, cellsize);
                collectables.push_back(sh);
            }
            else {
                collectable st(Stoptime, cellsize);
                st.pos = findcpos(&snake, levelw, levelh, cellsize);
                collectables.push_back(st);
            }
            BonasTime = sf::Time::Zero;
            level[enemys[0]->pos.y / (float)cellsize][enemys[0]->pos.x / (float)cellsize] = '#';
        }
        if (BonasTime.asSeconds() > 5)
        {
            BonasTime = sf::Time::Zero;
            level[enemys[0]->pos.y / (float)cellsize][enemys[0]->pos.x / (float)cellsize] = '#';
        }

        for (int i = 1; i < collectables.size(); i++)
        {
            if (gridx == collectables[i].pos.x / cellsize && gridy == collectables[i].pos.y / cellsize)
            {
                switch (collectables[i].kind)
                {
                case Stoptime:
                    moving = false;
                    stime = elapsed;
                    break;
                case SPEED:
                    speed += 20;
                    break;
                default:
                    break;
                }
                collectables.erase(collectables.begin() + i);
            }
        }
        if (gridx == collectables[0].pos.x / cellsize && gridy == collectables[0].pos.y / cellsize)
        {
            snakegrow(&snake, cellsize);
            collectables[0].pos = findcpos(&snake, levelw, levelh, cellsize);
        }
        if (enemys.size() > 0)
        {
            int egridx = (((int)enemys[1]->pos.x+(int)enemys[1]->dir.x) / (int)cellsize);
            int egridy = (((int)enemys[1]->pos.y+(int)enemys[1]->dir.y) / (int)cellsize);
            if (egridx == collectables[0].pos.x / cellsize && egridy == collectables[0].pos.y / cellsize)
            {
                collectables[0].pos = findcpos(&snake, levelw, levelh, cellsize);
            }
        }

        window.clear();
        for (int i = 0; i < level.size(); i++)
        {
            for (int j = 0; j < (level[i]).size(); j++) {
                sf::RectangleShape rectangle;
                rectangle.setOrigin(sf::Vector2f(-0.05 * cellsize, -0.05 * cellsize));
                rectangle.setSize(sf::Vector2f(0.95 * cellsize, 0.95 * cellsize));
                rectangle.setFillColor(sf::Color(18, 18, 18, 255));
                if (level[i][j] == '#') rectangle.setFillColor(sf::Color(100, 18, 180, 255));
                if (level[i][j] == '*') rectangle.setFillColor(sf::Color(160, 38, 180, 255));
                rectangle.setPosition(sf::Vector2f(j * cellsize, i * cellsize));
                window.draw(rectangle);
            }
        }

        for (collectable c : collectables)
        {
            c.shape.setPosition(c.pos);
            window.draw(c.shape);
        }

        for (enemy* e : enemys)
        {
            e->shape.setPosition(e->pos);
            window.draw(e->shape);
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

bool wallCollision(sf::Vector2f gridpos)
{
    return (level[gridpos.y][gridpos.x] == '#');
}

bool soCollision(sf::Vector2f gridpos)
{
    if (level[gridpos.y][gridpos.x] == '*')
    {
        level[gridpos.y][gridpos.x] = ' ';
        return true;
    }
    return false;
}

void selfcollision(std::vector<node>* snake)
{
    for (int i = 1; i < snake->size(); i++)
    {
        int x = (int)(*snake)[i].pos.x - (int)(*snake)[0].pos.x;
        int y = (int)(*snake)[i].pos.y - (int)(*snake)[0].pos.y;
        if ( x == 0 && y == 0)
        {
            for (int j = 0; j < snake->size()-i; j++)
            {
                snake->pop_back();
            }
            return;
        }
    }
}

sf::Vector2f findcpos(std::vector<node>* snake, int levelw, int levelh, int cellsize)
{
    sf::Vector2f pos(0, 0);
here:
    while (true)
    {
        for (node n : *snake)
        {
            int x = (int)(n.pos.x / (float)cellsize) - (int)(pos.x / (float)cellsize);
            int y = (int)(n.pos.y / (float)cellsize) - (int)(pos.y / (float)cellsize);
            if ( x == 0 && y == 0)
            {
                pos.x = rand() % levelw;
                pos.y = rand() % levelh;
                goto here;
            }
        }
        for (collectable c : collectables)
        {
            int x = (int)(c.pos.x / (float)cellsize) - (int)(pos.x / (float)cellsize);
            int y = (int)(c.pos.y / (float)cellsize) - (int)(pos.y / (float)cellsize);
            if (x == 0 && y == 0)
            {
                pos.x = rand() % levelw;
                pos.y = rand() % levelh;
                goto here;
            }
        }
        if (level[pos.y][pos.x] == ' ') return pos * (float)cellsize;
        pos.x = rand() % levelw;
        pos.y = rand() % levelh;
    }
}

void snakegrow(std::vector<node>* snake,int cellsize) {
    node body(cellsize);
    body.pos = snake->back().pos - snake->back().dir * (float)cellsize;
    body.targetPos = snake->back().pos;
    body.dir = snake->back().dir;
    snake->push_back(body);
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
