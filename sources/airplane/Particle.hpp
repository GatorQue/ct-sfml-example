#ifndef SOURCES_SCOUT_PARTICLE_HPP_
#define SOURCES_SCOUT_PARTICLE_HPP_

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

struct Particle
{
  enum Type
  {
    Propellant,
    Smoke,
    ParticleCount
  };

  sf::Vector2f position;
  sf::Color color;
  sf::Time lifetime;
};

#endif
