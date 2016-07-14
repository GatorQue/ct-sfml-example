#include "MusicPlayer.hpp"

MusicPlayer::MusicPlayer() :
  music(),
  filenames(),
  volume(100.f)
{
  filenames[Music::MenuTheme] = "assets/music/MenuTheme.ogg";
  filenames[Music::MissionTheme] = "assets/music/MissionTheme.ogg";
}

void MusicPlayer::play(Music::ID theme)
{
  std::string filename = filenames[theme];

  if(!music.openFromFile(filename))
    throw std::runtime_error("Music " + filename + " could not be loaded.");

  music.setVolume(volume);
  music.setLoop(true);
  music.play();
}

void MusicPlayer::stop()
{
  music.stop();
}

void MusicPlayer::setVolume(float volume)
{
  this->volume = volume;
}

void MusicPlayer::setPaused(bool paused)
{
  if(paused)
    music.pause();
  else if(sf::SoundSource::Playing == music.getStatus())
    music.play();
}
