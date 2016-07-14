#ifndef SOURCES_SCOUT_BUTTON_HPP_
#define SOURCES_SCOUT_BUTTON_HPP_

#include "Component.hpp"
#include "ResourceIdentifiers.hpp"
#include "State.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class SoundPlayer;

namespace GUI
{

  class Button : public Component
  {
    public:
      typedef std::shared_ptr<Button> Ptr;
      typedef std::function<void()> Callback;
      enum Type
      {
        Normal,
        Selected,
        Pressed,
        ButtonCount
      };

      Button(State::Context context);

      void setCallback(Callback callback);
      void setText(const std::string& text);
      void setToggle(bool flag);

      virtual bool isSelectable() const;
      virtual void select();
      virtual void deselect();

      virtual void activate();
      virtual void deactivate();

      virtual void handleEvent(const sf::Event& event);

    private:
      Callback callback;
      sf::Sprite sprite;
      sf::Text text;
      bool isToggle;
      SoundPlayer& sounds;

      virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
      void changeTexture(Type buttonType);
  };
}

#endif
