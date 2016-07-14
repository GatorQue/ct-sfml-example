#ifndef SOURCES_SCOUT_GAMESERVER_HPP_
#define SOURCES_SCOUT_GAMESERVER_HPP_

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Thread.hpp>
#include <SFML/System/Vector2.hpp>

#include <vector>
#include <memory>
#include <map>

class GameServer : private sf::NonCopyable
{
  public:
    explicit GameServer(sf::Vector2f battlefieldSize);
    virtual ~GameServer();

    void notifyPlayerSpawn(sf::Int32 aircraftIdentifier);
    void notifyPlayerRealtimeChange(sf::Int32 aircraftIdentifier, sf::Int32 action, bool actionEnabled);
    void notifyPlayerEvent(sf::Int32 aircraftIdentifier, sf::Int32 action);

  private:
    // A GameServerRemotePeer refers to one instance of the game, it may be
    // local or from another computer
    struct RemotePeer
    {
      RemotePeer();

      sf::TcpSocket socket;
      sf::Time lastPacketTime;
      std::vector<sf::Int32> aircraftIdentifiers;
      bool ready;
      bool timedOut;
    };

    // Structure to store information about current aircraft state
    struct AircraftInfo
    {
      sf::Vector2f position;
      sf::Int32 hitpoints;
      sf::Int32 missileAmmo;
      std::map<sf::Int32, bool> realtimeActions;
    };

    // Unique pointer to remote peers
    typedef std::unique_ptr<RemotePeer> PeerPtr;

    sf::Thread thread;
    sf::Clock clock;
    sf::TcpListener listenerSocket;
    bool listeningState;
    sf::Time clientTimeoutTime;

    std::size_t maxConnectedPlayers;
    std::size_t connectedPlayers;

    float worldHeight;
    sf::FloatRect battleFieldRect;
    float battleFieldScrollSpeed;

    std::size_t aircraftCount;
    std::map<sf::Int32, AircraftInfo> aircraftInfo;

    std::vector<PeerPtr> peers;
    sf::Int32 aircraftIdentifierCounter;
    bool waitingThreadEnd;

    sf::Time lastSpawnTime;
    sf::Time timeForNextSpawn;

    void setListening(bool enable);
    void executionThread();
    void tick();
    sf::Time now() const;

    void handleIncomingPackets();
    void handleIncomingPacket(sf::Packet& packet, RemotePeer& receivingPeer, bool& detectedTimeout);

    void handleIncomingConnections();
    void handleDisconnections();

    void informWorldState(sf::TcpSocket& socket);
    void broadcastMessage(const std::string& message);
    void sendToAll(sf::Packet& packet);
    void updateClientState();
};

#endif
