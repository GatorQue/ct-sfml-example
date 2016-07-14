#include "Pickup.hpp"
#include "Category.hpp"
#include "CommandQueue.hpp"
#include "DataTables.hpp"
#include "ResourceHolder.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
  const std::vector<PickupData> Table = initializePickupData();
}

Pickup::Pickup(Type type, const TextureHolder& textures) :
  Entity(1),
  type(type),
  sprite(textures.get(Table[type].texture), Table[type].textureRect)
{
  centerOrigin(sprite);
}

unsigned int Pickup::getCategory() const
{
  return Category::Pickup;
}

sf::FloatRect Pickup::getBoundingRect() const
{
  return getWorldTransform().transformRect(sprite.getGlobalBounds());
}

void Pickup::apply(Aircraft& player) const
{
  Table[type].action(player);
}

void Pickup::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
  target.draw(sprite, states);
}

