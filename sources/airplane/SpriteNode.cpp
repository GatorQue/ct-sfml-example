#include "SpriteNode.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

SpriteNode::SpriteNode(const sf::Texture& texture) :
  sprite(texture)
{
}

SpriteNode::SpriteNode(const sf::Texture& texture, const sf::IntRect& rect) :
  sprite(texture, rect)
{
}

void SpriteNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
  target.draw(sprite, states);
}
