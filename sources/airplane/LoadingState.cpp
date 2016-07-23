#include "LoadingState.hpp"
#include "ResourceHolder.hpp"
#include "WindowUtils.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

LoadingState::LoadingState(StateStack& stack, Context context) :
    State(stack, context),
    target(*context.target),
    loadingText(),
    progressBarBackground(),
    progressBar(),
    loadingTask(context)
{
  sf::Vector2f viewSize = target.getView().getSize();
  
  sf::Font& font = context.fonts->get(Fonts::Main);
  loadingText.setFont(font);
  loadingText.setString("Loading Resources");
  centerOrigin(loadingText);
  loadingText.setPosition(viewSize.x / 2.f, viewSize.y / 2.f + 50.f);
  
  progressBarBackground.setFillColor(sf::Color::White);
  progressBarBackground.setSize(sf::Vector2f(viewSize.x - 20, 10));
  progressBarBackground.setPosition(10, loadingText.getPosition().y + 40);
  
  progressBar.setFillColor(sf::Color(100,100,100));
  progressBar.setSize(sf::Vector2f(200, 10));
  progressBar.setPosition(10, loadingText.getPosition().y + 40);
  
  setCompletion(0.f);
  
  loadingTask.execute();
  printf("LoadingState\n");
}

void LoadingState::draw()
{
  printf("LoadingState::draw()\n");
  target.setView(target.getDefaultView());
  
  target.draw(loadingText);
  target.draw(progressBarBackground);
  target.draw(progressBar);
}

bool LoadingState::update(sf::Time)
{
  printf("LoadingState::update\n");
  // Update the progress bar from the remote task or finish it
  if(loadingTask.isFinished())
  {
    requestStackPop();
  }
  else
  {
    setCompletion(loadingTask.getCompletion());
  }
  return false;
}

bool LoadingState::handleEvent(const sf::Event& event)
{
  printf("LoadingState::handleEvent\n");
  return false;
}

void LoadingState::setCompletion(float percent)
{
  if(percent > 1.f) // clamp
    percent = 1.f;
    
  progressBar.setSize(sf::Vector2f(progressBarBackground.getSize().x * percent, progressBar.getSize().y));
}
