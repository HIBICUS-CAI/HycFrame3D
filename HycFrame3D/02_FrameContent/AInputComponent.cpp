#include "AInputComponent.h"

#include "ActorObject.h"
#include "UiObject.h"

AInputComponent::AInputComponent(const std::string &CompName,
                                 ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), InputPrecessFunctionPtr(nullptr) {}

AInputComponent::~AInputComponent() {}

bool
AInputComponent::init() {
  if (!InputPrecessFunctionPtr) {
    P_LOG(LOG_ERROR, "there's still no input func in : %s\n",
          getCompName().c_str());
  }

  return true;
}

void
AInputComponent::update(Timer &Timer) {
  if (InputPrecessFunctionPtr) {
    InputPrecessFunctionPtr(this, Timer);
  }
}

void
AInputComponent::destory() {}

void
AInputComponent::setInputFunction(ActorInputProcessFuncType FuncPtr) {
#ifdef _DEBUG
  assert(FuncPtr);
#endif // _DEBUG
  InputPrecessFunctionPtr = FuncPtr;
}

void
AInputComponent::clearInputFunction() {
  InputPrecessFunctionPtr = nullptr;
}

ActorObject *
AInputComponent::getActorObject(const std::string &ActorName) {
  return getSceneNode().getActorObject(ActorName);
}

UiObject *
AInputComponent::getUiObject(const std::string &UiName) {
  return getSceneNode().getUiObject(UiName);
}
