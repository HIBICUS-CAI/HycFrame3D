#pragma once

#include "UiComponent.h"

#include <unordered_map>

struct UI_TIMER {
  std::string Name = "";
  bool ActiveFlag = false;
  float Time = 0.f;

  bool IsGreaterThan(float Value) { return (Time > Value); }
};

class UTimerComponent : public UiComponent {
private:
  std::unordered_map<std::string, UI_TIMER> TimerMap;

public:
  UTimerComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~UTimerComponent();

  UTimerComponent &operator=(const UTimerComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    TimerMap = Source.TimerMap;
    UiComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(const Timer &Timer);
  virtual void destory();

public:
  void addTimer(const std::string &TimerName);
  void startTimer(const std::string &TimerName);
  void pauseTimer(const std::string &TimerName);
  void resetTimer(const std::string &TimerName);
  UI_TIMER *getTimer(const std::string &TimerName);
};
