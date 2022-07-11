#include "ATimerComponent.h"

#include "ActorObject.h"

ATimerComponent::ATimerComponent(const std::string &CompName,
                                 ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), TimerMap({}) {}

ATimerComponent::~ATimerComponent() {}

bool ATimerComponent::init() {
  for (auto &T : TimerMap) {
    T.second.Time = 0.f;
  }

  return true;
}

void ATimerComponent::update(const Timer &Timer) {
  float Deltatime = Timer.floatDeltaTime() / 1000.f;
  for (auto &t : TimerMap) {
    if (t.second.ActiveFlag) {
      t.second.Time += Deltatime;
    }
  }
}

void ATimerComponent::destory() { TimerMap.clear(); }

void ATimerComponent::addTimer(const std::string &TimerName) {
  ACTOR_TIMER T = {};
  T.Name = TimerName;
  TimerMap.insert({TimerName, T});
}

void ATimerComponent::startTimer(const std::string &TimerName) {
  if (TimerMap.find(TimerName) != TimerMap.end()) {
    TimerMap[TimerName].ActiveFlag = true;
  }
}

void ATimerComponent::pauseTimer(const std::string &TimerName) {
  if (TimerMap.find(TimerName) != TimerMap.end()) {
    TimerMap[TimerName].ActiveFlag = false;
  }
}

void ATimerComponent::resetTimer(const std::string &TimerName) {
  if (TimerMap.find(TimerName) != TimerMap.end()) {
    TimerMap[TimerName].ActiveFlag = false;
    TimerMap[TimerName].Time = 0.f;
  }
}

ACTOR_TIMER *ATimerComponent::getTimer(const std::string &TimerName) {
  if (TimerMap.find(TimerName) != TimerMap.end()) {
    return &(TimerMap[TimerName]);
  } else {
    P_LOG(LOG_WARNING, "this timer doesnt exist : {}", TimerName);
    return nullptr;
  }
}
