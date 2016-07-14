#ifndef SOURCES_SCOUT_AIRCRAFT_HPP_
#define SOURCES_SCOUT_AIRCRAFT_HPP_

#include "Entity.hpp"
#include "Animation.hpp"
#include "Command.hpp"
#include "Projectile.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/Sprite.hpp>

class TextNode;

class Aircraft : public Entity
{
  public:
    enum Type {
      Eagle,
      Raptor,
      Avenger,
      TypeCount
    };

    explicit Aircraft(Type type, const TextureHolder& textures, const FontHolder& fonts);

    virtual unsigned int getCategory() const;
    virtual sf::FloatRect getBoundingRect() const;
    virtual bool isMarkedForRemoval() const;
    virtual void remove();
    bool isAllied() const;
    float getMaxSpeed() const;
    void disablePickups();

    void increaseFireRate();
    void increaseSpread();
    void collectMissiles(unsigned int count);

    void fire();
    void launchMissile();
    void playLocalSound(CommandQueue& commands, SoundEffect::ID effect);

    int getIdentifier();
    void setIdentifier(int identifier);

    int getMissileAmmo() const;
    void setMissileAmmo(int ammo);

  private:
    Type type;
    sf::Sprite sprite;
    Animation explosion;

    Command fireCommand;
    Command missileCommand;
    Command dropPickupCommand;

    sf::Time fireCountdown;
    bool isFiring;
    bool isLaunchingMissile;
    bool showExplosion;
    bool explosionBegan;
    bool spawnedPickup;
    bool pickupsEnabled;

    int fireRateLevel;
    int spreadLevel;
    int missileAmmo;

    TextNode* healthDisplay;
    TextNode* missileDisplay;

    float travelledDistance;
    std::size_t directionIndex;

    int identifier;

    virtual void drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
    virtual void updateCurrent(sf::Time dt, CommandQueue& commands);
    void updateMovementPattern(sf::Time dt);
    void checkPickupDrop(CommandQueue& commands);
    void checkProjectileLaunch(sf::Time dt, CommandQueue& commands);

    void createBullets(SceneNode& node, const TextureHolder& textures) const;
    void createProjectile(SceneNode& node, Projectile::Type type,
        float xOffset, float yOffset, const TextureHolder& textures) const;
    void createPickup(SceneNode& node, const TextureHolder& textures) const;

    void updateTexts();
    void updateRollAnimation();
};

#endif
