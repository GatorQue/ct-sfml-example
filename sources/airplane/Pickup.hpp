#ifndef SOURCES_SCOUT_PICKUP_HPP_
#define SOURCES_SCOUT_PICKUP_HPP_

#include "Entity.hpp"
#include "Command.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>

class Aircraft;

class Pickup : public Entity
{
  public:
    enum Type
    {
      HealthRefill,
      MissileRefill,
      FireSpread,
      FireRate,
      TypeCount
    };

    Pickup(Type type, const TextureHolder& textures);

    virtual unsigned int getCategory() const;
    virtual sf::FloatRect getBoundingRect() const;

    void apply(Aircraft& player) const;

  protected:
    virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

  private:
    Type type;
    sf::Sprite sprite;
};

#endif
