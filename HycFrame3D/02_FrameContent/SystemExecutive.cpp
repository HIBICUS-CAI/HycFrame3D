#include "SystemExecutive.h"

#include "AnimationSystem.h"
#include "AudioSystem.h"
#include "ButtonSystem.h"
#include "CollisionSystem.h"
#include "InputSystem.h"
#include "InstanceSystem.h"
#include "InteractSystem.h"
#include "ObjectContainer.h"
#include "RenderSystem.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "System.h"
#include "TimerSystem.h"

SystemExecutive::SystemExecutive()
    : SceneManagerPtr(nullptr), CurrentSceneNode(nullptr), SystemsArray({}) {}

SystemExecutive::~SystemExecutive() {}

bool SystemExecutive::startUp(SceneManager *SceneManager) {
  if (!SceneManager) {
    P_LOG(LOG_ERROR, "invalid scene manager pointer\n");
    return false;
  }

  SceneManagerPtr = SceneManager;
  CurrentSceneNode = SceneManager->getCurrentSceneNode();

  System *Sys = nullptr;

  Sys = new InputSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);
  Sys = new CollisionSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);
  Sys = new ButtonSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);
  Sys = new InteractSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);
  Sys = new InstanceSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);
  Sys = new TimerSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);
  Sys = new AudioSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);
  Sys = new AnimationSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);
  Sys = new RenderSystem(this);
  if (!Sys) {
    return false;
  }
  SystemsArray.push_back(Sys);

  return initAllSystem();
}

void SystemExecutive::cleanAndStop() {
  for (auto &Sys : SystemsArray) {
    Sys->destory();
    delete Sys;
  }
  SystemsArray.clear();
}

void SystemExecutive::runAllSystems(Timer &Timer) {
  checkCurrentScene();

  CurrentSceneNode->getObjectContainer()->deleteAllDeadObjects();
  CurrentSceneNode->getObjectContainer()->initAllNewObjects();

  for (const auto &Sys : SystemsArray) {
    Sys->run(Timer);
  }
}

SceneManager *SystemExecutive::getSceneManager() const {
#ifdef _DEBUG
  assert(SceneManagerPtr);
#endif // _DEBUG
  return SceneManagerPtr;
}

bool SystemExecutive::initAllSystem() {
  for (auto &Sys : SystemsArray) {
    if (!Sys->init()) {
      return false;
    }
  }
  return true;
}

void SystemExecutive::checkCurrentScene() {
  if (SceneManagerPtr->getSceneSwitchFlg()) {
    CurrentSceneNode = SceneManagerPtr->getCurrentSceneNode();
    bool Result = initAllSystem();
#ifdef _DEBUG
    assert(Result && "init all system failed");
#endif // _DEBUG
    (void)Result;
  }
}
