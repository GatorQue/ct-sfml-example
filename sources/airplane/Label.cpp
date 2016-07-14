#include "Label.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace GUI
{
  Label::Label(const std::string& text, const FontHolder& fonts) :
    text(text, fonts.get(Fonts::Main), 16)
  {
  }

  bool Label::isSelectable() const
  {
    return false;
  }

  void Label::setText(const std::string& text)
  {
    this->text.setString(text);
  }

  void Label::handleEvent(const sf::Event& event)
  {
    // Do nothing
  }

  void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    states.transform *= getTransform();
    target.draw(text, states);
  }
}
