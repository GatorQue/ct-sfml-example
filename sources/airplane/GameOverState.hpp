#ifndef SOURCES_SCOUT_GAMEOVERSTATE_HPP_
#define SOURCES_SCOUT_GAMEOVERSTATE_HPP_

#include "State.hpp"
#include "Container.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

class GameOverState : public State
{
  public:
    GameOverState(StateStack& stack, Context context, const std::string& text);

    virtual void draw();
    virtual bool update(sf::Time dt);
    virtual bool handleEvent(const sf::Event& event);

  private:
    sf::Text gameOverText;
    sf::Time elapsedTime;
};

#endif
