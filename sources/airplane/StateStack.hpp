#ifndef SOURCES_SCOUT_STATESTACK_HPP_
#define SOURCES_SCOUT_STATESTACK_HPP_

#include "State.hpp"
#include "StateIdentifiers.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>

#include <functional>
#include <map>
#include <utility>
#include <vector>

// Forward declare SFML classes
namespace sf
{
  class Event;
  class RenderWindow;
}

class StateStack : private sf::NonCopyable
{
  public:
    enum Action
    {
      Push,
      Pop,
      Clear,
    };

    explicit StateStack(State::Context context);

    template <typename T>
    void registerState(States::ID stateID);
    template <typename T, typename Param1>
    void registerState(States::ID stateID, Param1 arg1);

    void update(sf::Time dt);
    void draw();
    void handleEvent(const sf::Event& event);

    void pushState(States::ID stateID);
    void popState();
    void clearStates();

    bool isEmpty() const;

  private:
    struct PendingChange
    {
      explicit PendingChange(Action action, States::ID stateID = States::None);

      Action action;
      States::ID stateID;
    };

    std::vector<State::Ptr> stack;
    std::vector<PendingChange> pendingList;

    State::Context context;
    std::map<States::ID, std::function<State::Ptr()> > factories;

    State::Ptr createState(States::ID stateID);
    void applyPendingChanges();
};

template <typename T>
void StateStack::registerState(States::ID stateID)
{
  factories[stateID] = [this] ()
  {
    return State::Ptr(new T(*this, context));
  };
}

template <typename T, typename Param1>
void StateStack::registerState(States::ID stateID, Param1 arg1)
{
  factories[stateID] = [this, arg1] ()
  {
    return State::Ptr(new T(*this, context, arg1));
  };
}

#endif
