#include "UTimerComponent.h"

#include "UiObject.h"

UTimerComponent::UTimerComponent(const std::string &CompName, UiObject *UiOwner)
    : UiComponent(CompName, UiOwner), TimerMap({}) {}

UTimerComponent::~UTimerComponent() {}

bool UTimerComponent::init() {
  for (auto &T : TimerMap) {
    T.second.Time = 0.f;
  }

  return true;
}

void UTimerComponent::update(Timer &Timer) {
  float Deltatime = Timer.floatDeltaTime() / 1000.f;
  for (auto &T : TimerMap) {
    if (T.second.ActiveFlag) {
      T.second.Time += Deltatime;
    }
  }
}

void UTimerComponent::destory() { TimerMap.clear(); }

void UTimerComponent::addTimer(const std::string &TimerName) {
  UI_TIMER T = {};
  T.Name = TimerName;
  TimerMap.insert({TimerName, T});
}

void UTimerComponent::startTimer(const std::string &TimerName) {
  if (TimerMap.find(TimerName) != TimerMap.end()) {
    TimerMap[TimerName].ActiveFlag = true;
  }
}

void UTimerComponent::pauseTimer(const std::string &TimerName) {
  if (TimerMap.find(TimerName) != TimerMap.end()) {
    TimerMap[TimerName].ActiveFlag = false;
  }
}

void UTimerComponent::resetTimer(const std::string &TimerName) {
  if (TimerMap.find(TimerName) != TimerMap.end()) {
    TimerMap[TimerName].ActiveFlag = false;
    TimerMap[TimerName].Time = 0.f;
  }
}

UI_TIMER *UTimerComponent::getTimer(const std::string &TimerName) {
  if (TimerMap.find(TimerName) != TimerMap.end()) {
    return &(TimerMap[TimerName]);
  } else {
    P_LOG(LOG_WARNING, "this timer doesnt exist : %s\n", TimerName.c_str());
    return nullptr;
  }
}
