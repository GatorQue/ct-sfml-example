#ifndef SOURCES_SCOUT_STRINGUTILS_HPP_
#define SOURCES_SCOUT_STRINGUTILS_HPP_

#include <SFML/Window/Keyboard.hpp>

#include <sstream>

template <typename T>
std::string toString(const T& value)
{
  std::stringstream stream;
  stream << value;
  return stream.str();
}

// Convert Keyboard enumeration to string
std::string toString(sf::Keyboard::Key key);

#endif

