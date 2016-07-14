#ifndef SOURCES_SCOUT_MATHUTILS_HPP_
#define SOURCES_SCOUT_MATHUTILS_HPP_

#include <SFML/System/Vector2.hpp>

// Degree/radian conversion
float toDegree(float radian);
float toRadian(float degree);

// Random number generation
int randomInt(int exclusiveMax);

// Vector operations
float length(sf::Vector2f vector);
sf::Vector2f unitVector(sf::Vector2f vector);

#endif
