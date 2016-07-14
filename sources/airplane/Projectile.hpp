#ifndef SOURCES_SCOUT_PROJECTILE_HPP_
#define SOURCES_SCOUT_PROJECTILE_HPP_

#include "Entity.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>

class Projectile : public Entity
{
  public:
    enum Type
    {
      AlliedBullet,
      EnemyBullet,
      Missile,
      TypeCount
    };

    Projectile(Type type, const TextureHolder& textures);

    bool isGuided() const;
    void guideTowards(sf::Vector2f position);

    virtual unsigned int getCategory() const;
    virtual sf::FloatRect getBoundingRect() const;
    float getMaxSpeed() const;
    int getDamage() const;

  private:
    Type type;
    sf::Sprite sprite;
    sf::Vector2f targetDirection;

    virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
    virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
};

#endif

