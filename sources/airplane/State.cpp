#include "State.hpp"
#include "StateStack.hpp"

State::Context::Context(sf::RenderTarget& target,
    FontHolder& fonts,
    MusicPlayer& music,
    SoundPlayer& sounds,
    TextureHolder& textures,
    KeyBinding& keys1,
    KeyBinding& keys2) :
  target(&target),
  fonts(&fonts),
  music(&music),
  sounds(&sounds),
  textures(&textures),
  keys1(&keys1),
  keys2(&keys2)
{
}

State::State(StateStack& stack, Context context) :
  stack(&stack),
  context(context)
{
}

State::~State()
{
}

void State::requestStackPush(States::ID stateID)
{
  stack->pushState(stateID);
}

void State::requestStackPop()
{
  stack->popState();
}

void State::requestStateClear()
{
  stack->clearStates();
}

State::Context State::getContext() const
{
  return context;
}

void State::onActivate()
{
  // do nothing by default
}

void State::onDestroy()
{
  // do nothing by default
}
