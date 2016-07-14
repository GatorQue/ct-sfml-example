#include "Player.hpp"
#include "Aircraft.hpp"
#include "CommandQueue.hpp"
#include "Foreach.hpp"
#include "NetworkProtocol.hpp"

#include <SFML/Network/Packet.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <algorithm>
#include <map>
#include <string>

struct AircraftMover
{
  AircraftMover(float vx, float vy, int identifier) :
    velocity(vx, vy),
    aircraftID(identifier)
  {
  }

  void operator() (Aircraft& aircraft, sf::Time) const
  {
    if(aircraft.getIdentifier() == aircraftID)
      aircraft.accelerate(velocity * aircraft.getMaxSpeed());
  }

  sf::Vector2f velocity;
  int aircraftID;
};

struct AircraftFireTrigger
{
  AircraftFireTrigger(int identifier) :
    aircraftID(identifier)
  {
  }

  void operator() (Aircraft& aircraft, sf::Time) const
  {
    if(aircraft.getIdentifier() == aircraftID)
      aircraft.fire();
  }

  int aircraftID;
};

struct AircraftMissileTrigger
{
  AircraftMissileTrigger(int identifier) :
    aircraftID(identifier)
  {
  }

  void operator() (Aircraft& aircraft, sf::Time) const
  {
    if(aircraft.getIdentifier() == aircraftID)
      aircraft.launchMissile();
  }

  int aircraftID;
};

Player::Player(sf::TcpSocket* socket, sf::Int32 identifier, const KeyBinding* binding) :
  keyBinding(binding),
  actionBinding(),
  actionProxies(),
  currentMissionStatus(MissionRunning),
  identifier(identifier),
  socket(socket)
{
  // Set initial action bindings
  initializeActions();

  // Assign all categories to player's aircraft
  FOREACH(auto& pair, actionBinding)
    pair.second.category = Category::PlayerAircraft;
}

void Player::handleEvent(const sf::Event& event, CommandQueue& commands)
{
  // Event
  if(event.type == sf::Event::KeyPressed)
  {
    PlayerActions::Action action;
    if(keyBinding &&
       keyBinding->checkAction(event.key.code, action) &&
       !isRealtimeAction(action))
    {
      // Network connected -> send event over network
      if(socket)
      {
        sf::Packet packet;
        packet << static_cast<sf::Int32>(Client::PlayerEvent);
        packet << identifier;
        packet << static_cast<sf::Int32>(action);
        socket->send(packet);
      }
      // Network disconnected -> local event
      else
      {
        commands.push(actionBinding[action]);
      }
    }
  }

  // Realtime change (network connected)
  if (socket &&
      (event.type == sf::Event::KeyPressed ||
       event.type == sf::Event::KeyReleased))
  {
    PlayerActions::Action action;
    if(keyBinding &&
       keyBinding->checkAction(event.key.code, action) &&
       isRealtimeAction(action))
    {
      // Send realtime change over network
      sf::Packet packet;
      packet << static_cast<sf::Int32>(Client::PlayerRealtimeChange);
      packet << identifier;
      packet << static_cast<sf::Int32>(action);
      packet << (event.type == sf::Event::KeyPressed);
      socket->send(packet);
    }
  }
}

bool Player::isLocal() const
{
  // No key binding means this player is remote
  return keyBinding != nullptr;
}

void Player::disableAllRealtimeActions()
{
  FOREACH(auto& action, actionProxies)
  {
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Client::PlayerRealtimeChange);
    packet << identifier;
    packet << static_cast<sf::Int32>(action.first);
    packet << false;
    socket->send(packet);
  }
}

void Player::handleRealtimeInput(CommandQueue& commands)
{
  // Check if this is a networked game and local player or just a single
  // player game
  if(!socket || (socket && isLocal()))
  {
    // Lookup all actions and push corresponding commands to queue
    std::vector<PlayerActions::Action> activeActions =
      keyBinding->getRealtimeActions();
    FOREACH(PlayerActions::Action action, activeActions)
      commands.push(actionBinding[action]);
  }
}

void Player::handleRealtimeNetworkInput(CommandQueue& commands)
{
  if(socket && !isLocal())
  {
    // Traverse all realtime input proxies. Because this is a networked game,
    // the input isn't handled directly
    FOREACH(auto pair, actionProxies)
    {
      if(pair.second && isRealtimeAction(pair.first))
        commands.push(actionBinding[pair.first]);
    }
  }
}

void Player::handleNetworkEvent(PlayerActions::Action action, CommandQueue& commands)
{
  commands.push(actionBinding[action]);
}

void Player::handleNetworkRealtimeChange(PlayerActions::Action action, bool actionEnabled)
{
  actionProxies[action] = actionEnabled;
}

void Player::setMissionStatus(MissionStatus status)
{
  currentMissionStatus = status;
}

Player::MissionStatus Player::getMissionStatus() const
{
  return currentMissionStatus;
}

void Player::initializeActions()
{
  actionBinding[PlayerActions::MoveLeft].action =
    derivedAction<Aircraft>(AircraftMover(-1, 0, identifier));
  actionBinding[PlayerActions::MoveRight].action =
    derivedAction<Aircraft>(AircraftMover(1, 0, identifier));
  actionBinding[PlayerActions::MoveUp].action =
    derivedAction<Aircraft>(AircraftMover(0, -1, identifier));
  actionBinding[PlayerActions::MoveDown].action =
    derivedAction<Aircraft>(AircraftMover(0, 1, identifier));
  actionBinding[PlayerActions::Fire].action =
    derivedAction<Aircraft>(AircraftFireTrigger(identifier));
  actionBinding[PlayerActions::LaunchMissile].action =
    derivedAction<Aircraft>(AircraftMissileTrigger(identifier));
}
