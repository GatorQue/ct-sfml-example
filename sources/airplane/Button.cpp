#include "Button.hpp"
#include "ResourceHolder.hpp"
#include "SoundPlayer.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

namespace GUI
{
  Button::Button(State::Context context) :
    callback(),
    sprite(context.textures->get(Textures::Buttons)),
    text("", context.fonts->get(Fonts::Main), 16),
    isToggle(false),
    sounds(*context.sounds)
  {
    changeTexture(Normal);

    sf::FloatRect bounds = sprite.getLocalBounds();
    text.setPosition(bounds.width / 2.f, bounds.height / 2.f);
  }

  void Button::setCallback(Callback callback)
  {
    this->callback = std::move(callback);
  }

  void Button::setText(const std::string& text)
  {
    this->text.setString(text);
    centerOrigin(this->text);
  }

  void Button::setToggle(bool flag)
  {
    isToggle = flag;
  }

  bool Button::isSelectable() const
  {
    return true;
  }

  void Button::select()
  {
    Component::select();

    changeTexture(Selected);
  }

  void Button::deselect()
  {
    Component::deselect();

    changeTexture(Normal);
  }

  void Button::activate()
  {
    Component::activate();

    // If we are toggle then we should show that the button is pressed and
    // thus "toggled"
    if(isToggle)
      changeTexture(Pressed);

    if(callback)
      callback();

    // If we are not a toggle then deactivate the button since we are just
    // momentarily activated.
    if(!isToggle)
      deactivate();

    sounds.play(SoundEffect::Button);
  }

  void Button::deactivate()
  {
    Component::deactivate();

    if(isToggle)
    {
      // Reset texture to right one depending on if we are selected or not.
      if(isSelected())
        changeTexture(Selected);
      else
        changeTexture(Normal);
    }
  }

  void Button::handleEvent(const sf::Event& event)
  {
  }

  void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    states.transform *= getTransform();
    target.draw(sprite, states);
    target.draw(text, states);
  }

  void Button::changeTexture(Type buttonType)
  {
    sf::IntRect textureRect(0, 50*buttonType, 200, 50);
    sprite.setTextureRect(textureRect);
  }
}
