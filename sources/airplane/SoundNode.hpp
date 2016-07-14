#ifndef SOURCES_SCOUT_SOUNDNODE_HPP_
#define SOURCES_SCOUT_SOUNDNODE_HPP_

#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"

class SoundPlayer;

class SoundNode : public SceneNode
{
  public:
    explicit SoundNode(SoundPlayer& sounds);

    void playSound(SoundEffect::ID sound, sf::Vector2f position);

    virtual unsigned int getCategory() const;

  private:
    SoundPlayer& sounds;
};

#endif
