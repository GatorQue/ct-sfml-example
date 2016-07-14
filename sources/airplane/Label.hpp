#ifndef SOURCES_SCOUT_LABEL_HPP_
#define SOURCES_SCOUT_LABEL_HPP_

#include "Component.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Text.hpp>

namespace GUI
{
  class Label : public Component
  {
    public:
      typedef std::shared_ptr<Label> Ptr;

      Label(const std::string& text, const FontHolder& fonts);

      virtual bool isSelectable() const;
      void setText(const std::string& text);

      virtual void handleEvent(const sf::Event& event);

    private:
      sf::Text text;

      void draw(sf::RenderTarget& target, sf::RenderStates states) const;
  };

}

#endif
