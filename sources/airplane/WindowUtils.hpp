#ifndef SOURCES_SCOUT_WINDOWUTILS_HPP_
#define SOURCES_SCOUT_WINDOWUTILS_HPP_

namespace sf
{
  class Sprite;
  class Text;
}

class Animation;

void centerOrigin(sf::Sprite& sprite);
void centerOrigin(sf::Text& text);
void centerOrigin(Animation& animation);

#endif
