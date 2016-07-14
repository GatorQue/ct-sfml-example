#ifndef SOURCES_SCOUT_TEXTNODE_HPP_
#define SOURCES_SCOUT_TEXTNODE_HPP_

#include "SceneNode.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

class TextNode : public SceneNode
{
  public:
    explicit TextNode(const FontHolder& fonts, const std::string& text);

    void setString(const std::string& text);

  private:
    sf::Text text;

    virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif
