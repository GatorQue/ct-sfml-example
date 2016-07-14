#ifndef SOURCES_SCOUT_COMMANDQUEUE_HPP_
#define SOURCES_SCOUT_COMMANDQUEUE_HPP_

#include "Command.hpp"

#include <queue>

class CommandQueue
{
  public:
    void push(const Command& command);
    Command pop();
    bool isEmpty() const;

  private:
    std::queue<Command> queue;
};

#endif
