#ifndef SOURCES_SCOUT_RESOURCEIDENTIFIERS_HPP_
#define SOURCES_SCOUT_RESOURCEIDENTIFIERS_HPP_

// Forward declaration of SFML classes
namespace sf
{
  class Font;
  class Shader;
  class SoundBuffer;
  class Texture;
}

namespace Resources
{
  enum Type
  {
    Font,
    Shader,
    Sound,
    Texture
  };
}

namespace Fonts
{
  enum ID
  {
    Main,
  };
}

namespace Music
{
  enum ID
  {
    MenuTheme,
    MissionTheme,
  };
}

namespace Shaders
{
  enum ID
  {
    BrightnessPass,
    DownSamplePass,
    GaussianBlurPass,
    AddPass,
  };
}

namespace SoundEffect
{
  enum ID
  {
    AlliedGunfire,
    EnemyGunfire,
    Explosion1,
    Explosion2,
    LaunchMissile,
    CollectPickup,
    Button,
  };
}

namespace Textures
{
  enum ID
  {
    Entities,
    Jungle,
    TitleScreen,
    Buttons,
    Explosion,
    Particle,
    FinishLine
  };
}

// Forward declaration and a few type definitions
template <typename Resource, typename Identifier>
class ResourceHolder;

typedef ResourceHolder<sf::Font, Fonts::ID> FontHolder;
typedef ResourceHolder<sf::Shader, Shaders::ID> ShaderHolder;
typedef ResourceHolder<sf::SoundBuffer, SoundEffect::ID> SoundBufferHolder;
typedef ResourceHolder<sf::Texture, Textures::ID> TextureHolder;

#endif
