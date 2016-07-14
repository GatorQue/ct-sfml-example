#ifndef SOURCES_SCOUT_CONTAINER_HPP_
#define SOURCES_SCOUT_CONTAINER_HPP_

#include "Component.hpp"

#include <memory>
#include <vector>

namespace GUI
{
  class Container : public Component
  {
    public:
      typedef std::shared_ptr<Container> Ptr;

      Container();

      void pack(Component::Ptr component);

      virtual bool isSelectable() const;
      virtual void handleEvent(const sf::Event& event);

    private:
      std::vector<Component::Ptr> children;
      int selectedChild;

      virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

      bool hasSelection() const;
      void select(std::size_t index);
      void selectNext();
      void selectPrevious();
  };
}

#endif
