#include "TitleState.hpp"
#include "ResourceHolder.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

TitleState::TitleState(StateStack& stack, Context context) :
  State(stack, context),
  backgroundSprite(),
  text(),
  showText(true),
  textEffectTime(sf::Time::Zero)
{
  sf::Texture& texture = context.textures->get(Textures::TitleScreen);
  backgroundSprite.setTexture(texture);

  sf::Font& font = context.fonts->get(Fonts::Main);
  text.setFont(font);
  text.setString("Press any key to start");
  centerOrigin(text);
  text.setPosition(context.target->getView().getSize() / 2.f);
}

void TitleState::draw()
{
  sf::RenderTarget& target = *getContext().target;
  target.draw(backgroundSprite);

  if(showText)
    target.draw(text);
}

bool TitleState::update(sf::Time dt)
{
  textEffectTime += dt;

  if(textEffectTime >= sf::seconds(0.5f))
  {
    showText = !showText;
    textEffectTime = sf::Time::Zero;
  }

  return true;
}

bool TitleState::handleEvent(const sf::Event& event)
{
  // If any key is pressed, trigger the next screen
  if(event.type == sf::Event::KeyReleased)
  {
    requestStackPop();
    requestStackPush(States::Menu);
  }

  return true;
}
