#pragma once

#include "Hyc3DCommon.h"

#include <Windows.h>

class RootSystem {
private:
  class SceneManager *SceneManagerPtr;
  class ObjectFactory *ObjectFactoryPtr;
  class SystemExecutive *SystemExecutivePtr;

  Timer Timer;

public:
  RootSystem();
  ~RootSystem();

  bool startUp(HINSTANCE Instance, int CmdShow);
  void cleanAndStop();
  void runGameLoop();
};
