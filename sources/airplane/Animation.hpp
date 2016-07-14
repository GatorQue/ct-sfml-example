#ifndef SOURCES_SCOUT_ANIMATION_HPP_
#define SOURCES_SCOUT_ANIMATION_HPP_

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>

class Animation : public sf::Drawable, public sf::Transformable
{
  public:
    Animation();
    explicit Animation(const sf::Texture& texture);

    void setTexture(const sf::Texture& texture);
    const sf::Texture* getTexture() const;

    void setFrameSize(sf::Vector2i frameSize);
    sf::Vector2i getFrameSize() const;

    void setNumFrames(std::size_t numFrames);
    std::size_t getNumFrames() const;

    void setDuration(sf::Time duration);
    sf::Time getDuration() const;

    void setRepeating(bool flag);
    bool isRepeating() const;

    void restart();
    bool isFinished() const;

    sf::FloatRect getLocalBounds() const;
    sf::FloatRect getGlobalBounds() const;

    void update(sf::Time dt);

  private:
    sf::Sprite sprite;
    sf::Vector2i frameSize;
    std::size_t numFrames;
    std::size_t currentFrame;
    sf::Time duration;
    sf::Time elapsedTime;
    bool repeat;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

#endif

