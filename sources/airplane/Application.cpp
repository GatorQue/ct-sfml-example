#include "Application.hpp"
#include "State.hpp"
#include "StateIdentifiers.hpp"
#include "StringUtils.hpp"
#include "GameOverState.hpp"
#include "GameState.hpp"
#include "LoadingState.hpp"
#include "MenuState.hpp"
#include "MultiplayerGameState.hpp"
#include "PauseState.hpp"
#include "SettingsState.hpp"
#include "TitleState.hpp"

const sf::Time Application::TimePerFrame = sf::seconds(1.f / 60.f);

Application::Application() :
  window(sf::VideoMode(1024, 768), "Scout", sf::Style::Close),
  fonts(),
  music(),
  sounds(),
  textures(),
  keyBinding1(1),
  keyBinding2(2),
  stateStack(State::Context(window, fonts, music, sounds, textures, keyBinding1, keyBinding2)),
  statisticsText(),
  statisticsUpdateTime(),
  statisticsNumFrames(0)
{
  window.setKeyRepeatEnabled(false);

  fonts.load_now(Fonts::Main, "assets/fonts/Sansation.ttf");

  textures.load_now(Textures::TitleScreen, "assets/textures/TitleScreen.png");
  textures.load_now(Textures::Buttons, "assets/textures/Buttons.png");

  statisticsText.setFont(fonts.get(Fonts::Main));
  statisticsText.setPosition(5.f, 5.f);
  statisticsText.setCharacterSize(10u);

  registerStates();
  stateStack.pushState(States::Title);

  music.setVolume(25.f);
}

void Application::run()
{
  sf::Clock clock;
  sf::Time timeSinceLastUpdate = sf::Time::Zero;

  while(window.isOpen())
  {
    sf::Time dt = clock.restart();
    timeSinceLastUpdate += dt;
    while(timeSinceLastUpdate > TimePerFrame)
    {
      timeSinceLastUpdate -= TimePerFrame;

      processInput();
      update(TimePerFrame);

      // Check inside this loop, because stack might be empty before update
      // call
      if(stateStack.isEmpty())
        window.close();
    }

    updateStatistics(dt);
    render();
  }
}

void Application::processInput()
{
  sf::Event event;
  while(window.pollEvent(event))
  {
    stateStack.handleEvent(event);

    if(event.type == sf::Event::Closed)
    {
      window.close();
    }
  }
}

void Application::update(sf::Time dt)
{
  stateStack.update(dt);
}

void Application::render()
{
  window.clear();

  stateStack.draw();

  window.setView(window.getDefaultView());
  window.draw(statisticsText);

  window.display();
}

void Application::updateStatistics(sf::Time dt)
{
  statisticsUpdateTime += dt;
  statisticsNumFrames += 1;
  if(statisticsUpdateTime >= sf::seconds(1.0f))
  {
    statisticsText.setString("FPS: " + toString(statisticsNumFrames));

    statisticsUpdateTime -= sf::seconds(1.0f);
    statisticsNumFrames = 0;
  }
}

void Application::registerStates()
{
  stateStack.registerState<TitleState>(States::Title);
  stateStack.registerState<MenuState>(States::Menu);
  stateStack.registerState<GameState>(States::Game);
  stateStack.registerState<LoadingState>(States::Loading);
  stateStack.registerState<MultiplayerGameState>(States::HostGame, true);
  stateStack.registerState<MultiplayerGameState>(States::JoinGame, false);
  stateStack.registerState<PauseState>(States::Pause);
  stateStack.registerState<PauseState>(States::NetworkPause, true);
  stateStack.registerState<SettingsState>(States::Settings);
  stateStack.registerState<GameOverState>(States::GameOver, "Mission Failed!");
  stateStack.registerState<GameOverState>(States::MissionSuccess, "Mission Successful!");
}

