#include "UInteractComponent.h"

#include "ActorObject.h"
#include "UiObject.h"

UInteractComponent::UInteractComponent(const std::string &CompName,
                                       UiObject *UiOwner)
    : UiComponent(CompName, UiOwner), InitProcessFunctionPtr(nullptr),
      UpdateProcessFunctionPtr(nullptr), DestoryProcessFunctionPtr(nullptr) {}

UInteractComponent::~UInteractComponent() {}

bool
UInteractComponent::init() {
  if (InitProcessFunctionPtr) {
    return InitProcessFunctionPtr(this);
  } else {
    return false;
  }
}

void
UInteractComponent::update(Timer &timer) {
  if (UpdateProcessFunctionPtr) {
    return UpdateProcessFunctionPtr(this, timer);
  }
}

void
UInteractComponent::destory() {
  if (DestoryProcessFunctionPtr) {
    return DestoryProcessFunctionPtr(this);
  }
}

void
UInteractComponent::setInitFunction(UiInteractInitFuncType InitFuncPtr) {
#ifdef _DEBUG
  assert(InitFuncPtr);
#endif // _DEBUG
  InitProcessFunctionPtr = InitFuncPtr;
}

void
UInteractComponent::setUpdateFunction(UiInteractUpdateFuncType UpdateFuncPtr) {
#ifdef _DEBUG
  assert(UpdateFuncPtr);
#endif // _DEBUG
  UpdateProcessFunctionPtr = UpdateFuncPtr;
}

void
UInteractComponent::setDestoryFunction(
    UiInteractDestoryFuncType DestoryFuncPtr) {
#ifdef _DEBUG
  assert(DestoryFuncPtr);
#endif // _DEBUG
  DestoryProcessFunctionPtr = DestoryFuncPtr;
}

void
UInteractComponent::clearInitFunction() {
  InitProcessFunctionPtr = nullptr;
}

void
UInteractComponent::clearUpdateFunction() {
  UpdateProcessFunctionPtr = nullptr;
}

void
UInteractComponent::clearDestoryFunction() {
  DestoryProcessFunctionPtr = nullptr;
}

ActorObject *
UInteractComponent::getActorObject(const std::string &ActorName) {
  return getSceneNode().GetActorObject(ActorName);
}

UiObject *
UInteractComponent::getUiObject(const std::string &UiName) {
  return getSceneNode().GetUiObject(UiName);
}
