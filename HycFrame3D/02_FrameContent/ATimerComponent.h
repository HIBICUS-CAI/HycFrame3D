#pragma once

#include "ActorComponent.h"

#include <unordered_map>

struct ACTOR_TIMER {
  std::string Name = "";
  bool ActiveFlag = false;
  float Time = 0.f;

  bool isGreaterThan(float Value) { return (Time > Value); }
};

class ATimerComponent : public ActorComponent {
private:
  std::unordered_map<std::string, ACTOR_TIMER> TimerMap;

public:
  ATimerComponent(const std::string &CompName, class ActorObject *ActorOwner);
  virtual ~ATimerComponent();

  ATimerComponent &operator=(const ATimerComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    TimerMap = Source.TimerMap;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(Timer &Timer);
  virtual void destory();

public:
  void addTimer(const std::string &TimerName);
  void startTimer(const std::string &TimerName);
  void pauseTimer(const std::string &TimerName);
  void resetTimer(const std::string &TimerName);
  ACTOR_TIMER *getTimer(const std::string &TimerName);
};
