#include "Projectile.hpp"
#include "DataTables.hpp"
#include "EmitterNode.hpp"
#include "ResourceHolder.hpp"
#include "MathUtils.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>
#include <cmath>

namespace
{
  const std::vector<ProjectileData> Table = initializeProjectileData();
}

Projectile::Projectile(Type type, const TextureHolder& textures) :
  Entity(1),
  type(type),
  sprite(textures.get(Table[type].texture), Table[type].textureRect),
  targetDirection()
{
  centerOrigin(sprite);

  // Add particle system for missiles
  if(isGuided())
  {
    std::unique_ptr<EmitterNode> smoke(new EmitterNode(Particle::Smoke));
    smoke->setPosition(0.f, getBoundingRect().height / 2.f);
    attachChild(std::move(smoke));

    std::unique_ptr<EmitterNode> propellant(new EmitterNode(Particle::Propellant));
    propellant->setPosition(0.f, getBoundingRect().height / 2.f);
    attachChild(std::move(propellant));
  }
}

void Projectile::guideTowards(sf::Vector2f position)
{
  assert(isGuided());
  targetDirection = unitVector(position - getWorldPosition());
}

bool Projectile::isGuided() const
{
  return type == Missile;
}

void Projectile::updateCurrent(sf::Time dt, CommandQueue& commands)
{
  if(isGuided())
  {
    const float approachRate = 200.f;

    sf::Vector2f newVelocity = unitVector(approachRate *
        dt.asSeconds() * targetDirection + getVelocity());

    newVelocity *= getMaxSpeed();
    float angle = std::atan2(newVelocity.y, newVelocity.x);

    setRotation(toDegree(angle) + 90.f);
    setVelocity(newVelocity);
  }

  Entity::updateCurrent(dt, commands);
}

void Projectile::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
  target.draw(sprite, states);
}

unsigned int Projectile::getCategory() const
{
  if(type == EnemyBullet)
    return Category::EnemyProjectile;
  else
    return Category::AlliedProjectile;
}

sf::FloatRect Projectile::getBoundingRect() const
{
  return getWorldTransform().transformRect(sprite.getGlobalBounds());
}

float Projectile::getMaxSpeed() const
{
  return Table[type].speed;
}

int Projectile::getDamage() const
{
  return Table[type].damage;
}

