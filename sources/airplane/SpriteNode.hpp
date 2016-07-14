#ifndef SOURCES_SCOUT_SPRITENODE_HPP_
#define SOURCES_SCOUT_SPRITENODE_HPP_

#include "SceneNode.hpp"

#include <SFML/Graphics/Sprite.hpp>

class SpriteNode : public SceneNode
{
  public:
    explicit SpriteNode(const sf::Texture& texture);
    SpriteNode(const sf::Texture& texture, const sf::IntRect& rect);

  private:
    sf::Sprite sprite;

    virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif
