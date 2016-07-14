#include "TextNode.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

TextNode::TextNode(const FontHolder& fonts, const std::string& text) :
  text()
{
  this->text.setFont(fonts.get(Fonts::Main));
  this->text.setCharacterSize(20);
  setString(text);
}

void TextNode::setString(const std::string& text)
{
  this->text.setString(text);
  centerOrigin(this->text);
}

void TextNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
  target.draw(text, states);
}

