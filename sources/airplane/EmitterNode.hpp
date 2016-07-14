#ifndef SOURCES_SCOUT_EMITTERNODE_HPP_
#define SOURCES_SCOUT_EMITTERNODE_HPP_

#include "SceneNode.hpp"
#include "Particle.hpp"

class ParticleNode;

class EmitterNode : public SceneNode
{
  public:
    explicit EmitterNode(Particle::Type type);

  private:
    sf::Time accumulatedTime;
    Particle::Type type;
    ParticleNode* particleSystem;

    virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
    void emitParticles(sf::Time dt);
};

#endif
