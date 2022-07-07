#pragma once

#include "Hyc3DCommon.h"

#include <vector>

class SystemExecutive {
private:
  class SceneManager *SceneManagerPtr;
  class SceneNode *CurrentSceneNode;

  std::vector<class System *> SystemsArray;

public:
  SystemExecutive();
  ~SystemExecutive();

  bool
  startUp(class SceneManager *SceneManager);
  void
  cleanAndStop();
  void
  runAllSystems(Timer &Timer);

  class SceneManager *
  getSceneManager() const;

private:
  bool
  initAllSystem();
  void
  checkCurrentScene();
};
