#ifndef SOURCES_SCOUT_SETTINGSSTATE_HPP_
#define SOURCES_SCOUT_SETTINGSSTATE_HPP_

#include "State.hpp"
#include "Container.hpp"
#include "Button.hpp"
#include "KeyBinding.hpp"
#include "Label.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <array>

class SettingsState : public State
{
  public:
    SettingsState(StateStack& stack, Context context);

    virtual void draw();
    virtual bool update(sf::Time dt);
    virtual bool handleEvent(const sf::Event& event);

  private:
    sf::Sprite backgroundSprite;
    GUI::Container guiContainer;
    std::array<GUI::Button::Ptr, 2*PlayerActions::ActionCount> bindingButtons;
    std::array<GUI::Label::Ptr, 2*PlayerActions::ActionCount> bindingLabels;

    void updateLabels();
    void addButtonLabel(std::size_t index, std::size_t x, std::size_t y,
        const std::string& text, Context context);
};

#endif

