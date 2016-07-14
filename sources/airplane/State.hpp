#ifndef SOURCES_SCOUT_STATE_HPP_
#define SOURCES_SCOUT_STATE_HPP_

#include "StateIdentifiers.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>

#include <memory>

// Forward declare SFML classes
namespace sf
{
  class RenderTarget;
}

class StateStack;
class MusicPlayer;
class SoundPlayer;
class KeyBinding;

class State
{
  public:
    typedef std::unique_ptr<State> Ptr;

    struct Context
    {
      Context(sf::RenderTarget& target,
          FontHolder& fonts,
          MusicPlayer& music,
          SoundPlayer& sounds,
          TextureHolder& textures,
          KeyBinding& keys1,
          KeyBinding& keys2);

      sf::RenderTarget* target;
      FontHolder* fonts;
      MusicPlayer* music;
      SoundPlayer* sounds;
      TextureHolder* textures;
      KeyBinding* keys1;
      KeyBinding* keys2;
    };

    State(StateStack& stack, Context context);
    virtual ~State();

    virtual void draw() = 0;
    virtual bool update(sf::Time dt) = 0;
    virtual bool handleEvent(const sf::Event& event) = 0;

    virtual void onActivate();
    virtual void onDestroy();

  protected:
    void requestStackPush(States::ID stateID);
    void requestStackPop();
    void requestStateClear();

    Context getContext() const;

  private:
    StateStack* stack;
    Context context;
};

#endif
