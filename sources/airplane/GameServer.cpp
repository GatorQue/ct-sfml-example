#include "GameServer.hpp"
#include "Aircraft.hpp"
#include "Foreach.hpp"
#include "MathUtils.hpp"
#include "NetworkProtocol.hpp"
#include "Pickup.hpp"

#include <SFML/Network/Packet.hpp>

GameServer::RemotePeer::RemotePeer() :
  ready(false),
  timedOut(false)
{
  socket.setBlocking(false);
}

GameServer::GameServer(sf::Vector2f battlefieldSize) :
    thread(&GameServer::executionThread, this),
    clock(),
    listenerSocket(),
    listeningState(false),
    clientTimeoutTime(sf::seconds(3.f)),
    maxConnectedPlayers(10),
    connectedPlayers(0),
    worldHeight(5000.f),
    battleFieldRect(0.f, worldHeight - battlefieldSize.y, battlefieldSize.x, battlefieldSize.y),
    battleFieldScrollSpeed(-50.f),
    aircraftCount(0),
    aircraftInfo(),
    peers(1),
    aircraftIdentifierCounter(1),
    waitingThreadEnd(false),
    lastSpawnTime(sf::Time::Zero),
    timeForNextSpawn(sf::seconds(5.f))
{
  listenerSocket.setBlocking(false);
  peers[0].reset(new RemotePeer());
  thread.launch();
}

GameServer::~GameServer()
{
  waitingThreadEnd = true;
  thread.wait();
}

void GameServer::notifyPlayerRealtimeChange(sf::Int32 aircraftIdentifier, sf::Int32 action, bool actionEnabled)
{
  for(std::size_t i=0; i<connectedPlayers; ++i)
  {
    if(peers[i]->ready)
    {
      sf::Packet packet;
      packet << static_cast<sf::Int32>(Server::PlayerRealtimeChange);
      packet << aircraftIdentifier;
      packet << action;
      packet << actionEnabled;

      peers[i]->socket.send(packet);
    }
  }
}

void GameServer::notifyPlayerEvent(sf::Int32 aircraftIdentifier, sf::Int32 action)
{
  for(std::size_t i=0; i<connectedPlayers; ++i)
  {
    if(peers[i]->ready)
    {
      sf::Packet packet;
      packet << static_cast<sf::Int32>(Server::PlayerEvent);
      packet << aircraftIdentifier;
      packet << action;

      peers[i]->socket.send(packet);
    }
  }
}

void GameServer::notifyPlayerSpawn(sf::Int32 aircraftIdentifier)
{
  for(std::size_t i=0; i<connectedPlayers; ++i)
  {
    if(peers[i]->ready)
    {
      sf::Packet packet;
      packet << static_cast<sf::Int32>(Server::PlayerConnect);
      packet << aircraftIdentifier;
      packet << aircraftInfo[aircraftIdentifier].position.x;
      packet << aircraftInfo[aircraftIdentifier].position.y;

      peers[i]->socket.send(packet);
    }
  }
}

void GameServer::setListening(bool enable)
{
  // Check if it isn't already listening
  if(enable)
  {
    if(!listeningState)
      listeningState = (listenerSocket.listen(serverPort) == sf::TcpListener::Done);
  }
  else
  {
    listenerSocket.close();
    listeningState = false;
  }
}

void GameServer::executionThread()
{
  setListening(true);

  sf::Time stepInterval = sf::seconds(1.f / 60.f);
  sf::Time stepTime = sf::Time::Zero;
  sf::Time tickInterval = sf::seconds(1.f / 20.f);
  sf::Time tickTime = sf::Time::Zero;
  sf::Clock stepClock, tickClock;

  while(!waitingThreadEnd)
  {
    handleIncomingPackets();
    handleIncomingConnections();

    stepTime += stepClock.getElapsedTime();
    stepClock.restart();

    tickTime += tickClock.getElapsedTime();
    tickClock.restart();

    // Fixed update step
    while(stepTime >= stepInterval)
    {
      battleFieldRect.top += battleFieldScrollSpeed * stepInterval.asSeconds();
      stepTime -= stepInterval;
    }

    // Fixed tick step
    while(tickTime >= tickInterval)
    {
      tick();
      tickTime -= tickInterval;
    }

    // Sleep to prevent server from consuming 100% CPU
    sf::sleep(sf::milliseconds(100));
  }
}

void GameServer::tick()
{
  updateClientState();

  // Check for mission success = all planes with position.y < offset
  bool allAircraftsDone = true;
  FOREACH(auto pair, aircraftInfo)
  {
    // As long as one player has not crossed the finish line yet, set variable
    // to false
    if(pair.second.position.y > 0.f)
      allAircraftsDone = false;
  }
  if(allAircraftsDone && aircraftInfo.size() > 0)
  {
    sf::Packet missionSuccessPacket;
    missionSuccessPacket << static_cast<sf::Int32>(Server::MissionSuccess);
    sendToAll(missionSuccessPacket);
  }

  // Remove ID's of aircraft that have been destroyed (relevant if a client
  // has two, and loses one)
  for(auto itr = aircraftInfo.begin(); itr != aircraftInfo.end();)
  {
    if(itr->second.hitpoints <= 0)
      aircraftInfo.erase(itr++);
    else
      ++itr;
  }

  // Check if its time to attempt to spawn enemies
  if(now() >= timeForNextSpawn + lastSpawnTime)
  {
    // No more enemies are spawned near the end
    if(battleFieldRect.top > 600.f)
    {
      std::size_t enemyCount = 1u + randomInt(2);
      float spawnCenter = static_cast<float>(randomInt(500) - 250);

      // In case only one enemy is being spawned, it appears directly at the
      // spawnCenter
      float planeDistance = 0.f;
      float nextSpawnPosition = spawnCenter;

      // In case there are two enemies being spawned together, each is spawned
      // at each side of the spawnCenter, with a minimum distance
      if(enemyCount == 2)
      {
        planeDistance = static_cast<float>(150 + randomInt(250));
        nextSpawnPosition = spawnCenter - planeDistance / 2.f;
      }

      // Send the spawn orders to all clients
      for(std::size_t i=0; i<enemyCount; ++i)
      {
        sf::Packet packet;
        packet << static_cast<sf::Int32>(Server::SpawnEnemy);
        packet << static_cast<sf::Int32>(1 + randomInt(Aircraft::TypeCount - 1));
        packet << worldHeight - battleFieldRect.top + 500;
        packet << nextSpawnPosition;

        nextSpawnPosition += planeDistance / 2.f;

        sendToAll(packet);
      }

      lastSpawnTime = now();
      timeForNextSpawn = sf::milliseconds(2000 + randomInt(6000));
    }
  }
}

sf::Time GameServer::now() const
{
  return clock.getElapsedTime();
}

void GameServer::handleIncomingPackets()
{
  bool detectedTimeout = false;

  FOREACH(PeerPtr& peer, peers)
  {
    if(peer->ready)
    {
      sf::Packet packet;
      while(peer->socket.receive(packet) == sf::Socket::Done)
      {
        // Interpret packet and react to it
        handleIncomingPacket(packet, *peer, detectedTimeout);

        // Packet was indeed received, update the ping timer
        peer->lastPacketTime = now();
        packet.clear();
      }

      if(now() >= peer->lastPacketTime + clientTimeoutTime)
      {
        peer->timedOut = true;
        detectedTimeout = true;
      }
    }
  }

  if(detectedTimeout)
    handleDisconnections();
}

void GameServer::handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout)
{
  sf::Int32 packetType;
  packet >> packetType;

  switch (packetType)
  {
    case Client::Quit:
      {
        receivingPeer.timedOut = true;
        detectedTimeout = true;
      }
      break;

    case Client::PlayerEvent:
      {
        sf::Int32 aircraftIdentifier;
        sf::Int32 action;
        packet >> aircraftIdentifier >> action;

        notifyPlayerEvent(aircraftIdentifier, action);
      }
      break;

    case Client::PlayerRealtimeChange:
      {
        sf::Int32 aircraftIdentifier;
        sf::Int32 action;
        bool actionEnabled;
        packet >> aircraftIdentifier >> action >> actionEnabled;
        aircraftInfo[aircraftIdentifier].realtimeActions[action] = actionEnabled;
        notifyPlayerRealtimeChange(aircraftIdentifier, action, actionEnabled);
      }
      break;

    case Client::RequestCoopPartner:
      {
        receivingPeer.aircraftIdentifiers.push_back(aircraftIdentifierCounter);
        aircraftInfo[aircraftIdentifierCounter].position = sf::Vector2f(
            battleFieldRect.width / 2,
            battleFieldRect.top + battleFieldRect.height / 2);
        aircraftInfo[aircraftIdentifierCounter].hitpoints = 100;
        aircraftInfo[aircraftIdentifierCounter].missileAmmo = 2;

        sf::Packet requestPacket;
        requestPacket << static_cast<sf::Int32>(Server::AcceptCoopPartner);
        requestPacket << aircraftIdentifierCounter;
        requestPacket << aircraftInfo[aircraftIdentifierCounter].position.x;
        requestPacket << aircraftInfo[aircraftIdentifierCounter].position.y;

        receivingPeer.socket.send(requestPacket);
        aircraftCount++;

        // Inform every other peer about this new plane
        FOREACH(PeerPtr& peer, peers)
        {
          if (peer.get() != &receivingPeer && peer->ready)
          {
            sf::Packet notifyPacket;
            notifyPacket << static_cast<sf::Int32>(Server::PlayerConnect);
            notifyPacket << aircraftIdentifierCounter;
            notifyPacket << aircraftInfo[aircraftIdentifierCounter].position.x;
            notifyPacket << aircraftInfo[aircraftIdentifierCounter].position.y;
            peer->socket.send(notifyPacket);
          }
        }
        aircraftIdentifierCounter++;
      }
      break;

    case Client::PositionUpdate:
      {
        sf::Int32 numAircrafts;
        packet >> numAircrafts;

        for (sf::Int32 i = 0; i < numAircrafts; ++i)
        {
          sf::Int32 aircraftIdentifier;
          sf::Int32 aircraftHitpoints;
          sf::Int32 missileAmmo;
          sf::Vector2f aircraftPosition;
          packet >> aircraftIdentifier;
          packet >> aircraftPosition.x;
          packet >> aircraftPosition.y;
          packet >> aircraftHitpoints;
          packet >> missileAmmo;
          aircraftInfo[aircraftIdentifier].position = aircraftPosition;
          aircraftInfo[aircraftIdentifier].hitpoints = aircraftHitpoints;
          aircraftInfo[aircraftIdentifier].missileAmmo = missileAmmo;
        }
      }
      break;

    case Client::GameEvent:
      {
        sf::Int32 action;
        float x;
        float y;

        packet >> action;
        packet >> x;
        packet >> y;

        // Enemy explodes: With certain probability, drop pickup
        // To avoid multiple messages spawning multiple pickups, only listen to
        // first peer (host)
        if (action == GameActions::EnemyExplode &&
            randomInt(3) == 0 &&
            &receivingPeer == peers[0].get())
        {
          sf::Packet packet;
          packet << static_cast<sf::Int32>(Server::SpawnPickup);
          packet << static_cast<sf::Int32>(randomInt(Pickup::TypeCount));
          packet << x;
          packet << y;

          sendToAll(packet);
        }
      }
      break;
  }
}

void GameServer::updateClientState()
{
  sf::Packet updateClientStatePacket;
  updateClientStatePacket << static_cast<sf::Int32>(Server::UpdateClientState);
  updateClientStatePacket << static_cast<float>(battleFieldRect.top + battleFieldRect.height);
  updateClientStatePacket << static_cast<sf::Int32>(aircraftInfo.size());

  FOREACH(auto aircraft, aircraftInfo)
  {
    updateClientStatePacket << aircraft.first;
    updateClientStatePacket << aircraft.second.position.x;
    updateClientStatePacket << aircraft.second.position.y;
  }

  sendToAll(updateClientStatePacket);
}

void GameServer::handleIncomingConnections()
{
  if(!listeningState)
    return;

  if(listenerSocket.accept(peers[connectedPlayers]->socket) == sf::TcpListener::Done)
  {
    // order the new client to spawn its own plane ( player 1 )
    aircraftInfo[aircraftIdentifierCounter].position = sf::Vector2f(battleFieldRect.width / 2, battleFieldRect.top + battleFieldRect.height / 2);
    aircraftInfo[aircraftIdentifierCounter].hitpoints = 100;
    aircraftInfo[aircraftIdentifierCounter].missileAmmo = 2;

    sf::Packet packet;
    packet << static_cast<sf::Int32>(Server::SpawnSelf);
    packet << aircraftIdentifierCounter;
    packet << aircraftInfo[aircraftIdentifierCounter].position.x;
    packet << aircraftInfo[aircraftIdentifierCounter].position.y;

    peers[connectedPlayers]->aircraftIdentifiers.push_back(aircraftIdentifierCounter);

    broadcastMessage("New player!");
    informWorldState(peers[connectedPlayers]->socket);
    notifyPlayerSpawn(aircraftIdentifierCounter++);

    peers[connectedPlayers]->socket.send(packet);
    peers[connectedPlayers]->ready = true;
    peers[connectedPlayers]->lastPacketTime = now(); // prevent initial timeouts
    aircraftCount++;
    connectedPlayers++;

    if(connectedPlayers >= maxConnectedPlayers)
      setListening(false);
    else
      peers.push_back(PeerPtr(new RemotePeer()));
  }
}

void GameServer::handleDisconnections()
{
  for(auto itr = peers.begin(); itr != peers.end();)
  {
    if((*itr)->timedOut)
    {
      // Inform everyone of the disconnection, erase
      FOREACH(sf::Int32 identifier, (*itr)->aircraftIdentifiers)
      {
        sf::Packet packet;
        packet << static_cast<sf::Int32>(Server::PlayerDisconnect);
        packet << identifier;
        sendToAll(packet);

        aircraftInfo.erase(identifier);
      }

      connectedPlayers--;
      aircraftCount -= (*itr)->aircraftIdentifiers.size();

      itr = peers.erase(itr);

      // Go back to a listening state if needed
      if(connectedPlayers < maxConnectedPlayers)
      {
        peers.push_back(PeerPtr(new RemotePeer()));
        setListening(true);
      }

      broadcastMessage("An ally has disconnected.");
    }
    else
    {
      ++itr;
    }
  }
}

// Tell the newly connected peer about how the world is currently
void GameServer::informWorldState(sf::TcpSocket& socket)
{
  sf::Packet packet;
  packet << static_cast<sf::Int32>(Server::InitialState);
  packet << worldHeight << battleFieldRect.top + battleFieldRect.height;
  packet << static_cast<sf::Int32>(aircraftCount);

  for(std::size_t i=0; i<connectedPlayers; ++i)
  {
    if(peers[i]->ready)
    {
      FOREACH(sf::Int32 identifier, peers[i]->aircraftIdentifiers)
      {
        packet << identifier;
        packet << aircraftInfo[identifier].position.x;
        packet << aircraftInfo[identifier].position.y;
        packet << aircraftInfo[identifier].hitpoints;
        packet << aircraftInfo[identifier].missileAmmo;
      }
    }
  }

  socket.send(packet);
}

void GameServer::broadcastMessage(const std::string& message)
{
  for(std::size_t i=0; i<connectedPlayers; ++i)
  {
    if(peers[i]->ready)
    {
      sf::Packet packet;
      packet << static_cast<sf::Int32>(Server::BroadcastMessage);
      packet << message;

      peers[i]->socket.send(packet);
    }
  }
}

void GameServer::sendToAll(sf::Packet& packet)
{
  FOREACH(PeerPtr& peer, peers)
  {
    if(peer->ready)
      peer->socket.send(packet);
  }
}

