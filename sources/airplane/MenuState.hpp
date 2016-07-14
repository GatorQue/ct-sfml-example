#ifndef SOURCES_SCOUT_MENUSTATE_HPP_
#define SOURCES_SCOUT_MENUSTATE_HPP_

#include "State.hpp"
#include "Container.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <vector>

class MenuState : public State
{
  public:
    MenuState(StateStack& stack, Context context);

    virtual void draw();
    virtual bool update(sf::Time dt);
    virtual bool handleEvent(const sf::Event& event);

  private:
    sf::Sprite backgroundSprite;
    GUI::Container guiContainer;
};

#endif
