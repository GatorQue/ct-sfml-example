#include "KeyBinding.hpp"
#include "Foreach.hpp"

#include <algorithm>
#include <string>

KeyBinding::KeyBinding(int controlPreconfiguration) :
  keyMap()
{
  // Set initial key bindings for player 1
  if(controlPreconfiguration == 1)
  {
    keyMap[sf::Keyboard::Left] = PlayerActions::MoveLeft;
    keyMap[sf::Keyboard::Right] = PlayerActions::MoveRight;
    keyMap[sf::Keyboard::Up] = PlayerActions::MoveUp;
    keyMap[sf::Keyboard::Down] = PlayerActions::MoveDown;
    keyMap[sf::Keyboard::Space] = PlayerActions::Fire;
    keyMap[sf::Keyboard::M] = PlayerActions::LaunchMissile;
  }
  else if(controlPreconfiguration == 2)
  {
    keyMap[sf::Keyboard::A] = PlayerActions::MoveLeft;
    keyMap[sf::Keyboard::D] = PlayerActions::MoveRight;
    keyMap[sf::Keyboard::W] = PlayerActions::MoveUp;
    keyMap[sf::Keyboard::S] = PlayerActions::MoveDown;
    keyMap[sf::Keyboard::F] = PlayerActions::Fire;
    keyMap[sf::Keyboard::R] = PlayerActions::LaunchMissile;
  }
}

void KeyBinding::assignKey(PlayerActions::Action action, sf::Keyboard::Key key)
{
  // Remove all keys that already map to action
  for(auto itr = keyMap.begin(); itr != keyMap.end();)
  {
    if(itr->second == action)
      keyMap.erase(itr++);
    else
      ++itr;
  }

  // Insert new binding
  keyMap[key] = action;
}

sf::Keyboard::Key KeyBinding::getAssignedKey(PlayerActions::Action action) const
{
  FOREACH(auto pair, keyMap)
  {
    if(pair.second == action)
      return pair.first;
  }

  return sf::Keyboard::Unknown;
}

bool KeyBinding::checkAction(sf::Keyboard::Key key, PlayerActions::Action& out) const
{
  auto found = keyMap.find(key);
  if(found == keyMap.end())
  {
    return false;
  }
  else
  {
    out = found->second;
    return true;
  }
}

std::vector<PlayerActions::Action> KeyBinding::getRealtimeActions() const
{
  // Return all realtime actions that are currently active
  std::vector<PlayerActions::Action> actions;

  FOREACH(auto pair, keyMap)
  {
    // If key is pressed and an action is a realtime action, store it
    if(sf::Keyboard::isKeyPressed(pair.first) && isRealtimeAction(pair.second))
      actions.push_back(pair.second);
  }

  return actions;
}

bool isRealtimeAction(PlayerActions::Action action)
{
  switch(action)
  {
    case PlayerActions::MoveLeft:
    case PlayerActions::MoveRight:
    case PlayerActions::MoveUp:
    case PlayerActions::MoveDown:
    case PlayerActions::Fire:
      return true;

    default:
      return false;
  }
}
