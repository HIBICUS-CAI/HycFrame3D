#include "ObjectContainer.h"

#include "SceneNode.h"

ObjectContainer::ObjectContainer(SceneNode &SceneNode)
    : SceneNodeOwner(SceneNode), ActorObjectMap({}), UiObjectMap({}),
      NewActorObjectArray({}), NewUiObjectArray({}), DeadActorObjectArray({}),
      DeadUiObjectArray({}) {
  (void)SceneNodeOwner;
}

ObjectContainer::~ObjectContainer() {}

ActorObject *
ObjectContainer::getActorObject(const std::string &ActorName) {
  auto Found = ActorObjectMap.find(ActorName);
  if (Found != ActorObjectMap.end()) {
    return &(Found->second);
  } else {
    P_LOG(LOG_WARNING, "cannot find actor object name : %s\n",
          ActorName.c_str());
    return nullptr;
  }
}

void
ObjectContainer::addActorObject(const ActorObject &NewActor) {
  NewActorObjectArray.emplace_back(NewActor);
}

void
ObjectContainer::deleteActorObject(const std::string &ActorName) {
  auto Found = ActorObjectMap.find(ActorName);
  if (Found != ActorObjectMap.end()) {
    DeadActorObjectArray.emplace_back(Found->second);
    ActorObjectMap.erase(ActorName);
  } else {
    P_LOG(LOG_WARNING, "cannot find actor object name : %s\n",
          ActorName.c_str());
  }
}

UiObject *
ObjectContainer::getUiObject(const std::string &UiName) {
  auto Found = UiObjectMap.find(UiName);
  if (Found != UiObjectMap.end()) {
    return &(Found->second);
  } else {
    P_LOG(LOG_WARNING, "cannot find ui object name : %s\n", UiName.c_str());
    return nullptr;
  }
}

void
ObjectContainer::addUiObject(const UiObject &NewUi) {
  NewUiObjectArray.emplace_back(NewUi);
}

void
ObjectContainer::deleteUiObject(const std::string &UiName) {
  auto Found = UiObjectMap.find(UiName);
  if (Found != UiObjectMap.end()) {
    DeadUiObjectArray.emplace_back(Found->second);
    UiObjectMap.erase(UiName);
  } else {
    P_LOG(LOG_WARNING, "cannot find ui object name : %s\n", UiName.c_str());
  }
}

void
ObjectContainer::deleteAllActor() {
  for (auto &Actor : ActorObjectMap) {
    DeadActorObjectArray.emplace_back(Actor.second);
  }
  ActorObjectMap.clear();
}

void
ObjectContainer::deleteAllUi() {
  for (auto &Ui : UiObjectMap) {
    DeadUiObjectArray.emplace_back(Ui.second);
  }
  UiObjectMap.clear();
}

void
ObjectContainer::initAllNewObjects() {
  for (auto &NewActor : NewActorObjectArray) {
    const std::string &Name = NewActor.getObjectName();
    ActorObjectMap.insert({Name, NewActor});
    auto &Actor = ActorObjectMap.find(Name)->second;
    bool Result = Actor.init();
#ifdef _DEBUG
    assert(Result);
#endif // _DEBUG
    (void)Result;
  }
  NewActorObjectArray.clear();

  for (auto &NewUi : NewUiObjectArray) {
    const std::string &Name = NewUi.getObjectName();
    UiObjectMap.insert({Name, NewUi});
    auto &Ui = UiObjectMap.find(Name)->second;
    bool Result = Ui.init();
#ifdef _DEBUG
    assert(Result);
#endif // _DEBUG
    (void)Result;
  }
  NewUiObjectArray.clear();
}

void
ObjectContainer::deleteAllDeadObjects() {
  for (auto &DeadActor : DeadActorObjectArray) {
    DeadActor.destory();
  }
  DeadActorObjectArray.clear();

  for (auto &DeadUi : DeadUiObjectArray) {
    DeadUi.destory();
  }
  DeadUiObjectArray.clear();
}
