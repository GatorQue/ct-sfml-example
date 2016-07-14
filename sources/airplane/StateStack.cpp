#include "StateStack.hpp"
#include "Foreach.hpp"

#include <cassert>

StateStack::StateStack(State::Context context) :
  stack(),
  pendingList(),
  context(context),
  factories()
{
}

void StateStack::update(sf::Time dt)
{
  // Iterate from top to bottom, stop as soon as update() return false
  for(auto itr = stack.rbegin(); itr != stack.rend(); ++itr)
  {
    if(!(*itr)->update(dt))
      break;
  }

  applyPendingChanges();
}

void StateStack::draw()
{
  // Draw all active states from bottom to top
  FOREACH(State::Ptr& state, stack)
    state->draw();
}

void StateStack::handleEvent(const sf::Event& event)
{
  // Iterate from top to bottom, stop as soon as handleEvent() returns false
  for(auto itr = stack.rbegin(); itr != stack.rend(); ++itr)
  {
    if(!(*itr)->handleEvent(event))
      break;
  }

  applyPendingChanges();
}

void StateStack::pushState(States::ID stateID)
{
  pendingList.push_back(PendingChange(Push, stateID));
}

void StateStack::popState()
{
  pendingList.push_back(PendingChange(Pop));
}

void StateStack::clearStates()
{
  pendingList.push_back(PendingChange(Clear));
}

bool StateStack::isEmpty() const
{
  return stack.empty();
}

State::Ptr StateStack::createState(States::ID stateID)
{
  auto found = factories.find(stateID);
  assert(found != factories.end());

  return found->second();
}

void StateStack::applyPendingChanges()
{
  FOREACH(PendingChange change, pendingList)
  {
    switch(change.action)
    {
      case Push:
        stack.push_back(createState(change.stateID));
        break;
      case Pop:
        stack.back()->onDestroy();
        stack.pop_back();

        if(!stack.empty())
          stack.back()->onActivate();
        break;
      case Clear:
        FOREACH(State::Ptr& state, stack)
          state->onDestroy();

        stack.clear();
        break;
    }
  }

  pendingList.clear();
}

StateStack::PendingChange::PendingChange(Action action, States::ID stateID) :
  action(action),
  stateID(stateID)
{
}
