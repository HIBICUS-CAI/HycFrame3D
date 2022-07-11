#include "Hyc3DCommon.h"

#include "ConnectionManager.h"
#include "ObjectFactory.h"
#include "RootSystem.h"
#include "SceneManager.h"
#include "SystemExecutive.h"

#include <ID_Interface.h>
#include <WM_Interface.h>

RootSystem::RootSystem()
    : SceneManagerPtr(nullptr), ObjectFactoryPtr(nullptr),
      SystemExecutivePtr(nullptr), Timer({}) {}

RootSystem::~RootSystem() {}

bool RootSystem::startUp(HINSTANCE Instance, int CmdShow) {
  ConnectionManager::createInstance();
  if (!ConnectionManager::instance()->initConnections()) {
    return false;
  }

  if (!window::startUp()) {
    P_LOG(LOG_ERROR, "failed to start up windows system");
    return false;
  }
  if (!window::createWindow("a game wnd dont know how to name",
#ifdef _DEBUG
                            Instance, CmdShow, 1280, 720, false))
#else
                            Instance, CmdShow, 1280, 720, true))
#endif // _DEBUG
  {
    P_LOG(LOG_ERROR, "failed to create window");
    return false;
  }
  if (!input::startUp()) {
    P_LOG(LOG_ERROR, "failed to start up input system");
    return false;
  }

  SceneManagerPtr = new SceneManager();
  ObjectFactoryPtr = new ObjectFactory();
  SystemExecutivePtr = new SystemExecutive();
  assert(SceneManagerPtr && ObjectFactoryPtr && SystemExecutivePtr);

  if (!SceneManagerPtr->startUp(ObjectFactoryPtr)) {
    P_LOG(LOG_ERROR, "cannot init scene manager correctly");
    return false;
  }
  if (!SystemExecutivePtr->startUp(SceneManagerPtr)) {
    P_LOG(LOG_ERROR, "cannot init system executive correctly");
    return false;
  }
  if (!ObjectFactoryPtr->startUp(SceneManagerPtr)) {
    P_LOG(LOG_ERROR, "cannot init object factory correctly");
    return false;
  }
  if (!SceneManagerPtr->deferedStartUp()) {
    P_LOG(LOG_ERROR, "cannot parse entry scene correctly");
    return false;
  }

  return true;
}

void RootSystem::cleanAndStop() {
  SceneManagerPtr->cleanAndStop();
  ObjectFactoryPtr->cleanAndStop();
  SystemExecutivePtr->cleanAndStop();
  delete SceneManagerPtr;
  SceneManagerPtr = nullptr;
  delete ObjectFactoryPtr;
  ObjectFactoryPtr = nullptr;
  delete SystemExecutivePtr;
  SystemExecutivePtr = nullptr;

  input::cleanAndStop();
  window::cleanAndStop();

  ConnectionManager::instance()->terminateConnections();
  ConnectionManager::instance()->destoryInstance();
}

void RootSystem::runGameLoop() {
  MSG Msg = {0};
  while (WM_QUIT != Msg.message) {
    if (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
    } else {
      Timer.timeIn();

      SceneManagerPtr->checkLoadStatus();
      SystemExecutivePtr->runAllSystems(Timer);
      ConnectionManager::instance()->executeConnections();

      Timer.timeOut();
    }
  }
}
