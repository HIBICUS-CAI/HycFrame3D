#include "UInputComponent.h"

#include "ActorObject.h"
#include "UiObject.h"

UInputComponent::UInputComponent(const std::string &CompName, UiObject *UiOwner)
    : UiComponent(CompName, UiOwner), InputPrecessFunctionPtr(nullptr) {}

UInputComponent::~UInputComponent() {}

bool UInputComponent::init() {
  if (!InputPrecessFunctionPtr) {
    P_LOG(LOG_ERROR, "there's still no input func in : %s\n",
          getCompName().c_str());
  }

  return true;
}

void UInputComponent::update(Timer &Timer) {
  if (InputPrecessFunctionPtr) {
    InputPrecessFunctionPtr(this, Timer);
  }
}

void UInputComponent::destory() {}

void UInputComponent::setInputFunction(UiInputProcessFuncType FuncPtr) {
#ifdef _DEBUG
  assert(FuncPtr);
#endif // _DEBUG
  InputPrecessFunctionPtr = FuncPtr;
}

void UInputComponent::clearInputFunction() {
  InputPrecessFunctionPtr = nullptr;
}

ActorObject *UInputComponent::getActorObject(const std::string &ActorName) {
  return getSceneNode().getActorObject(ActorName);
}

UiObject *UInputComponent::getUiObject(const std::string &UiName) {
  return getSceneNode().getUiObject(UiName);
}
