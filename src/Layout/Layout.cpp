#include <boost/algorithm/string.hpp>
#include <stack>
#include <unordered_map>
#include <SFML/Graphics.hpp>

#include "Layout.hpp"

namespace WordSizes
{
    std::unordered_map<std::wstring, int> word_lengths;
    std::unordered_map<std::wstring, int> word_heights;
}

Layout::Layout()
    : cursor_x(0), cursor_y(0), is_font_set(font.loadFromFile("/usr/share/fonts/TTF/DejaVuSerif.ttf")),
      text(), is_last_box_finalized(false)
{
    text.setCharacterSize(14);
    text.setFont(font);

    initializeSpaceWordSizes();
}

void Layout::initializeSpaceWordSizes()
{
    if (WordSizes::word_lengths.count(L" ") == 0)
    {
        text.setString(" ");
        WordSizes::word_lengths[L" "] = text.getLocalBounds().width;
        WordSizes::word_heights[L" "] = text.getLocalBounds().height;
        text.setString("");
    }
}

int Layout::get_word_length(const std::wstring &word)
{
    return get_word_size(word).first;
}

int Layout::get_word_height(const std::wstring &word)
{
    return get_word_size(word).second;
}

std::pair<int, int> Layout::get_word_size(const std::wstring &word)
{
    auto &word_lengths = WordSizes::word_lengths;
    auto &word_heights = WordSizes::word_heights;

    if (word_lengths.find(word) == word_lengths.end())
    {
        text.setString(word);
        int word_length = text.getLocalBounds().width;
        int word_height = text.getLocalBounds().height;

        word_lengths[word] = word_length;
        word_heights[word] = word_height;

        return {word_length, word_height};
    }

    return {word_lengths.at(word), word_heights.at(word)};
}

void Layout::construct_from_document(const Document &doc)
{
    boxes.clear();
    cursor_x = 0;
    cursor_y = 0;
    document = doc;

    processDocumentNode(document.get_elements().front()->get_children());
}

void Layout::processDocumentNode(const std::vector<std::shared_ptr<HTMLElement>> &children)
{
    for (const auto &node : children)
    {
        if (node->is_text_node())
            add_string(node->get_text());
        else if (node->is_paragraph_node() && !boxes.empty())
            finalizeCurrentBox();
        else if (!node->get_children().empty())
            processDocumentNode(node->get_children());
    }
}

void Layout::finalizeCurrentBox()
{
    is_last_box_finalized = true;
    cursor_y += PARAGRAPH_BREAK_PADDING;
}

void Layout::add_string(const std::wstring &string)
{
    std::vector<std::wstring> words;
    boost::split(words, string, boost::is_any_of(L" "));

    for (const auto &word : words)
    {
        addWordToCurrentBox(word);
    }
}

void Layout::addWordToCurrentBox(const std::wstring &word)
{
    auto [word_length, word_height] = get_word_size(word);

    if (!boxes.empty() && !is_last_box_finalized)
    {
        extendCurrentBox(word, word_length, word_height);
    }
    else
    {
        startNewBox(word, word_length, word_height);
    }
}

void Layout::extendCurrentBox(const std::wstring &word, int word_length, int word_height)
{
    auto &current_box = boxes.back();

    if (fitsWithinWidth(current_box.get_width() + word_length))
    {
        current_box.extendBox(word);
    }
    else
    {
        finalizeCurrentBox();
        startNewBox(word, word_length, word_height);
    }
}

bool Layout::fitsWithinWidth(int length) const
{
    return cursor_x + length <= width;
}

void Layout::startNewBox(const std::wstring &word, int word_length, int word_height)
{
    Box new_box(cursor_x, cursor_y);
    new_box.extendBox(word);
    boxes.push_back(new_box);
    is_last_box_finalized = false;
}

std::vector<Box> Layout::get_boxes() const
{
    return boxes;
}

int Layout::get_max_width()
{
    return std::max_element(boxes.begin(), boxes.end(), [](const auto &a, const auto &b) {
               return a.get_width() < b.get_width();
           })->get_width();
}

int Layout::get_max_height()
{
    return std::max_element(boxes.begin(), boxes.end(), [](const auto &a, const auto &b) {
               return a.get_coordinates().at(1) + a.get_height() < b.get_coordinates().at(1) + b.get_height();
           })->get_coordinates().at(1) + std::max_element(boxes.begin(), boxes.end(), [](const auto &a, const auto &b) {
                                                          return a.get_height() < b.get_height();
                                                      })->get_height();
}

void Layout::update()
{
    construct_from_document(document);

    auto center = view.getCenter();
    auto size = view.getSize();
    sf::FloatRect view_bounds(center.x - (size.x / 2), center.y - (size.y / 2), size.x, size.y);

    for (auto &box : boxes)
    {
        auto coordinates = box.get_coordinates();
        box.set_visible(view_bounds.intersects(sf::FloatRect(coordinates.at(0), coordinates.at(1), box.get_width(), box.get_height())));
    }
}

void Layout::set_width(const int &new_width)
{
    width = new_width;
}

void Layout::set_view(const sf::View &new_view)
{
    view = new_view;
}
