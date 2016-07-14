#ifndef SOURCES_SCOUT_ENTITY_HPP_
#define SOURCES_SCOUT_ENTITY_HPP_

#include "SceneNode.hpp"

class Entity : public SceneNode
{
  public:
    explicit Entity(int hitPoints);

    void setVelocity(sf::Vector2f velocity);
    void setVelocity(float vx, float vy);
    void accelerate(sf::Vector2f velocity);
    void accelerate(float vx, float vy);
    sf::Vector2f getVelocity() const;

    int getHitpoints() const;
    void setHitpoints(int points);
    void repair(int points);
    void damage(int points);
    void destroy();
    virtual void remove();
    virtual bool isDestroyed() const;

  protected:
    virtual void updateCurrent(sf::Time dt, CommandQueue& commands);

  private:
    int hitPoints;
    sf::Vector2f velocity;

};

#endif
