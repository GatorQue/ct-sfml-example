#include "MultiplayerGameState.hpp"
#include "Foreach.hpp"
#include "MusicPlayer.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>

sf::IpAddress getAddressFromFile()
{
  { // Try to open existing file (RAII block)
    std::ifstream inputFile("assets/config/ip.txt");
    std::string ipAddress;
    if(inputFile >> ipAddress)
      return ipAddress;
  }

  // If open/read failed, create new file
  std::ofstream outputFile("assets/config/ip.txt");
  std::string localAddress = "127.0.0.1";
  outputFile << localAddress;
  return localAddress;
}

MultiplayerGameState::MultiplayerGameState(StateStack& stack, Context context, bool isHost) :
  State(stack, context),
  world(*context.target, *context.fonts, *context.sounds, true),
  target(*context.target),
  textures(*context.textures),
  connected(false),
  gameServer(nullptr),
  activeState(true),
  hasFocus(true),
  host(isHost),
  gameStarted(false),
  clientTimeout(sf::seconds(2.f)),
  timeSinceLastPacket(sf::seconds(0.f))
{
  sf::Font& font = context.fonts->get(Fonts::Main);
  broadcastText.setFont(font);
  broadcastText.setPosition(1024.f / 2, 100.f);

  playerInvitationText.setFont(font);
  playerInvitationText.setCharacterSize(20);
  playerInvitationText.setColor(sf::Color::White);
  playerInvitationText.setString("Press Enter to spawn player 2");
  playerInvitationText.setPosition(1000 - playerInvitationText.getLocalBounds().width, 760 - playerInvitationText.getLocalBounds().height);

  // We reuse this text for "Attempt to connect" and "Failed to connect" messages
  failedConnectionText.setFont(font);
  failedConnectionText.setString("Attempting to connect...");
  failedConnectionText.setCharacterSize(35);
  failedConnectionText.setColor(sf::Color::White);
  centerOrigin(failedConnectionText);
  failedConnectionText.setPosition(target.getSize().x / 2.f, target.getSize().y / 2.f);

  // Render a "establishing connection" frame for user feedback
  target.clear(sf::Color::Black);
  target.draw(failedConnectionText);
  //target.display();
  failedConnectionText.setString("Could not connect to the remote server!");
  centerOrigin(failedConnectionText);

  sf::IpAddress ip;
  if(isHost)
  {
    gameServer.reset(new GameServer(sf::Vector2f(target.getSize())));
    ip = "127.0.0.1";
  }
  else
  {
    ip = getAddressFromFile();
  }

  if(socket.connect(ip, serverPort, sf::seconds(5.f)) == sf::TcpSocket::Done)
    connected = true;
  else
    failedConnectionClock.restart();

  socket.setBlocking(false);

  // Play game theme
  //context.music->play(Music::MissionTheme);
}

void MultiplayerGameState::draw()
{
  if(connected)
  {
    world.draw();

    // Broadcast messages in default view
    target.setView(target.getDefaultView());

    if(!broadcasts.empty())
      target.draw(broadcastText);

    if(localPlayerIdentifiers.size() < 2 && playerInvitationTime < sf::seconds(0.5f))
      target.draw(playerInvitationText);
  }
  else
  {
    target.draw(failedConnectionText);
  }
}

void MultiplayerGameState::onActivate()
{
  activeState = true;
}

void MultiplayerGameState::onDestroy()
{
  if(!host && connected)
  {
    // Inform server this client is dying
    sf::Packet packet;
    packet << static_cast<sf::Int32>(Client::Quit);
    socket.send(packet);
  }
}

bool MultiplayerGameState::update(sf::Time dt)
{
  // Connected to server: Handle all the network logic
  if(connected)
  {
    world.update(dt);

    // Remove players whose aircrafts were destroyed
    bool foundLocalPlane = false;
    for(auto itr = players.begin(); itr != players.end();)
    {
      // Check if there are no more local planes for remote clients
      auto itrLocal = std::find(localPlayerIdentifiers.begin(), localPlayerIdentifiers.end(), itr->first);
      if(itrLocal != localPlayerIdentifiers.end())
      {
        foundLocalPlane = true;
      }

      if(!world.getAircraft(itr->first))
      {
        if(foundLocalPlane)
          localPlayerIdentifiers.erase(itrLocal);
        itr = players.erase(itr);

        // No more players left: Mission failed
        if(players.empty())
          requestStackPush(States::GameOver);
      }
      else
      {
        ++itr;
      }
    }

    if(!foundLocalPlane && gameStarted)
    {
      requestStackPush(States::GameOver);
    }

    // Only handle the realtime input if the window has focus and the game is
    // unpaused
    CommandQueue& commands = world.getCommandQueue();
    if(activeState && hasFocus)
    {
      FOREACH(auto& pair, players)
        pair.second->handleRealtimeInput(commands);
    }

    // Always handle the network input
    FOREACH(auto& pair, players)
      pair.second->handleRealtimeNetworkInput(commands);

    // Handle messages from server that may have arrived
    sf::Packet packet;
    if(socket.receive(packet) == sf::Socket::Done)
    {
      timeSinceLastPacket = sf::seconds(0.f);
      sf::Int32 packetType;
      packet >> packetType;
      handlePacket(packetType, packet);
    }
    else
    {
      // Check for timeout with the server
      if(timeSinceLastPacket > clientTimeout)
      {
        connected = false;

        failedConnectionText.setString("Lost connection to server");
        centerOrigin(failedConnectionText);

        failedConnectionClock.restart();
      }
    }

    updateBroadcastMessage(dt);

    // Time counter for blinking 2nd player text
    playerInvitationTime += dt;
    if(playerInvitationTime > sf::seconds(1.f))
      playerInvitationTime = sf::Time::Zero;

    // Events occurring in the game
    GameActions::Action gameAction;
    while(world.pollGameAction(gameAction))
    {
      sf::Packet packet;
      packet << static_cast<sf::Int32>(Client::GameEvent);
      packet << static_cast<sf::Int32>(gameAction.type);
      packet << gameAction.position.x;
      packet << gameAction.position.y;

      socket.send(packet);
    }

    // Regular position updates
    if(tickClock.getElapsedTime() > sf::seconds(1.f / 20.f))
    {
      sf::Packet positionUpdatePacket;
      positionUpdatePacket << static_cast<sf::Int32>(Client::PositionUpdate);
      positionUpdatePacket << static_cast<sf::Int32>(localPlayerIdentifiers.size());

      FOREACH(sf::Int32 identifier, localPlayerIdentifiers)
      {
        if(Aircraft* aircraft = world.getAircraft(identifier))
        {
          positionUpdatePacket << identifier;
          positionUpdatePacket << aircraft->getPosition().x;
          positionUpdatePacket << aircraft->getPosition().y;
          positionUpdatePacket << static_cast<sf::Int32>(aircraft->getHitpoints());
          positionUpdatePacket << static_cast<sf::Int32>(aircraft->getMissileAmmo());
        }
      }

      socket.send(positionUpdatePacket);
      tickClock.restart();
    }

    timeSinceLastPacket += dt;
  }
  else if(failedConnectionClock.getElapsedTime() >= sf::seconds(5.f))
  {
    requestStateClear();
    requestStackPush(States::Menu);
  }

  return true;
}

void MultiplayerGameState::disableAllRealtimeActions()
{
  activeState = false;

  FOREACH(sf::Int32 identifier, localPlayerIdentifiers)
    players[identifier]->disableAllRealtimeActions();
}

bool MultiplayerGameState::handleEvent(const sf::Event& event)
{
  // Game input handling
  CommandQueue& commands = world.getCommandQueue();

  // Forward event to all players
  FOREACH(auto& pair, players)
    pair.second->handleEvent(event, commands);

  if(event.type == sf::Event::KeyPressed)
  {
    // Enter pressed, add second player co-op (only if we one player)
    if(event.key.code == sf::Keyboard::Return && localPlayerIdentifiers.size() == 1)
    {
      sf::Packet packet;
      packet << static_cast<sf::Int32>(Client::RequestCoopPartner);

      socket.send(packet);
    }

    // Escape pressed, trigger the pause screen
    if(event.key.code == sf::Keyboard::Escape)
    {
      disableAllRealtimeActions();
      requestStackPush(States::NetworkPause);
    }
  }
  else if(event.type == sf::Event::GainedFocus)
  {
    hasFocus = true;
  }
  else if(event.type == sf::Event::LostFocus)
  {
    hasFocus = false;
  }

  return true;
}

void MultiplayerGameState::updateBroadcastMessage(sf::Time elapsedTime)
{
  if (broadcasts.empty())
    return;

  // Update broadcast timer
  broadcastElapsedTime += elapsedTime;
  if(broadcastElapsedTime > sf::seconds(2.5f))
  {
    // If message has expired, remove it
    broadcasts.erase(broadcasts.begin());

    // Continue to display next broadcast message
    if(!broadcasts.empty())
    {
      broadcastText.setString(broadcasts.front());
      centerOrigin(broadcastText);
      broadcastElapsedTime = sf::Time::Zero;
    }
  }
}

void MultiplayerGameState::handlePacket(sf::Int32 packetType, sf::Packet& packet)
{
  switch(packetType)
  {
    // Send message to call clients
    case Server::BroadcastMessage:
      {
        std::string message;
        packet >> message;
        broadcasts.push_back(message);

        // Just added first message, display immediately
        if(broadcasts.size() == 1)
        {
          broadcastText.setString(broadcasts.front());
          centerOrigin(broadcastText);
          broadcastElapsedTime = sf::Time::Zero;
        }
      }
      break;

    // Sent by the server to order to spawn player 1 airplane on connect
    case Server::SpawnSelf:
      {
        sf::Int32 aircraftIdentifier;
        sf::Vector2f aircraftPosition;
        packet >> aircraftIdentifier;
        packet >> aircraftPosition.x;
        packet >> aircraftPosition.y;

        Aircraft* aircraft = world.addAircraft(aircraftIdentifier);
        aircraft->setPosition(aircraftPosition);

        players[aircraftIdentifier].reset(new Player(&socket, aircraftIdentifier, getContext().keys1));
        localPlayerIdentifiers.push_back(aircraftIdentifier);

        gameStarted = true;
      }
      break;

    //
    case Server::PlayerConnect:
      {
        sf::Int32 aircraftIdentifier;
        sf::Vector2f aircraftPosition;
        packet >> aircraftIdentifier;
        packet >> aircraftPosition.x;
        packet >> aircraftPosition.y;

        Aircraft* aircraft = world.addAircraft(aircraftIdentifier);
        aircraft->setPosition(aircraftPosition);

        players[aircraftIdentifier].reset(new Player(&socket, aircraftIdentifier, nullptr));
      }
      break;

    //
    case Server::PlayerDisconnect:
      {
        sf::Int32 aircraftIdentifier;
        packet >> aircraftIdentifier;

        world.removeAircraft(aircraftIdentifier);
        players.erase(aircraftIdentifier);
      }
      break;

    //
    case Server::InitialState:
      {
        float worldHeight, currentScroll;
        packet >> worldHeight >> currentScroll;

        world.setWorldHeight(worldHeight);
        world.setCurrentBattleFieldPosition(currentScroll);

        sf::Int32 aircraftCount;
        packet >> aircraftCount;
        for(sf::Int32 i=0; i<aircraftCount; ++i)
        {
          sf::Int32 aircraftIdentifier;
          sf::Int32 hitpoints;
          sf::Int32 missileAmmo;
          sf::Vector2f aircraftPosition;

          packet >> aircraftIdentifier;
          packet >> aircraftPosition.x;
          packet >> aircraftPosition.y;
          packet >> hitpoints;
          packet >> missileAmmo;

          Aircraft* aircraft = world.addAircraft(aircraftIdentifier);
          aircraft->setPosition(aircraftPosition);
          aircraft->setHitpoints(hitpoints);
          aircraft->setMissileAmmo(missileAmmo);

          players[aircraftIdentifier].reset(new Player(&socket, aircraftIdentifier, nullptr));
        }
      }
      break;

    //
    case Server::AcceptCoopPartner:
      {
        sf::Int32 aircraftIdentifier;
        packet >> aircraftIdentifier;

        world.addAircraft(aircraftIdentifier);
        players[aircraftIdentifier].reset(new Player(&socket, aircraftIdentifier, getContext().keys2));
        localPlayerIdentifiers.push_back(aircraftIdentifier);
      }
      break;

    // Player event (like missile fired) occurs
    case Server::PlayerEvent:
      {
        sf::Int32 aircraftIdentifier;
        sf::Int32 action;
        packet >> aircraftIdentifier >> action;

        auto itr = players.find(aircraftIdentifier);
        if(itr != players.end())
          itr->second->handleNetworkEvent(static_cast<PlayerActions::Action>(action), world.getCommandQueue());
      }
      break;

    // Player's movement or fire keyboard state changes
    case Server::PlayerRealtimeChange:
      {
        sf::Int32 aircraftIdentifier;
        sf::Int32 action;
        bool actionEnabled;
        packet >> aircraftIdentifier >> action >> actionEnabled;

        auto itr = players.find(aircraftIdentifier);
        if(itr != players.end())
          itr->second->handleNetworkRealtimeChange(static_cast<PlayerActions::Action>(action), actionEnabled);
      }
      break;

    // New enemy to be created
    case Server::SpawnEnemy:
      {
        float height;
        sf::Int32 type;
        float relativeX;
        packet >> type >> height >> relativeX;

        world.addEnemy(static_cast<Aircraft::Type>(type), relativeX, height);
        world.sortEnemies();
      }
      break;

    // Mission successfully completed
    case Server::MissionSuccess:
      {
        requestStackPush(States::MissionSuccess);
      }
      break;

    // Pickup created
    case Server::SpawnPickup:
      {
        sf::Int32 type;
        sf::Vector2f position;
        packet >> type >> position.x >> position.y;

        world.createPickup(position, static_cast<Pickup::Type>(type));
      }
      break;

    //
    case Server::UpdateClientState:
      {
        float currentWorldPosition;
        sf::Int32 aircraftCount;
        packet >> currentWorldPosition >> aircraftCount;

        float currentViewPosition = world.getViewBounds().top + world.getViewBounds().height;

        // Set the world's scroll compensation according to whether the view
        // is behind or too advanced
        world.setWorldScrollCompensation(currentViewPosition / currentWorldPosition);

        for(sf::Int32 i=0; i<aircraftCount; ++i)
        {
          sf::Vector2f aircraftPosition;
          sf::Int32 aircraftIdentifier;
          packet >> aircraftIdentifier >> aircraftPosition.x >> aircraftPosition.y;

          Aircraft* aircraft = world.getAircraft(aircraftIdentifier);
          bool isLocalPlane = std::find(localPlayerIdentifiers.begin(),
              localPlayerIdentifiers.end(), aircraftIdentifier) !=
            localPlayerIdentifiers.end();
          if(aircraft && !isLocalPlane)
          {
            sf::Vector2f interpolatedPosition = aircraft->getPosition() +
              (aircraftPosition - aircraft->getPosition()) * 0.1f;
            aircraft->setPosition(interpolatedPosition);
          }
        }
      }
      break;
  }
}
