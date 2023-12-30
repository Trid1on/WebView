#include <iostream>
#include "WW_Window/WW_Window.hpp"
#include "Document/Document.hpp"
#include "HTMLParser/HTMLParser.hpp"
#include "Loader/Loader.hpp"

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

int main(int argc, char* argv[])
{
    Loader loader;
    const std::wstring html = (argc < 2) ? loader.load_from_file("../../tests/html_files/demo.html") : loader.load_from_file(std::string(argv[1]));

    if (html.empty())
        return -1;

    WW_Window window(WINDOW_WIDTH, WINDOW_HEIGHT);
    HTMLParser parser;
    Document document = parser.construct_document_from_string(html);

    Layout layout;
    layout.set_width(WINDOW_WIDTH);
    layout.set_view(window.getView());
    layout.construct_from_document(document);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            handleEvent(event, window, layout);
        }

        window.load_from_layout(layout);
        window.clear(sf::Color::White);
        window.show_layout();
        window.display();
    }

    return 0;
}

void handleEvent(const sf::Event& event, WW_Window& window, Layout& layout)
{
    switch (event.type)
    {
        case sf::Event::Closed:
            window.close();
            break;

        case sf::Event::KeyPressed:
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                window.close();
            break;

        case sf::Event::Resized:
            handleResizeEvent(window, layout);
            break;

        default:
            break;
    }
}

void handleResizeEvent(WW_Window& window, Layout& layout)
{
    sf::Vector2f size = static_cast<sf::Vector2f>(window.getSize());
    sf::View view(sf::FloatRect(0.f, 0.f, size.x, size.y));
    window.setView(view);
    layout.set_view(view);
    layout.set_width(size.x);
    layout.update();
}
