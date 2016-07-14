#include "MenuState.hpp"
#include "Button.hpp"
#include "MusicPlayer.hpp"
#include "ResourceHolder.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/View.hpp>

MenuState::MenuState(StateStack& stack, Context context) :
  State(stack, context),
  guiContainer()
{
  sf::Texture& texture = context.textures->get(Textures::TitleScreen);
  backgroundSprite.setTexture(texture);

  auto playButton = std::make_shared<GUI::Button>(context);
  playButton->setPosition(100, 300);
  playButton->setText("Play");
  playButton->setCallback([this] ()
      {
        requestStackPop();
        requestStackPush(States::Game);
      });

  auto hostPlayButton = std::make_shared<GUI::Button>(context);
  hostPlayButton->setPosition(100, 350);
  hostPlayButton->setText("Host");
  hostPlayButton->setCallback([this] ()
      {
        requestStackPop();
        requestStackPush(States::HostGame);
      });

  auto joinPlayButton = std::make_shared<GUI::Button>(context);
  joinPlayButton->setPosition(100, 400);
  joinPlayButton->setText("Join");
  joinPlayButton->setCallback([this] ()
      {
        requestStackPop();
        requestStackPush(States::JoinGame);
      });

  auto settingsButton = std::make_shared<GUI::Button>(context);
  settingsButton->setPosition(100, 450);
  settingsButton->setText("Settings");
  settingsButton->setCallback([this] ()
      {
        requestStackPush(States::Settings);
      });

  auto exitButton = std::make_shared<GUI::Button>(context);
  exitButton->setPosition(100, 500);
  exitButton->setText("Exit");
  exitButton->setCallback([this] ()
      {
        requestStackPop();
      });

  guiContainer.pack(playButton);
  guiContainer.pack(hostPlayButton);
  guiContainer.pack(joinPlayButton);
  guiContainer.pack(settingsButton);
  guiContainer.pack(exitButton);

  //context.music->play(Music::MenuTheme);
}

void MenuState::draw()
{
  sf::RenderTarget& target = *getContext().target;

  target.setView(target.getDefaultView());

  target.draw(backgroundSprite);
  target.draw(guiContainer);
}

bool MenuState::update(sf::Time dt)
{
  return true;
}

bool MenuState::handleEvent(const sf::Event& event)
{
  guiContainer.handleEvent(event);
  return false;
}
