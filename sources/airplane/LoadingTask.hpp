#ifndef SOURCES_SCOUT_LOADINGTASK_HPP_
#define SOURCES_SCOUT_LOADINGTASK_HPP_

#include "State.hpp"

#include <SFML/System/Thread.hpp>
#include <SFML/System/Mutex.hpp>
#include <SFML/System/Lock.hpp>
#include <SFML/System/Clock.hpp>

class LoadingTask
{
  public:
    LoadingTask(State::Context context);
    
    void execute();
    bool isFinished();
    float getCompletion();
    
  private:
    State::Context context;
    sf::Thread thread;
    size_t pendingTotal;
    size_t pendingCount;
    sf::Mutex mutex;

    void runTask();
};

#endif