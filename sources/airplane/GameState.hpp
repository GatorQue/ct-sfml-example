#ifndef SOURCES_SCOUT_GAMESTATE_HPP_
#define SOURCES_SCOUT_GAMESTATE_HPP_

#include "State.hpp"
#include "Player.hpp"
#include "World.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

class GameState : public State
{
  public:
    GameState(StateStack& stack, Context context);

    virtual void draw();
    virtual bool update(sf::Time dt);
    virtual bool handleEvent(const sf::Event& event);

  private:
    World world;
    Player player;
};

#endif

