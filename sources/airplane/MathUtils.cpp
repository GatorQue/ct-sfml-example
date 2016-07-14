#include "MathUtils.hpp"

#include <cassert>
#include <cmath>
#include <ctime>
#include <random>

#define PI_CONST 3.141592653589793238462643383f

namespace
{
  std::default_random_engine createRandomEngine()
  {
    auto seed = static_cast<unsigned long>(std::time(nullptr));
    return std::default_random_engine(seed);
  }

  auto RandomEngine = createRandomEngine();
}

float toDegree(float radian)
{
  return 180.f / PI_CONST * radian;
}

float toRadian(float degree)
{
  return PI_CONST / 180.f * degree;
}

int randomInt(int exclusiveMax)
{
  std::uniform_int_distribution<> distr(0, exclusiveMax - 1);
  return distr(RandomEngine);
}

float length(sf::Vector2f vector)
{
  return std::sqrt(vector.x * vector.x + vector.y * vector.y);
}

sf::Vector2f unitVector(sf::Vector2f vector)
{
  assert(vector != sf::Vector2f(0.f, 0.f));
  return vector / length(vector);
}
