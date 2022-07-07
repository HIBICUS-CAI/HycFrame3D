#include "SceneManager.h"

#include "ObjectContainer.h"
#include "ObjectFactory.h"
#include "SceneNode.h"

#include <TextUtility.h>

#include <thread>

SceneManager::SceneManager()
    : ObjectFactoryPtr(nullptr), LoadingScenePtr(nullptr),
      CurrentScenePtr(nullptr), NextScenePtr(nullptr), LoadNewSceneFlag(false),
      NewSceneInfo({"", ""}), SceneSwitchFlag(false) {}

SceneManager::~SceneManager() {}

bool SceneManager::startUp(ObjectFactory *ObjectFactory) {
  if (!ObjectFactory) {
    P_LOG(LOG_ERROR, "invalid object factory pointer\n");
    return false;
  }

  ObjectFactoryPtr = ObjectFactory;

  LoadingScenePtr = new SceneNode("temp-loading-scene", this);

  if (!LoadingScenePtr) {
    P_LOG(LOG_ERROR, "fail to create temp loading scene\n");
    return false;
  }

  CurrentScenePtr = LoadingScenePtr;

  return true;
}

bool SceneManager::deferedStartUp() {
  LoadingScenePtr->releaseScene();
  delete LoadingScenePtr;
  if (!loadLoadingScene()) {
    P_LOG(LOG_ERROR, "failed to load loading scene\n");
    return false;
  }

  using namespace hyc;
  using namespace hyc::text;
  TomlNode EntryInfo = {};
  std::string ErrorMess = "";
  if (!loadTomlAndParse(EntryInfo,
                        ".\\Assets\\Configs\\scene-entry-config.toml",
                        ErrorMess)) {
    P_LOG(LOG_ERROR, "failed to parse entry scene config : %s\n",
          ErrorMess.c_str());
    return false;
  }

  loadSceneNode(getAs<std::string>(EntryInfo["entry-scene"]["name"]),
                getAs<std::string>(EntryInfo["entry-scene"]["file"]));

  return true;
}

void SceneManager::cleanAndStop() {
  if (CurrentScenePtr != LoadingScenePtr) {
    CurrentScenePtr->releaseScene();
    delete CurrentScenePtr;
  }
  releaseLoadingScene();
}

void SceneManager::loadSceneNode(const std::string &Name,
                                 const std::string &File) {
  LoadNewSceneFlag = true;
  NewSceneInfo[0] = Name;
  NewSceneInfo[1] = ".\\Assets\\Scenes\\" + File;
}

void SceneManager::checkLoadStatus() {
  if (SceneSwitchFlag) {
    SceneSwitchFlag = false;
  }

  if (LoadNewSceneFlag) {
    LoadNewSceneFlag = false;
    SceneSwitchFlag = true;
    SceneNode *NeedRelScenePtr = nullptr;
    if (CurrentScenePtr != LoadingScenePtr) {
      NeedRelScenePtr = CurrentScenePtr;
      CurrentScenePtr = LoadingScenePtr;
    }

    std::thread LoadThread(&SceneManager::loadNextScene, this, NeedRelScenePtr);
    LoadThread.detach();
  }

  if (NextScenePtr && (CurrentScenePtr == LoadingScenePtr)) {
    SceneSwitchFlag = true;
    CurrentScenePtr = NextScenePtr;
    NextScenePtr = nullptr;
  }
}

ObjectFactory *SceneManager::getObjectFactory() const {
  return ObjectFactoryPtr;
}

SceneNode *SceneManager::getCurrentSceneNode() const { return CurrentScenePtr; }

bool SceneManager::loadLoadingScene() {
  LoadingScenePtr = ObjectFactoryPtr->createSceneNode(
      "loading-scene", ".\\Assets\\Scenes\\loading-scene.json");
  CurrentScenePtr = LoadingScenePtr;

  return (LoadingScenePtr ? true : false);
}

void SceneManager::releaseLoadingScene() {
  LoadingScenePtr->releaseScene();
  delete LoadingScenePtr;
}

void SceneManager::loadNextScene(SceneNode *RelScene) {
  if (RelScene) {
    RelScene->releaseScene();
    delete RelScene;
  }

  NextScenePtr =
      ObjectFactoryPtr->createSceneNode(NewSceneInfo[0], NewSceneInfo[1]);
}

bool SceneManager::getSceneSwitchFlg() const { return SceneSwitchFlag; }
