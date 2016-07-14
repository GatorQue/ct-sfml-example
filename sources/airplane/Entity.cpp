#include "Entity.hpp"

#include <cassert>

Entity::Entity(int hitPoints) :
  hitPoints(hitPoints),
  velocity()
{
}

void Entity::setVelocity(sf::Vector2f velocity)
{
  this->velocity = velocity;
}

void Entity::setVelocity(float vx, float vy)
{
  velocity.x = vx;
  velocity.y = vy;
}

sf::Vector2f Entity::getVelocity() const
{
  return velocity;
}

void Entity::accelerate(sf::Vector2f velocity)
{
  this->velocity += velocity;
}

void Entity::accelerate(float vx, float vy)
{
  velocity.x += vx;
  velocity.y += vy;
}

int Entity::getHitpoints() const
{
  return hitPoints;
}

void Entity::setHitpoints(int points)
{
  assert(points > 0);

  hitPoints = points;
}

void Entity::repair(int points)
{
  assert(points > 0);

  hitPoints += points;
}

void Entity::damage(int points)
{
  assert(points > 0);

  hitPoints -= points;
}

void Entity::destroy()
{
  hitPoints = 0;
}

void Entity::remove()
{
  destroy();
}

bool Entity::isDestroyed() const
{
  return hitPoints <= 0;
}

void Entity::updateCurrent(sf::Time dt, CommandQueue& command)
{
  move(velocity * dt.asSeconds());
}
