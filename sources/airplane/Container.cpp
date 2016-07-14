#include "Container.hpp"
#include "Foreach.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

namespace GUI
{
  Container::Container() :
    children(),
    selectedChild(-1)
  {
  }

  void Container::pack(Component::Ptr component)
  {
    children.push_back(component);

    if (!hasSelection() && component->isSelectable())
      select(children.size() - 1);
  }

  bool Container::isSelectable() const
  {
    return false;
  }

  void Container::handleEvent(const sf::Event& event)
  {
    // If we have selected a child then give it events
    if(hasSelection() && children[selectedChild]->isActive())
    {
      children[selectedChild]->handleEvent(event);
    }
    else if(event.type == sf::Event::KeyReleased)
    {
      if(event.key.code == sf::Keyboard::W ||
         event.key.code == sf::Keyboard::Up)
      {
        selectPrevious();
      }
      else if(event.key.code == sf::Keyboard::S ||
              event.key.code == sf::Keyboard::Down)
      {
        selectNext();
      }
      else if(event.key.code == sf::Keyboard::Return ||
              event.key.code == sf::Keyboard::Space)
      {
        if (hasSelection())
          children[selectedChild]->activate();
      }
    }
  }

  void Container::draw(sf::RenderTarget& target, sf::RenderStates states) const
  {
    states.transform *= getTransform();

    FOREACH(const Component::Ptr& child, children)
      target.draw(*child, states);
  }

  bool Container::hasSelection() const
  {
    return selectedChild >= 0;
  }

  void Container::select(std::size_t index)
  {
    if(children[index]->isSelectable())
    {
      if(hasSelection())
        children[selectedChild]->deselect();

      children[index]->select();
      selectedChild = index;
    }
  }

  void Container::selectNext()
  {
    if(!hasSelection())
      return;

    // Search next component that is selectable
    int next = selectedChild;
    do
      next = (next + 1) % children.size();
    while (!children[next]->isSelectable());

    // Select that component
    select(next);
  }

  void Container::selectPrevious()
  {
    if(!hasSelection())
      return;

    // Search previous component that is selectable
    int prev = selectedChild;
    do
      prev = (prev + children.size() - 1) % children.size();
    while (!children[prev]->isSelectable());

    // Select that component
    select(prev);
  }
}
