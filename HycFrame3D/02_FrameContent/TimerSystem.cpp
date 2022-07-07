#include "TimerSystem.h"

#include "ATimerComponent.h"
#include "ComponentContainer.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SystemExecutive.h"
#include "UTimerComponent.h"

TimerSystem::TimerSystem(SystemExecutive *SysExecutive)
    : System("timer-system", SysExecutive), ATimerArrayPtr(nullptr),
      UTimerArrayPtr(nullptr) {}

TimerSystem::~TimerSystem() {}

bool
TimerSystem::init() {
#ifdef _DEBUG
  assert(getSystemExecutive());
#endif // _DEBUG

  ATimerArrayPtr = static_cast<std::vector<ATimerComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_TIMER));
  UTimerArrayPtr = static_cast<std::vector<UTimerComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::U_TIMER));

  if (!(ATimerArrayPtr && UTimerArrayPtr)) {
    return false;
  }

  return true;
}

void
TimerSystem::run(Timer &Timer) {
  for (auto &Atmc : *ATimerArrayPtr) {
    if (Atmc.getCompStatus() == STATUS::ACTIVE) {
      Atmc.update(Timer);
    }
  }

  for (auto &Utmc : *UTimerArrayPtr) {
    if (Utmc.getCompStatus() == STATUS::ACTIVE) {
      Utmc.update(Timer);
    }
  }
}

void
TimerSystem::destory() {}
