#ifndef SOURCES_SCOUT_SCENENODE_HPP_
#define SOURCES_SCOUT_SCENENODE_HPP_

#include "Category.hpp"

#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>

#include <memory>
#include <set>
#include <vector>
#include <utility>

struct Command;
class CommandQueue;

class SceneNode : public sf::Transformable, public sf::Drawable, private sf::NonCopyable
{
  public:
    typedef std::unique_ptr<SceneNode> Ptr;
    typedef std::pair<SceneNode*, SceneNode*> Pair;

    explicit SceneNode(Category::Type category = Category::None);

    void attachChild(Ptr child);
    Ptr detachChild(const SceneNode& node);

    void update(sf::Time dt, CommandQueue& commands);

    sf::Vector2f getWorldPosition() const;
    sf::Transform getWorldTransform() const;

    void onCommand(const Command& command, sf::Time dt);
    virtual unsigned int getCategory() const;

    void checkSceneCollision(SceneNode& sceneGraph, std::set<Pair>& collisionPairs);
    void checkNodeCollision(SceneNode& node, std::set<Pair>& collisionPairs);
    void removeWrecks();
    virtual sf::FloatRect getBoundingRect() const;
    virtual bool isMarkedForRemoval() const;
    virtual bool isDestroyed() const;

  private:
    Category::Type defaultCategory;
    SceneNode* parent;
    std::vector<Ptr> children;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const final;
    virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
    void drawChildren(sf::RenderTarget& target, sf::RenderStates states) const;
    void drawBoundingRect(sf::RenderTarget& target, sf::RenderStates states) const;

    virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
    void updateChildren(sf::Time dt, CommandQueue& commands);
};

bool collision(const SceneNode& lhs, const SceneNode& rhs);
float distance(const SceneNode& lhs, const SceneNode& rhs);

#endif
