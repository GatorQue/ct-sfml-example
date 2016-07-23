#include "World.hpp"
#include "Foreach.hpp"
#include "NetworkNode.hpp"
#include "ParticleNode.hpp"
#include "Projectile.hpp"
#include "SoundNode.hpp"
#include "TextNode.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

World::World(sf::RenderTarget& outputTarget, FontHolder& fonts, SoundPlayer& sounds, bool networked) :
    target(outputTarget),
    sceneTexture(),
    worldView(outputTarget.getDefaultView()),
    textures(),
    fonts(fonts),
    sounds(sounds),
    sceneGraph(),
    sceneLayers(),
    commandQueue(),
    worldBounds(0.f, 0.f, worldView.getSize().x, 5000.f),
    spawnPosition(worldView.getSize().x / 2.f, worldBounds.height - worldView.getSize().y / 2.f),
    scrollSpeed(-50.f),
    scrollSpeedCompensation(1.f),
    playerAircrafts(),
    enemySpawnPoints(),
    activeEnemies(),
    bloomEffect(),
    networkedWorld(networked),
    networkNode(nullptr)
{
  sceneTexture.create(target.getSize().x, target.getSize().y);

  loadTextures();
  buildScene();

  // Prepare the view
  worldView.setCenter(spawnPosition);
}

void World::setWorldScrollCompensation(float compensation)
{
  scrollSpeedCompensation = compensation;
}

void World::update(sf::Time dt)
{
  // Scroll the world
  worldView.move(0.f, scrollSpeed * dt.asSeconds() * scrollSpeedCompensation);

  FOREACH(Aircraft* a, playerAircrafts)
    a->setVelocity(0.f, 0.f);

  // Setup commands to destroy entities, and guide missiles
  destroyEntitiesOutsideView();
  guideMissiles();

  // Forward commands to the scene graph
  while(!commandQueue.isEmpty())
    sceneGraph.onCommand(commandQueue.pop(), dt);

  adaptPlayerVelocity();

  // Collision detection and response (may destroy entities)
  handleCollisions();

  // Remove aircrafts that were destroyed (World::removeWrecks() only destroys
  // the entities, not the pointers in playerAircrafts)
  auto firstToRemove = std::remove_if(playerAircrafts.begin(), playerAircrafts.end(), std::mem_fn(&Aircraft::isMarkedForRemoval));
  playerAircrafts.erase(firstToRemove, playerAircrafts.end());

  // Remove all destroyed entities, create new ones
  sceneGraph.removeWrecks();
  spawnEnemies();

  // Regular update step, adapt position (correct if outside view)
  sceneGraph.update(dt, commandQueue);
  adaptPlayerPosition();

  updateSounds();
}

void World::draw()
{
  if (PostEffect::isSupported())
  {
    sceneTexture.clear();
    sceneTexture.setView(worldView);
    sceneTexture.draw(sceneGraph);
    sceneTexture.display();
    bloomEffect.apply(sceneTexture, target);
  }
  else
  {
    target.setView(worldView);
    target.draw(sceneGraph);
  }
}

CommandQueue& World::getCommandQueue()
{
  return commandQueue;
}

Aircraft* World::getAircraft(int identifier) const
{
  FOREACH(Aircraft* a, playerAircrafts)
    if(a->getIdentifier() == identifier)
      return a;

  return nullptr;
}

void World::removeAircraft(int identifier)
{
  Aircraft* aircraft = getAircraft(identifier);
  if(aircraft)
  {
    aircraft->destroy();
    playerAircrafts.erase(std::find(playerAircrafts.begin(), playerAircrafts.end(), aircraft));
  }
}

Aircraft* World::addAircraft(int identifier)
{
  std::unique_ptr<Aircraft> player(new Aircraft(Aircraft::Eagle, textures, fonts));
  player->setPosition(worldView.getCenter());
  player->setIdentifier(identifier);

  playerAircrafts.push_back(player.get());
  sceneLayers[UpperAir]->attachChild(std::move(player));
  return playerAircrafts.back();
}

void World::createPickup(sf::Vector2f position, Pickup::Type type)
{
  std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
  pickup->setPosition(position);
  pickup->setVelocity(0.f, 1.f);
  sceneLayers[UpperAir]->attachChild(std::move(pickup));
}

bool World::pollGameAction(GameActions::Action& out)
{
  return networkNode->pollGameAction(out);
}

void World::setCurrentBattleFieldPosition(float lineY)
{
  worldView.setCenter(worldView.getCenter().x, lineY - worldView.getSize().y/2);
  spawnPosition.y = worldBounds.height;
}

void World::setWorldHeight(float height)
{
  worldBounds.height = height;
}

bool World::hasAlivePlayer() const
{
  return playerAircrafts.size() > 0;
}

bool World::hasPlayerReachedEnd() const
{
  if(Aircraft* aircraft = getAircraft(1))
  {
    sf::Vector2f position = aircraft->getPosition();
    bool contained = worldBounds.contains(position);
    return !contained;
  }
  else
    return false;
}

void World::loadTextures()
{
  textures.load_now(Textures::Entities, "assets/textures/Entities.png");
  textures.load_now(Textures::Jungle, "assets/textures/Jungle.png");
  textures.load_now(Textures::Explosion, "assets/textures/Explosion.png");
  textures.load_now(Textures::Particle, "assets/textures/Particle.png");
  textures.load_now(Textures::FinishLine, "assets/textures/FinishLine.png");
}

void World::adaptPlayerPosition()
{
  // Keep player's position inside the screen bounds, at least borderDistance
  // units from the border
  sf::FloatRect viewBounds = getViewBounds();
  const float borderDistance = 40.f;

  FOREACH(Aircraft* aircraft, playerAircrafts)
  {
    sf::Vector2f position = aircraft->getPosition();
    position.x = std::max(position.x, viewBounds.left + borderDistance);
    position.x = std::min(position.x, viewBounds.left + viewBounds.width - borderDistance);
    position.y = std::max(position.y, viewBounds.top + borderDistance);
    position.y = std::min(position.y, viewBounds.top + viewBounds.height - borderDistance);
    aircraft->setPosition(position);
  }
}

void World::adaptPlayerVelocity()
{
  FOREACH(Aircraft* aircraft, playerAircrafts)
  {
    sf::Vector2f velocity = aircraft->getVelocity();

    // If moving diagonally, reduce velocity (to have always the same velocity)
    if(velocity.x != 0.f && velocity.y != 0.f)
      aircraft->setVelocity(velocity / std::sqrt(2.f));

    // Add scrolling velocity
    aircraft->accelerate(0.f, scrollSpeed);
  }
}

bool matchesCategories(SceneNode::Pair& colliders, Category::Type type1, Category::Type type2)
{
  unsigned int category1 = colliders.first->getCategory();
  unsigned int category2 = colliders.second->getCategory();

  // Make sure first pair entry has category type1 and second has type2
  if(type1 & category1 && type2 & category2)
  {
    return true;
  }
  else if(type1 & category2 && type2 & category1)
  {
    std::swap(colliders.first, colliders.second);
    return true;
  }
  else
  {
    return false;
  }
}

void World::handleCollisions()
{
  std::set<SceneNode::Pair> collisionPairs;
  sceneGraph.checkSceneCollision(sceneGraph, collisionPairs);

  FOREACH(SceneNode::Pair pair, collisionPairs)
  {
    if(matchesCategories(pair, Category::PlayerAircraft, Category::EnemyAircraft))
    {
      auto& player = static_cast<Aircraft&>(*pair.first);
      auto& enemy = static_cast<Aircraft&>(*pair.second);

      // Collision: Player damage = enemy's remaining HP
      player.damage(enemy.getHitpoints());
      enemy.destroy();
    }
    else if(matchesCategories(pair, Category::PlayerAircraft, Category::Pickup))
    {
      auto& player = static_cast<Aircraft&>(*pair.first);
      auto& pickup = static_cast<Pickup&>(*pair.second);

      // Apply pickup effect to player, destroy projectile
      pickup.apply(player);
      pickup.destroy();
      player.playLocalSound(commandQueue, SoundEffect::CollectPickup);
    }
    else if(matchesCategories(pair, Category::EnemyAircraft, Category::AlliedProjectile) ||
          matchesCategories(pair, Category::PlayerAircraft, Category::EnemyProjectile))
    {
      auto& aircraft = static_cast<Aircraft&>(*pair.first);
      auto& projectile = static_cast<Projectile&>(*pair.second);

      // Apply projectile damage to aircraft, destroy projectile
      aircraft.damage(projectile.getDamage());
      projectile.destroy();
    }
  }
}

void World::updateSounds()
{
  sf::Vector2f listenerPosition;

  // 0 players (multiplayer mode, until server is connected) -> view center
  if(playerAircrafts.empty())
  {
    listenerPosition = worldView.getCenter();
  }
  // 1 or more players -> mean position between all aircrafts
  else
  {
    FOREACH(Aircraft* aircraft, playerAircrafts)
      listenerPosition += aircraft->getWorldPosition();

    listenerPosition /= static_cast<float>(playerAircrafts.size());
  }

  // Set listener's position
  sounds.setListenerPosition(listenerPosition);

  // Remove unused sounds
  sounds.removeStoppedSounds();
}

void World::buildScene()
{
  // Initialize the different layers
  for(std::size_t i=0; i<LayerCount; ++i)
  {
    Category::Type category = (i == LowerAir) ? Category::SceneAirLayer : Category::None;

    SceneNode::Ptr layer(new SceneNode(category));
    sceneLayers[i] = layer.get();

    sceneGraph.attachChild(std::move(layer));
  }

  // Prepare the tiled background
  sf::Texture& jungleTexture = textures.get(Textures::Jungle);
  jungleTexture.setRepeated(true);

  float viewHeight = worldView.getSize().y;
  sf::IntRect textureRect(worldBounds);
  textureRect.height += static_cast<int>(viewHeight);

  // Add the background sprite to the scene
  std::unique_ptr<SceneNode> jungleSprite(new SpriteNode(jungleTexture, textureRect));
  jungleSprite->setPosition(worldBounds.left, worldBounds.top - viewHeight);
  sceneLayers[Background]->attachChild(std::move(jungleSprite));

  // Add the finish line to the scene
  sf::Texture& finishTexture = textures.get(Textures::FinishLine);
  std::unique_ptr<SpriteNode> finishSprite(new SpriteNode(finishTexture));
  finishSprite->setPosition(0.f, -76.f);
  sceneLayers[Background]->attachChild(std::move(finishSprite));

  // Add smoke particle node to the scene
  std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(Particle::Smoke, textures));
  sceneLayers[LowerAir]->attachChild(std::move(smokeNode));

  // Add propellant particle node to the scene
  std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(Particle::Propellant, textures));
  sceneLayers[LowerAir]->attachChild(std::move(propellantNode));

  // Add sound effect node
  std::unique_ptr<SoundNode> soundNode(new SoundNode(sounds));
  sceneGraph.attachChild(std::move(soundNode));

  // Add network node, if necessary
  if(networkedWorld)
  {
    std::unique_ptr<NetworkNode> networkNode(new NetworkNode());
    this->networkNode = networkNode.get();
    sceneGraph.attachChild(std::move(networkNode));
  }

  // Add enemy aircraft
  addEnemies();
}

void World::addEnemies()
{
  if(networkedWorld)
    return;

  addEnemy(Aircraft::Raptor, 0.f, 500.f);
  addEnemy(Aircraft::Raptor, 0.f, 1000.f);
  addEnemy(Aircraft::Raptor, 100.f, 1100.f);
  addEnemy(Aircraft::Raptor, -100.f, 1100.f);
  addEnemy(Aircraft::Avenger, 70.f, 1500.f);
  addEnemy(Aircraft::Avenger, -70.f, 1500.f);
  addEnemy(Aircraft::Avenger, 70.f, 1710.f);
  addEnemy(Aircraft::Avenger, -70.f, 1710.f);
  addEnemy(Aircraft::Avenger, 30.f, 1850.f);
  addEnemy(Aircraft::Raptor, 300.f, 2200.f);
  addEnemy(Aircraft::Raptor, -300.f, 2200.f);
  addEnemy(Aircraft::Raptor, 0.f, 2200.f);
  addEnemy(Aircraft::Raptor, 0.f, 2500.f);
  addEnemy(Aircraft::Avenger, -300.f, 2700.f);
  addEnemy(Aircraft::Avenger, -300.f, 2700.f);
  addEnemy(Aircraft::Raptor, 0.f, 3000.f);
  addEnemy(Aircraft::Raptor, 250.f, 3250.f);
  addEnemy(Aircraft::Raptor, -250.f, 3250.f);
  addEnemy(Aircraft::Avenger, 0.f, 3500.f);
  addEnemy(Aircraft::Avenger, 0.f, 3700.f);
  addEnemy(Aircraft::Raptor, 0.f, 3800.f);
  addEnemy(Aircraft::Avenger, 0.f, 4000.f);
  addEnemy(Aircraft::Avenger, -200.f, 4200.f);
  addEnemy(Aircraft::Raptor, 200.f, 4200.f);
  addEnemy(Aircraft::Raptor, 0.f, 4400.f);

  sortEnemies();
}

void World::sortEnemies()
{
  // Sort all enemies according to their y value, such that lower enemies are
  // checked first for spawning
  std::sort(enemySpawnPoints.begin(), enemySpawnPoints.end(),
      [] (SpawnPoint lhs, SpawnPoint rhs)
    {
      return lhs.y < rhs.y;
    });
}

void World::addEnemy(Aircraft::Type type, float relX, float relY)
{
  SpawnPoint spawn(type, spawnPosition.x + relX, spawnPosition.y - relY);
  enemySpawnPoints.push_back(spawn);
}

void World::spawnEnemies()
{
  // Spawn all enemies entering the view area (including distance) this frame
  while(!enemySpawnPoints.empty() &&
      enemySpawnPoints.back().y > getBattlefieldBounds().top)
  {
    SpawnPoint spawn = enemySpawnPoints.back();

    std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.type, textures, fonts));
    enemy->setPosition(spawn.x, spawn.y);
    enemy->setRotation(180.f);
    if(networkedWorld)
      enemy->disablePickups();

    sceneLayers[UpperAir]->attachChild(std::move(enemy));

    // Enemy is spawned, remove from the list to spawn
    enemySpawnPoints.pop_back();
  }
}

void World::destroyEntitiesOutsideView()
{
  Command command;
  command.category = Category::Projectile | Category::EnemyAircraft;
  command.action = derivedAction<Entity>([this] (Entity& e, sf::Time)
      {
        if(!getBattlefieldBounds().intersects(e.getBoundingRect()))
          e.remove();
      });

  commandQueue.push(command);
}

void World::guideMissiles()
{
  // Setup command that stores all enemies in activeEnemies
  Command enemyCollector;
  enemyCollector.category = Category::EnemyAircraft;
  enemyCollector.action = derivedAction<Aircraft>(
      [this] (Aircraft& enemy, sf::Time)
      {
        if(!enemy.isDestroyed())
          activeEnemies.push_back(&enemy);
      });

  // Setup command that guides all missiles to the enemy which is currently
  // closest to the player
  Command missileGuider;
  missileGuider.category = Category::AlliedProjectile;
  missileGuider.action = derivedAction<Projectile>(
      [this] (Projectile& missile, sf::Time)
      {
        // Ignore unguided bullets
        if(!missile.isGuided())
          return;

        float minDistance = std::numeric_limits<float>::max();
        Aircraft* closestEnemy = nullptr;

        // Find closest enemy
        FOREACH(Aircraft* enemy, activeEnemies)
        {
          float enemyDistance = distance(missile, *enemy);

          if(enemyDistance < minDistance)
          {
            closestEnemy = enemy;
            minDistance = enemyDistance;
          }
        }

        if(closestEnemy)
          missile.guideTowards(closestEnemy->getWorldPosition());
      });

  // Push commands, reset active enemies
  commandQueue.push(enemyCollector);
  commandQueue.push(missileGuider);
  activeEnemies.clear();
}

sf::FloatRect World::getViewBounds() const
{
  return sf::FloatRect(worldView.getCenter() - worldView.getSize() / 2.f,
      worldView.getSize());
}

sf::FloatRect World::getBattlefieldBounds() const
{
  // Return view bounds + some area at top, where enemies spawn
  sf::FloatRect bounds = getViewBounds();
  bounds.top -= 100.f;
  bounds.height += 100.f;

  return bounds;
}
