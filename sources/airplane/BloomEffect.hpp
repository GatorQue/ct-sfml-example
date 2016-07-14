#ifndef SOURCES_SCOUT_BLOOMEFFECT_HPP_
#define SOURCES_SCOUT_BLOOMEFFECT_HPP_

#include "PostEffect.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Shader.hpp>

#include <array>

class BloomEffect : public PostEffect
{
  public:
    BloomEffect();

    virtual void apply(const sf::RenderTexture& input, sf::RenderTarget& output);

  private:
    typedef std::array<sf::RenderTexture, 2> RenderTextureArray;
    ShaderHolder shaders;
    sf::RenderTexture brightnessTexture;
    RenderTextureArray firstPassTextures;
    RenderTextureArray secondPassTextures;

    void prepareTextures(sf::Vector2u size);

    void filterBright(const sf::RenderTexture& input, sf::RenderTexture& output);
    void blurMultipass(RenderTextureArray& renderTextures);
    void blur(const sf::RenderTexture& input, sf::RenderTexture& output,
        sf::Vector2f offsetFactor);
    void downsample(const sf::RenderTexture& input, sf::RenderTexture& output);
    void add(const sf::RenderTexture& source, const sf::RenderTexture& bloom,
        sf::RenderTarget& target);

};

#endif
