#include "ActorObject.h"

#include "ActorComponent.h"
#include "ComponentContainer.h"
#include "SceneNode.h"

ActorObject::ActorObject(const std::string &ActorName, SceneNode &SceneNode)
    : Object(ActorName, SceneNode), ActorCompMap({}) {}

ActorObject::~ActorObject() {}

void
ActorObject::addAComponent(COMP_TYPE CompType) {
  std::string CompName = getObjectName();

  switch (CompType) {
  case COMP_TYPE::A_TRANSFORM:
    CompName += "-transform";
    break;
  case COMP_TYPE::A_INPUT:
    CompName += "-input";
    break;
  case COMP_TYPE::A_INTERACT:
    CompName += "-interact";
    break;
  case COMP_TYPE::A_TIMER:
    CompName += "-timer";
    break;
  case COMP_TYPE::A_COLLISION:
    CompName += "-collision";
    break;
  case COMP_TYPE::A_MESH:
    CompName += "-mesh";
    break;
  case COMP_TYPE::A_LIGHT:
    CompName += "-light";
    break;
  case COMP_TYPE::A_AUDIO:
    CompName += "-audio";
    break;
  case COMP_TYPE::A_PARTICLE:
    CompName += "-particle";
    break;
  case COMP_TYPE::A_ANIMATE:
    CompName += "-animate";
    break;
  case COMP_TYPE::A_SPRITE:
    CompName += "-sprite";
    break;
  default:
    break;
  }

  ActorCompMap.insert({CompType, CompName});
}

bool
ActorObject::init() {
  auto CompContainer = getSceneNode().GetComponentContainer();

  for (auto &CompInfo : ActorCompMap) {
    auto Comp = CompContainer->getComponent(CompInfo.second);
#ifdef _DEBUG
    assert(Comp);
#endif // _DEBUG
    static_cast<ActorComponent *>(Comp)->resetActorOwner(this);
    if (!Comp->init()) {
      return false;
    }
    Comp->setCompStatus(STATUS::ACTIVE);
  }

  setObjectStatus(STATUS::ACTIVE);

  return true;
}

void
ActorObject::destory() {
  auto CompContainer = getSceneNode().GetComponentContainer();

  for (auto &CompInfo : ActorCompMap) {
    auto Comp = CompContainer->getComponent(CompInfo.second);
#ifdef _DEBUG
    assert(Comp);
#endif // _DEBUG
    static_cast<ActorComponent *>(Comp)->resetActorOwner(this);
    Comp->destory();
    Comp->setCompStatus(STATUS::NEED_DESTORY);
    CompContainer->deleteComponent(CompInfo.first, CompInfo.second);
  }

  ActorCompMap.clear();
}

void
ActorObject::syncStatusToAllComps() {
  auto CompContainer = getSceneNode().GetComponentContainer();

  for (auto &CompInfo : ActorCompMap) {
    auto Comp = CompContainer->getComponent(CompInfo.second);
#ifdef _DEBUG
    assert(Comp);
#endif // _DEBUG
    Comp->setCompStatus(getObjectStatus());
  }
}
