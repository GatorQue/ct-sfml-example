#ifndef SOURCES_SCOUT_APPLICATION_HPP_
#define SOURCES_SCOUT_APPLICATION_HPP_

#include "KeyBinding.hpp"
#include "MusicPlayer.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SoundPlayer.hpp"
#include "StateStack.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>

class Application : private sf::NonCopyable
{
  public:
    Application();

    void run();

  private:
    static const sf::Time TimePerFrame;

    sf::RenderWindow window;
    FontHolder fonts;
    MusicPlayer music;
    SoundPlayer sounds;
    TextureHolder textures;

    KeyBinding keyBinding1;
    KeyBinding keyBinding2;
    StateStack stateStack;

    sf::Text statisticsText;
    sf::Time statisticsUpdateTime;
    std::size_t statisticsNumFrames;

    void processInput();
    void update(sf::Time dt);
    void render();

    void updateStatistics(sf::Time dt);
    void registerStates();
};

#endif
