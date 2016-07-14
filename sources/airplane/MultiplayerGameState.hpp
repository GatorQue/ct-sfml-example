#ifndef SOURCES_SCOUT_MULTIPLAYERGAMESTATE_HPP_
#define SOURCES_SCOUT_MULTIPLAYERGAMESTATE_HPP_

#include "State.hpp"
#include "GameServer.hpp"
#include "NetworkProtocol.hpp"
#include "Player.hpp"
#include "World.hpp"

#include <SFML/Graphics/Text.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/Packet.hpp>
#include <SFML/System/Clock.hpp>

#include <map>
#include <vector>

class MultiplayerGameState : public State
{
  public:
    MultiplayerGameState(StateStack& stack, Context context, bool isHost);

    virtual void draw();
    virtual bool update(sf::Time dt);
    virtual bool handleEvent(const sf::Event& event);
    virtual void onActivate();
    void onDestroy();

    void disableAllRealtimeActions();

  private:
    typedef std::unique_ptr<Player> PlayerPtr;

    World world;
    sf::RenderTarget& target;
    TextureHolder& textures;

    std::map<int, PlayerPtr> players;
    std::vector<sf::Int32> localPlayerIdentifiers;
    sf::TcpSocket socket;
    bool connected;
    std::unique_ptr<GameServer> gameServer;
    sf::Clock tickClock;

    std::vector<std::string> broadcasts;
    sf::Text broadcastText;
    sf::Time broadcastElapsedTime;

    sf::Text playerInvitationText;
    sf::Time playerInvitationTime;

    sf::Text failedConnectionText;
    sf::Clock failedConnectionClock;

    bool activeState;
    bool hasFocus;
    bool host;
    bool gameStarted;
    sf::Time clientTimeout;
    sf::Time timeSinceLastPacket;

    void updateBroadcastMessage(sf::Time elapsedTime);
    void handlePacket(sf::Int32 packetType, sf::Packet& packet);
};

#endif
