#ifndef SOURCES_SCOUT_PAUSESTATE_HPP_
#define SOURCES_SCOUT_PAUSESTATE_HPP_

#include "State.hpp"
#include "Container.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

class PauseState : public State
{
  public:
    PauseState(StateStack& stack, Context context, bool letUpdatesThrough = false);
    virtual ~PauseState();

    virtual void draw();
    virtual bool update(sf::Time dt);
    virtual bool handleEvent(const sf::Event& event);

  private:
    sf::Sprite backgroundSprite;
    sf::Text pausedText;
    GUI::Container guiContainer;
    bool letUpdatesThrough;
};

#endif
