#include "Aircraft.hpp"
#include "CommandQueue.hpp"
#include "DataTables.hpp"
#include "NetworkNode.hpp"
#include "Pickup.hpp"
#include "ResourceHolder.hpp"
#include "SoundNode.hpp"
#include "StringUtils.hpp"
#include "TextNode.hpp"
#include "MathUtils.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>

namespace
{
  const std::vector<AircraftData> Table = initializeAircraftData();
}

Aircraft::Aircraft(Type type, const TextureHolder& textures, const FontHolder& fonts) :
  Entity(Table[type].hitpoints),
  type(type),
  sprite(textures.get(Table[type].texture), Table[type].textureRect),
  explosion(textures.get(Textures::Explosion)),
  fireCommand(),
  missileCommand(),
  dropPickupCommand(),
  fireCountdown(sf::Time::Zero),
  isFiring(false),
  isLaunchingMissile(false),
  showExplosion(true),
  explosionBegan(false),
  spawnedPickup(false),
  pickupsEnabled(true),
  fireRateLevel(1),
  spreadLevel(1),
  missileAmmo(2),
  healthDisplay(nullptr),
  missileDisplay(nullptr),
  travelledDistance(0.f),
  directionIndex(0),
  identifier(0)
{
  explosion.setFrameSize(sf::Vector2i(256, 256));
  explosion.setNumFrames(16);
  explosion.setDuration(sf::seconds(1));

  centerOrigin(sprite);
  centerOrigin(explosion);

  fireCommand.category = Category::SceneAirLayer;
  fireCommand.action = [this, &textures] (SceneNode& node, sf::Time)
  {
    createBullets(node, textures);
  };

  missileCommand.category = Category::SceneAirLayer;
  missileCommand.action = [this, &textures] (SceneNode& node, sf::Time)
  {
    createProjectile(node, Projectile::Missile, 0.f, 0.5f, textures);
  };

  dropPickupCommand.category = Category::SceneAirLayer;
  dropPickupCommand.action = [this, &textures] (SceneNode& node, sf::Time)
  {
    createPickup(node, textures);
  };

  std::unique_ptr<TextNode> healthDisplay(new TextNode(fonts, ""));
  this->healthDisplay = healthDisplay.get();
  attachChild(std::move(healthDisplay));

  if(getCategory() == Category::PlayerAircraft)
  {
    std::unique_ptr<TextNode> missileDisplay(new TextNode(fonts, ""));
    missileDisplay->setPosition(0, 70);
    this->missileDisplay = missileDisplay.get();
    attachChild(std::move(missileDisplay));
  }

  // Update texts
  updateTexts();
}

int Aircraft::getMissileAmmo() const
{
  return missileAmmo;
}

void Aircraft::setMissileAmmo(int ammo)
{
  missileAmmo = ammo;
}

void Aircraft::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
  if(isDestroyed() && showExplosion)
    target.draw(explosion, states);
  else
    target.draw(sprite, states);
}

void Aircraft::disablePickups()
{
  pickupsEnabled = false;
}

void Aircraft::updateCurrent(sf::Time dt, CommandQueue& commands)
{
  // Update texts and roll animation
  updateTexts();
  updateRollAnimation();

  // Entity has been destroyed: Possibly drop pickup, mark for removal
  if(isDestroyed())
  {
    checkPickupDrop(commands);
    explosion.update(dt);

    // Play explosion sound only once
    if(!explosionBegan)
    {
      // Play sound effect
      SoundEffect::ID soundEffect =
        (randomInt(2) == 0) ?
        SoundEffect::Explosion1 :
        SoundEffect::Explosion2;
      playLocalSound(commands, soundEffect);

      // Emit network game action for enemy explosions
      if(!isAllied())
      {
        sf::Vector2f position = getWorldPosition();

        Command command;
        command.category = Category::Network;
        command.action = derivedAction<NetworkNode>([position] (NetworkNode& node, sf::Time)
            {
              node.notifyGameAction(GameActions::EnemyExplode, position);
            });

        commands.push(command);
      }

      explosionBegan = true;
    }
    return;
  }

  // Check if bullets or missiles are fired
  checkProjectileLaunch(dt, commands);

  // Update enemy movement pattern; apply velocity
  updateMovementPattern(dt);
  Entity::updateCurrent(dt, commands);
}

unsigned int Aircraft::getCategory() const
{
  if(isAllied())
      return Category::PlayerAircraft;
  else
      return Category::EnemyAircraft;
}

sf::FloatRect Aircraft::getBoundingRect() const
{
  return getWorldTransform().transformRect(sprite.getGlobalBounds());
}

bool Aircraft::isMarkedForRemoval() const
{
  return isDestroyed() && (explosion.isFinished() || !showExplosion);
}

void Aircraft::remove()
{
  Entity::remove();
  showExplosion = false;
}

bool Aircraft::isAllied() const
{
  return type == Eagle;
}

float Aircraft::getMaxSpeed() const
{
  return Table[type].speed;
}

void Aircraft::increaseFireRate()
{
  if(fireRateLevel < 10)
    ++fireRateLevel;
}

void Aircraft::increaseSpread()
{
  if(spreadLevel < 3)
    ++spreadLevel;
}

void Aircraft::collectMissiles(unsigned int count)
{
  missileAmmo += count;
}

void Aircraft::fire()
{
  // Only ships with fire interval != 0 are able to fire
  if(Table[type].fireInterval != sf::Time::Zero)
    isFiring = true;
}

void Aircraft::launchMissile()
{
  if(missileAmmo > 0)
  {
    isLaunchingMissile = true;
    --missileAmmo;
  }
}

void Aircraft::playLocalSound(CommandQueue& commands, SoundEffect::ID effect)
{
  sf::Vector2f worldPosition = getWorldPosition();

  Command command;
  command.category = Category::SoundEffect;
  command.action = derivedAction<SoundNode>(
      [effect, worldPosition] (SoundNode& node, sf::Time)
      {
        node.playSound(effect, worldPosition);
      });

  commands.push(command);
}

int Aircraft::getIdentifier()
{
  return identifier;
}

void Aircraft::setIdentifier(int identifier)
{
  this->identifier = identifier;
}

void Aircraft::updateMovementPattern(sf::Time dt)
{
  // Enemy airplane: Movement pattern
  const std::vector<Direction>& directions = Table[type].directions;
  if(!directions.empty())
  {
    // Moved long enough in current direction: Change direction
    if(travelledDistance > directions[directionIndex].distance)
    {
      directionIndex = (directionIndex + 1) % directions.size();
      travelledDistance = 0.f;
    }

    // Compute velocity from direction
    float radians = toRadian(directions[directionIndex].angle + 90.f);
    float vx = getMaxSpeed() * std::cos(radians);
    float vy = getMaxSpeed() * std::sin(radians);

    setVelocity(vx, vy);

    travelledDistance += getMaxSpeed() * dt.asSeconds();
  }
}

void Aircraft::checkPickupDrop(CommandQueue& commands)
{
  if(pickupsEnabled && !isAllied() && randomInt(3) == 0 && !spawnedPickup)
    commands.push(dropPickupCommand);

  spawnedPickup = true;
}

void Aircraft::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
  if(!isAllied())
    fire();

  // Check for automatic gunfire, allow only in intervals
  if(isFiring && fireCountdown <= sf::Time::Zero)
  {
    // Interval expired: We can fire a new bullet
    commands.push(fireCommand);
    playLocalSound(commands, isAllied() ?
        SoundEffect::AlliedGunfire : SoundEffect::EnemyGunfire);

    fireCountdown += sf::seconds(1.f / (fireRateLevel+1));
    isFiring = false;
  }
  else if(fireCountdown > sf::Time::Zero)
  {
    // Interval not expired: Decrease it further
    fireCountdown -= dt;
    isFiring = false;
  }

  // Check for missile launch
  if(isLaunchingMissile)
  {
    commands.push(missileCommand);
    playLocalSound(commands, SoundEffect::LaunchMissile);

    isLaunchingMissile = false;
  }
}

void Aircraft::createBullets(SceneNode& node, const TextureHolder& textures) const
{
  Projectile::Type type = isAllied() ? Projectile::AlliedBullet : Projectile::EnemyBullet;
  switch(spreadLevel)
  {
    case 1:
      createProjectile(node, type, 0.f, 0.5f, textures);
      break;

    case 2:
      createProjectile(node, type, -0.33f, 0.33f, textures);
      createProjectile(node, type, 0.33f, 0.33f, textures);
      break;

    case 3:
      createProjectile(node, type, -0.5f, 0.33f, textures);
      createProjectile(node, type, 0.f, 0.5f, textures);
      createProjectile(node, type, 0.5f, 0.33f, textures);
      break;
  }
}

void Aircraft::createProjectile(SceneNode& node, Projectile::Type type,
    float xOffset, float yOffset, const TextureHolder& textures) const
{
  std::unique_ptr<Projectile> projectile(new Projectile(type, textures));

  sf::Vector2f offset(
      xOffset * sprite.getGlobalBounds().width,
      yOffset * sprite.getGlobalBounds().height);
  sf::Vector2f velocity(0, projectile->getMaxSpeed());

  float sign = isAllied() ? -1.f : 1.f;
  projectile->setPosition(getWorldPosition() + offset * sign);
  projectile->setVelocity(velocity * sign);
  node.attachChild(std::move(projectile));
}

void Aircraft::createPickup(SceneNode& node, const TextureHolder& textures) const
{
  auto type = static_cast<Pickup::Type>(randomInt(Pickup::TypeCount));

  std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
  pickup->setPosition(getWorldPosition());
  pickup->setVelocity(0.f, 1.f);
  node.attachChild(std::move(pickup));
}

void Aircraft::updateTexts()
{
  // Display hitpoints
  if(isDestroyed())
    healthDisplay->setString("");
  else
    healthDisplay->setString(toString(getHitpoints()) + " HP");
  healthDisplay->setPosition(0.f, 50.f);
  healthDisplay->setRotation(-getRotation());

  // Display missiles, if available
  if(missileDisplay)
  {
    if(missileAmmo == 0 || isDestroyed())
      missileDisplay->setString("");
    else
      missileDisplay->setString("M: " + toString(missileAmmo));
  }
}

void Aircraft::updateRollAnimation()
{
  if(Table[type].hasRollAnimation)
  {
    sf::IntRect textureRect = Table[type].textureRect;

    // Roll left: Texture rect offset once
    if(getVelocity().x < 0.f)
      textureRect.left += textureRect.width;

    // Roll right: Texture rect offset twice
    if(getVelocity().x > 0.f)
      textureRect.left += 2 * textureRect.width;

    sprite.setTextureRect(textureRect);
  }
}
