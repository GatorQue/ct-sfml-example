#ifndef SOURCES_SCOUT_PLAYER_HPP_
#define SOURCES_SCOUT_PLAYER_HPP_

#include "Command.hpp"
#include "KeyBinding.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Window/Event.hpp>

#include <map>

namespace sf
{
  class TcpSocket;
}

class CommandQueue;

class Player : private sf::NonCopyable
{
  public:
    enum MissionStatus
    {
      MissionRunning,
      MissionSuccess,
      MissionFailure
    };

    Player(sf::TcpSocket* socket, sf::Int32 identifier, const KeyBinding* binding);

    void handleEvent(const sf::Event& event, CommandQueue& commands);
    void handleRealtimeInput(CommandQueue& commands);
    void handleRealtimeNetworkInput(CommandQueue& commands);

    // React to events or realtime state changes received over the network
    void handleNetworkEvent(PlayerActions::Action action, CommandQueue& commands);
    void handleNetworkRealtimeChange(PlayerActions::Action action, bool actionEnabled);

    void setMissionStatus(MissionStatus status);
    MissionStatus getMissionStatus() const;

    void disableAllRealtimeActions();
    bool isLocal() const;

  private:
    const KeyBinding* keyBinding;
    std::map<PlayerActions::Action, Command> actionBinding;
    std::map<PlayerActions::Action, bool> actionProxies;
    MissionStatus currentMissionStatus;
    int identifier;
    sf::TcpSocket* socket;

    void initializeActions();
};

#endif
