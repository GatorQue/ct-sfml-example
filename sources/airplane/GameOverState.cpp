#include "GameOverState.hpp"
#include "Player.hpp"
#include "ResourceHolder.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

GameOverState::GameOverState(StateStack& stack, Context context, const std::string& text) :
  State(stack, context),
  gameOverText(),
  elapsedTime(sf::Time::Zero)
{
  sf::Font& font = context.fonts->get(Fonts::Main);
  sf::Vector2f targetSize(context.target->getSize());

  gameOverText.setFont(font);
  gameOverText.setString(text);
  gameOverText.setCharacterSize(70);
  centerOrigin(gameOverText);
  gameOverText.setPosition(0.5f * targetSize.x, 0.4f * targetSize.y);
}

void GameOverState::draw()
{
  sf::RenderTarget& target = *getContext().target;
  target.setView(target.getDefaultView());

  // Create dark, semitransparent background
  sf::RectangleShape backgroundShape;
  backgroundShape.setFillColor(sf::Color(0, 0, 0, 150));
  backgroundShape.setSize(target.getView().getSize());

  target.draw(backgroundShape);
  target.draw(gameOverText);
}

bool GameOverState::update(sf::Time dt)
{
  // Show state for 3 seconds, after return to menu
  elapsedTime += dt;
  if(elapsedTime > sf::seconds(3))
  {
    requestStateClear();
    requestStackPush(States::Menu);
  }
  return false;
}

bool GameOverState::handleEvent(const sf::Event&)
{
  return false;
}
