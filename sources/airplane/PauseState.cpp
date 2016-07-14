#include "PauseState.hpp"
#include "Button.hpp"
#include "MusicPlayer.hpp"
#include "ResourceHolder.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

PauseState::PauseState(StateStack& stack, Context context, bool letUpdatesThrough) :
  State(stack, context),
  backgroundSprite(),
  pausedText(),
  guiContainer(),
  letUpdatesThrough(letUpdatesThrough)
{
  sf::Font& font = context.fonts->get(Fonts::Main);
  sf::Vector2f windowSize(context.target->getSize());

  pausedText.setFont(font);
  pausedText.setString("Game Paused");
  pausedText.setCharacterSize(70);
  centerOrigin(pausedText);
  pausedText.setPosition(0.5f * windowSize.x, 0.4f * windowSize.y);

  auto returnButton = std::make_shared<GUI::Button>(context);
  returnButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 75);
  returnButton->setText("Return");
  returnButton->setCallback([this] ()
      {
        requestStackPop();
      });

  auto backToMenuButton = std::make_shared<GUI::Button>(context);
  backToMenuButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 125);
  backToMenuButton->setText("Back to menu");
  backToMenuButton->setCallback([this] ()
      {
        requestStateClear();
        requestStackPush(States::Menu);
      });

  guiContainer.pack(returnButton);
  guiContainer.pack(backToMenuButton);

  context.music->setPaused(true);
}

PauseState::~PauseState()
{
  getContext().music->setPaused(false);
}

void PauseState::draw()
{
  sf::RenderTarget& target = *getContext().target;
  target.setView(target.getDefaultView());

  sf::RectangleShape backgroundShape;
  backgroundShape.setFillColor(sf::Color(0, 0, 0, 150));
  backgroundShape.setSize(sf::Vector2f(target.getSize()));

  target.draw(backgroundShape);
  target.draw(pausedText);
  target.draw(guiContainer);
}

bool PauseState::update(sf::Time dt)
{
  return letUpdatesThrough;
}

bool PauseState::handleEvent(const sf::Event& event)
{
  guiContainer.handleEvent(event);
  return false;
}
