#ifndef SOURCES_SCOUT_NETWORKPROTOCOL_HPP_
#define SOURCES_SCOUT_NETWORKPROTOCOL_HPP_

#include <SFML/Config.hpp>
#include <SFML/System/Vector2.hpp>

const unsigned short serverPort = 5000;

namespace Server
{
  // Packets originated in the server
  enum PacketType
  {
    BroadcastMessage, // format: [Int32:packetType] [string:message]
    SpawnSelf,        // format: [Int32:packetType]
    InitialState,
    PlayerEvent,
    PlayerRealtimeChange,
    PlayerConnect,
    PlayerDisconnect,
    AcceptCoopPartner,
    SpawnEnemy,
    SpawnPickup,
    UpdateClientState,
    MissionSuccess
  };
}

namespace Client
{
  // Packets originated in the client
  enum PacketType
  {
    PlayerEvent,
    PlayerRealtimeChange,
    RequestCoopPartner,
    PositionUpdate,
    GameEvent,
    Quit
  };
}

namespace GameActions
{
  enum Type
  {
    EnemyExplode,
  };

  struct Action
  {
    Action()
    {
      // leave unitialized
    }

    Action(Type type, sf::Vector2f position) :
      type(type),
      position(position)
    {
    }

    Type type;
    sf::Vector2f position;
  };
}

#endif
