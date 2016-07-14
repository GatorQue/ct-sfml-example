#include "NetworkNode.hpp"

NetworkNode::NetworkNode() :
  SceneNode(),
  pendingActions()
{
}

unsigned int NetworkNode::getCategory() const
{
  return Category::Network;
}

void NetworkNode::notifyGameAction(GameActions::Type type, sf::Vector2f position)
{
  pendingActions.push(GameActions::Action(type, position));
}

bool NetworkNode::pollGameAction(GameActions::Action& out)
{
  if(pendingActions.empty())
  {
    return false;
  }
  else
  {
    out = pendingActions.front();
    pendingActions.pop();
    return true;
  }
}
