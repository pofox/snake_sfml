#include <SFML/Graphics.hpp>

int main()
{
    sf::RenderWindow window(sf::VideoMode(1080, 720), "SFML works!",sf::Style::Close | sf::Style::Resize);
    sf::RectangleShape rectangle[88];

    for (int i = 0; i < 88; i++)
    {
        rectangle[i].setOrigin(sf::Vector2f(-5.0f, -5.0f));
        rectangle[i].setSize(sf::Vector2f(95.0f, 95.0f));
        rectangle[i].setFillColor(sf::Color(18, 18, 18, 255));
        rectangle[i].setPosition(sf::Vector2f((i%11)*100.0f, ((int)(i/11))*100.0f));
    }
    
    sf::CircleShape shape(45.0f);
    shape.setFillColor(sf::Color::Green);
    shape.setOrigin(sf::Vector2f(-7.5f, -7.5f));
    sf::Vector2f pos(0, 0);

    while (window.isOpen())
    {
        sf::Event evnt;
        while (window.pollEvent(evnt))
        {
            switch (evnt.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::TextEntered:
                switch (evnt.text.unicode)
                {
                case 'W':
                case 'w':
                    pos.y -= 100;
                    break;
                case 'S':
                case 's':
                    pos.y += 100;
                    break;
                case 'A':
                case 'a':
                    pos.x -= 100;
                    break;
                case 'D':
                case 'd':
                    pos.x += 100;
                    break;
                default:
                    break;
                }
                shape.setPosition(pos);
                if (evnt.text.unicode < 128)
                {
                    printf("%c", evnt.text.unicode);
                }
            }
        }

        window.clear();
        for (int i = 0; i < 88; i++)
        {
            window.draw(rectangle[i]);
        }
        window.draw(shape);
        window.display();
    }

    return 0;
}