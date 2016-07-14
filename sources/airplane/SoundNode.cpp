#include "SoundNode.hpp"
#include "SoundPlayer.hpp"

SoundNode::SoundNode(SoundPlayer& sounds) :
  SceneNode(),
  sounds(sounds)
{
}

void SoundNode::playSound(SoundEffect::ID sound, sf::Vector2f position)
{
  sounds.play(sound, position);
}

unsigned int SoundNode::getCategory() const
{
  return Category::SoundEffect;
}

