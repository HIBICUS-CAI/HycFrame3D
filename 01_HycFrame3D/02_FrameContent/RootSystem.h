#pragma once

#include "Hyc3DCommon.h"

class RootSystem {
private:
  class SceneManager *SceneManagerPtr;
  class ObjectFactory *ObjectFactoryPtr;
  class SystemExecutive *SystemExecutivePtr;

  ::Timer Timer;

public:
  RootSystem();
  ~RootSystem();

  bool startUp();
  void cleanAndStop();
  void runGameLoop();
};
