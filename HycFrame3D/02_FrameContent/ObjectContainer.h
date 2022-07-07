#pragma once

#include "Hyc3DCommon.h"

#include "ActorObject.h"
#include "UiObject.h"

#include <string>
#include <unordered_map>
#include <vector>

class ObjectContainer {
private:
  class SceneNode &SceneNodeOwner;

  std::unordered_map<std::string, ActorObject> ActorObjectMap;
  std::unordered_map<std::string, UiObject> UiObjectMap;

  std::vector<ActorObject> NewActorObjectArray;
  std::vector<UiObject> NewUiObjectArray;
  std::vector<ActorObject> DeadActorObjectArray;
  std::vector<UiObject> DeadUiObjectArray;

public:
  ObjectContainer(class SceneNode &SceneNode);
  ~ObjectContainer();

  ActorObject *getActorObject(const std::string &ActorName);
  void addActorObject(const ActorObject &NewActor);
  void deleteActorObject(const std::string &ActorName);
  UiObject *getUiObject(const std::string &UiName);
  void addUiObject(const UiObject &NewUi);
  void deleteUiObject(const std::string &UiName);

  void deleteAllActor();
  void deleteAllUi();

  void initAllNewObjects();
  void deleteAllDeadObjects();
};
