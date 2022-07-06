#include "AInteractComponent.h"

#include "ActorObject.h"
#include "UiObject.h"

AInteractComponent::AInteractComponent(const std::string &CompName,
                                       ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), InitProcessFunctionPtr(nullptr),
      UpdateProcessFunctionPtr(nullptr), DestoryProcessFunctionPtr(nullptr) {}

AInteractComponent::~AInteractComponent() {}

bool
AInteractComponent::init() {
  if (InitProcessFunctionPtr) {
    return InitProcessFunctionPtr(this);
  } else {
    return false;
  }
}

void
AInteractComponent::update(Timer &Timer) {
  if (UpdateProcessFunctionPtr) {
    return UpdateProcessFunctionPtr(this, Timer);
  }
}

void
AInteractComponent::destory() {
  if (DestoryProcessFunctionPtr) {
    return DestoryProcessFunctionPtr(this);
  }
}

void
AInteractComponent::setInitFunction(ActorInteractInitFuncType InitFuncPtr) {
#ifdef _DEBUG
  assert(InitFuncPtr);
#endif // _DEBUG
  InitProcessFunctionPtr = InitFuncPtr;
}

void
AInteractComponent::setUpdateFunction(
    ActorInteractUpdateFuncType UpdateFuncPtr) {
#ifdef _DEBUG
  assert(UpdateFuncPtr);
#endif // _DEBUG
  UpdateProcessFunctionPtr = UpdateFuncPtr;
}

void
AInteractComponent::setDestoryFunction(
    ActorInteractDestoryFuncType DestoryFuncPtr) {
#ifdef _DEBUG
  assert(DestoryFuncPtr);
#endif // _DEBUG
  DestoryProcessFunctionPtr = DestoryFuncPtr;
}

void
AInteractComponent::clearInitFunction() {
  InitProcessFunctionPtr = nullptr;
}

void
AInteractComponent::clearUpdateFunction() {
  UpdateProcessFunctionPtr = nullptr;
}

void
AInteractComponent::clearDestoryFunction() {
  DestoryProcessFunctionPtr = nullptr;
}

ActorObject *
AInteractComponent::getActorObject(const std::string &ActorName) {
  return getSceneNode().getActorObject(ActorName);
}

UiObject *
AInteractComponent::getUiObject(const std::string &UiName) {
  return getSceneNode().getUiObject(UiName);
}
