#pragma once

#include "Hyc3DCommon.h"

#include <array>
#include <string>

class SceneManager {
private:
  class ObjectFactory *ObjectFactoryPtr;

  class SceneNode *LoadingScenePtr;
  class SceneNode *CurrentScenePtr;
  class SceneNode *NextScenePtr;

  bool LoadNewSceneFlag;
  std::array<std::string, 2> NewSceneInfo;

  bool SceneSwitchFlag;

public:
  SceneManager();
  ~SceneManager();

  bool
  startUp(class ObjectFactory *ObjectFactory);
  bool
  deferedStartUp();
  void
  cleanAndStop();

  void
  loadSceneNode(const std::string &Name, const std::string &File);
  void
  checkLoadStatus();

  class ObjectFactory *
  getObjectFactory() const;
  class SceneNode *
  getCurrentSceneNode() const;

  bool
  getSceneSwitchFlg() const;

private:
  bool
  loadLoadingScene();
  void
  releaseLoadingScene();

  void
  loadNextScene(class SceneNode *RelScene);
};
