#include "GameState.hpp"
#include "MusicPlayer.hpp"

GameState::GameState(StateStack& stack, Context context) :
  State(stack, context),
  world(*context.target, *context.fonts, *context.sounds, false),
  player(nullptr, 1, context.keys1)
{
  world.addAircraft(1);
  player.setMissionStatus(Player::MissionRunning);

  // Play game theme
  //context.music->play(Music::MissionTheme);
}

void GameState::draw()
{
  world.draw();
}

bool GameState::update(sf::Time dt)
{
  world.update(dt);

  if(!world.hasAlivePlayer())
  {
    player.setMissionStatus(Player::MissionFailure);
    requestStackPush(States::GameOver);
  }
  else if(world.hasPlayerReachedEnd())
  {
    player.setMissionStatus(Player::MissionSuccess);
    requestStackPush(States::MissionSuccess);
  }

  CommandQueue& commands = world.getCommandQueue();
  player.handleRealtimeInput(commands);

  return true;
}

bool GameState::handleEvent(const sf::Event& event)
{
  // Game input handling
  CommandQueue& commands = world.getCommandQueue();
  player.handleEvent(event, commands);

  // Escape pressed, trigger the pause screen
  if(event.type == sf::Event::KeyPressed &&
     event.key.code == sf::Keyboard::Escape)
    requestStackPush(States::Pause);

  return true;
}
