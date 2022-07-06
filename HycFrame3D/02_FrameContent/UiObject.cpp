#include "UiObject.h"

#include "ComponentContainer.h"
#include "SceneNode.h"
#include "UiComponent.h"

UiObject::UiObject(const std::string &UiName, SceneNode &SceneNode)
    : Object(UiName, SceneNode), UiCompMap({}) {}

UiObject::~UiObject() {}

void
UiObject::addUComponent(COMP_TYPE CompType) {
  std::string CompName = getObjectName();

  switch (CompType) {
  case COMP_TYPE::U_TRANSFORM:
    CompName += "-transform";
    break;
  case COMP_TYPE::U_SPRITE:
    CompName += "-sprite";
    break;
  case COMP_TYPE::U_ANIMATE:
    CompName += "-animate";
    break;
  case COMP_TYPE::U_TIMER:
    CompName += "-timer";
    break;
  case COMP_TYPE::U_INPUT:
    CompName += "-input";
    break;
  case COMP_TYPE::U_INTERACT:
    CompName += "-interact";
    break;
  case COMP_TYPE::U_BUTTON:
    CompName += "-button";
    break;
  case COMP_TYPE::U_AUDIO:
    CompName += "-audio";
    break;
  default:
    break;
  }

  UiCompMap.insert({CompType, CompName});
}

bool
UiObject::init() {
  auto CompContainer = getSceneNode().GetComponentContainer();

  for (auto &CompInfo : UiCompMap) {
    auto Comp = CompContainer->getComponent(CompInfo.second);
#ifdef _DEBUG
    assert(Comp);
#endif // _DEBUG
    static_cast<UiComponent *>(Comp)->resetUiOwner(this);
    if (!Comp->init()) {
      return false;
    }
    Comp->setCompStatus(STATUS::ACTIVE);
  }

  setObjectStatus(STATUS::ACTIVE);

  return true;
}

void
UiObject::destory() {
  auto CompContainer = getSceneNode().GetComponentContainer();

  for (auto &CompInfo : UiCompMap) {
    auto Comp = CompContainer->getComponent(CompInfo.second);
#ifdef _DEBUG
    assert(Comp);
#endif // _DEBUG
    static_cast<UiComponent *>(Comp)->resetUiOwner(this);
    Comp->destory();
    Comp->setCompStatus(STATUS::NEED_DESTORY);
    CompContainer->deleteComponent(CompInfo.first, CompInfo.second);
  }

  UiCompMap.clear();
}

void
UiObject::syncStatusToAllComps() {
  auto CompContainer = getSceneNode().GetComponentContainer();

  for (auto &CompInfo : UiCompMap) {
    auto Comp = CompContainer->getComponent(CompInfo.second);
#ifdef _DEBUG
    assert(Comp);
#endif // _DEBUG
    Comp->setCompStatus(getObjectStatus());
  }
}
