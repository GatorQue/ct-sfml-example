#include "LoadingTask.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

LoadingTask::LoadingTask(State::Context context) :
  context(context),
  thread(&LoadingTask::runTask, this),
  pendingTotal(0),
  pendingCount(0),
  mutex()
{
}

void LoadingTask::execute()
{
  pendingTotal = context.fonts->getPendingCount();
  pendingTotal += context.textures->getPendingCount();
  pendingCount = 0;
  thread.launch();
}

bool LoadingTask::isFinished()
{
  sf::Lock lock(mutex);
  return pendingCount == pendingTotal;
}

float LoadingTask::getCompletion()
{
  sf::Lock lock(mutex);
  
  return pendingTotal ? pendingCount / pendingTotal : 1.f;
}

void LoadingTask::runTask()
{
  while(!isFinished())
  {
    // Load fonts first
    if(context.fonts->getPendingCount() > 0)
    {
      sf::Lock lock(mutex);
      context.fonts->loadNextPending();
      ++pendingCount;
    }
    // Load textures next
    else if(context.textures->getPendingCount() > 0)
    {
      sf::Lock lock(mutex);
      context.textures->loadNextPending();
      ++pendingCount;
    }
  }
}