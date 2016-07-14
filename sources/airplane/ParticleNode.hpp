#ifndef SOURCES_SCOUT_PARTICLENODE_HPP_
#define SOURCES_SCOUT_PARTICLENODE_HPP_

#include "SceneNode.hpp"
#include "Particle.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/VertexArray.hpp>

#include <deque>

class ParticleNode : public SceneNode
{
  public:
    ParticleNode(Particle::Type type, const TextureHolder& textures);

    void addParticle(sf::Vector2f position);
    Particle::Type getParticleType() const;
    virtual unsigned int getCategory() const;

  private:
    std::deque<Particle> particles;
    const sf::Texture& texture;
    Particle::Type type;

    mutable sf::VertexArray vertexArray;
    mutable bool needsVertexUpdate;

    virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
    virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

    void addVertex(float worldX, float worldY, float texCoordX,
        float texCoordY, const sf::Color& color) const;
    void computeVertices() const;
};

#endif
