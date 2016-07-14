#include "SettingsState.hpp"
#include "ResourceHolder.hpp"
#include "StringUtils.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

SettingsState::SettingsState(StateStack& stack, Context context) :
  State(stack, context),
  backgroundSprite(),
  guiContainer(),
  bindingButtons(),
  bindingLabels()
{
  backgroundSprite.setTexture(context.textures->get(Textures::TitleScreen));

  // Build key binding buttons and labels
  for(std::size_t x=0; x<2; ++x)
  {
    addButtonLabel(PlayerActions::MoveLeft, x, 0, "Move Left", context);
    addButtonLabel(PlayerActions::MoveRight, x, 1, "Move Right", context);
    addButtonLabel(PlayerActions::MoveUp, x, 2, "Move Up", context);
    addButtonLabel(PlayerActions::MoveDown, x, 3, "Move Down", context);
    addButtonLabel(PlayerActions::Fire, x, 4, "Fire", context);
    addButtonLabel(PlayerActions::LaunchMissile, x, 5, "Missile", context);
  }

  updateLabels();

  auto backButton = std::make_shared<GUI::Button>(context);
  backButton->setPosition(80.f, 620.f);
  backButton->setText("Back");
  backButton->setCallback(std::bind(&SettingsState::requestStackPop, this));

  guiContainer.pack(backButton);
}

void SettingsState::draw()
{
  sf::RenderTarget& target = *getContext().target;

  target.draw(backgroundSprite);
  target.draw(guiContainer);
}

bool SettingsState::update(sf::Time dt)
{
  return true;
}

bool SettingsState::handleEvent(const sf::Event& event)
{
  bool isKeyBinding = false;

  // Iterate through all key binding buttons to see if they are being pressed,
  // waiting for the user to enter a key
  for(std::size_t i = 0; i<2*PlayerActions::ActionCount; ++i)
  {
    if(bindingButtons[i]->isActive())
    {
      isKeyBinding = true;
      if(event.type == sf::Event::KeyReleased)
      {
        // Player 1
        if(i < PlayerActions::ActionCount)
          getContext().keys1->assignKey(
              static_cast<PlayerActions::Action>(i), event.key.code);
        // Player 2
        else
          getContext().keys2->assignKey(
              static_cast<PlayerActions::Action>(i - PlayerActions::ActionCount), event.key.code);

        bindingButtons[i]->deactivate();
      }
      break;
    }
  }

  // If pressed button changed key bindings, update labels; otherwise consider
  // other buttons in container
  if(isKeyBinding)
    updateLabels();
  else
    guiContainer.handleEvent(event);

  return false;
}

void SettingsState::updateLabels()
{
  for(std::size_t i=0; i<PlayerActions::ActionCount; ++i)
  {
    auto action = static_cast<PlayerActions::Action>(i);

    // Get keys of both players
    sf::Keyboard::Key key1 = getContext().keys1->getAssignedKey(action);
    sf::Keyboard::Key key2 = getContext().keys2->getAssignedKey(action);

    // Assign both key strings to labels
    bindingLabels[i]->setText(toString(key1));
    bindingLabels[i + PlayerActions::ActionCount]->setText(toString(key2));
  }
}

void SettingsState::addButtonLabel(std::size_t index, std::size_t x, std::size_t y,
    const std::string& text, Context context)
{
  // For x==0, start at index 0, otherwise start at half of array
  index += PlayerActions::ActionCount * x;

  bindingButtons[index] = std::make_shared<GUI::Button>(context);
  bindingButtons[index]->setPosition(400.f*x + 80.f, 50.f*y + 300.f);
  bindingButtons[index]->setText(text);
  bindingButtons[index]->setToggle(true);

  bindingLabels[index] = std::make_shared<GUI::Label>("", *context.fonts);
  bindingLabels[index]->setPosition(400.f*x + 300.f, 50.f*y + 315.f);

  guiContainer.pack(bindingButtons[index]);
  guiContainer.pack(bindingLabels[index]);
}

