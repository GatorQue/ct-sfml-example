#ifndef SOURCES_SCOUT_KEYBINDING_HPP_
#define SOURCES_SCOUT_KEYBINDING_HPP_

#include <SFML/Window/Keyboard.hpp>

#include <map>
#include <vector>

namespace PlayerActions
{
  enum Action
  {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Fire,
    LaunchMissile,
    ActionCount
  };
}

class KeyBinding
{
  public:
    explicit KeyBinding(int controlPreconfiguration);

    void assignKey(PlayerActions::Action action, sf::Keyboard::Key key);
    sf::Keyboard::Key getAssignedKey(PlayerActions::Action action) const;

    bool checkAction(sf::Keyboard::Key key, PlayerActions::Action& out) const;
    std::vector<PlayerActions::Action> getRealtimeActions() const;

  private:
    std::map<sf::Keyboard::Key, PlayerActions::Action> keyMap;

    void initializeActions();
};

bool isRealtimeAction(PlayerActions::Action action);

#endif
