#ifndef SOURCES_SCOUT_LOADINGSTATE_HPP_
#define SOURCES_SCOUT_LOADINGSTATE_HPP_

#include "State.hpp"
#include "LoadingTask.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

namespace sf
{
  class RenderTarget;
}

class LoadingState : public State
{
  public:
    LoadingState(StateStack& stack, Context context);
    
    virtual void draw();
    virtual bool update(sf::Time dt);
    virtual bool handleEvent(const sf::Event& event);
    
    void setCompletion(float percent);
    
  private:
    sf::RenderTarget& target;
    sf::Text loadingText;
    sf::RectangleShape progressBarBackground;
    sf::RectangleShape progressBar;
    
    LoadingTask loadingTask;
};

#endif