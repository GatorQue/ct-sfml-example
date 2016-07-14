#ifndef SOURCES_SCOUT_WORLD_HPP_
#define SOURCES_SCOUT_WORLD_HPP_

#include "Aircraft.hpp"
#include "BloomEffect.hpp"
#include "Command.hpp"
#include "CommandQueue.hpp"
#include "NetworkProtocol.hpp"
#include "Pickup.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SoundPlayer.hpp"
#include "SpriteNode.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <array>
#include <queue>
#include <vector>

// Forward declaration of SFML classes
namespace sf
{
  class RenderTarget;
}

class NetworkNode;

class World : private sf::NonCopyable
{
  public:
    explicit World(sf::RenderTarget& outputTarget, FontHolder& fonts,
        SoundPlayer& sounds, bool networked = false);

    void update(sf::Time dt);
    void draw();

    sf::FloatRect getViewBounds() const;
    CommandQueue& getCommandQueue();
    Aircraft* addAircraft(int identifier);
    void removeAircraft(int identifier);
    void setCurrentBattleFieldPosition(float lineY);
    void setWorldHeight(float height);

    void addEnemy(Aircraft::Type type, float relX, float relY);
    void sortEnemies();

    bool hasAlivePlayer() const;
    bool hasPlayerReachedEnd() const;

    void setWorldScrollCompensation(float compensation);

    Aircraft* getAircraft(int identifier) const;
    sf::FloatRect getBattlefieldBounds() const;

    void createPickup(sf::Vector2f, Pickup::Type type);
    bool pollGameAction(GameActions::Action& out);

  private:
    enum Layer
    {
      Background,
      LowerAir,
      UpperAir,
      LayerCount
    };

    struct SpawnPoint
    {
      SpawnPoint(Aircraft::Type type, float x, float y) :
        type(type),
        x(x),
        y(y)
      {
      }

      Aircraft::Type type;
      float x;
      float y;
    };

    sf::RenderTarget& target;
    sf::RenderTexture sceneTexture;
    sf::View worldView;
    TextureHolder textures;
    FontHolder& fonts;
    SoundPlayer& sounds;

    SceneNode sceneGraph;
    std::array<SceneNode*, LayerCount> sceneLayers;
    CommandQueue commandQueue;

    sf::FloatRect worldBounds;
    sf::Vector2f spawnPosition;
    float scrollSpeed;
    float scrollSpeedCompensation;
    std::vector<Aircraft*> playerAircrafts;

    std::vector<SpawnPoint> enemySpawnPoints;
    std::vector<Aircraft*> activeEnemies;

    BloomEffect bloomEffect;

    bool networkedWorld;
    NetworkNode* networkNode;

    void loadTextures();
    void adaptPlayerPosition();
    void adaptPlayerVelocity();
    void handleCollisions();
    void updateSounds();

    void buildScene();
    void addEnemies();
    void spawnEnemies();
    void destroyEntitiesOutsideView();
    void guideMissiles();

};

#endif

