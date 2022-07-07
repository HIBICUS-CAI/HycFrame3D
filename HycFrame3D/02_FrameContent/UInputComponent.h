#pragma once

#include "UiComponent.h"

using UiInputProcessFuncType = void (*)(class UInputComponent *, Timer &);

class UInputComponent : public UiComponent {
private:
  UiInputProcessFuncType InputPrecessFunctionPtr;

public:
  UInputComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~UInputComponent();

  UInputComponent &operator=(const UInputComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    InputPrecessFunctionPtr = Source.InputPrecessFunctionPtr;
    UiComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(Timer &Timer);
  virtual void destory();

public:
  void setInputFunction(UiInputProcessFuncType FuncPtr);
  void clearInputFunction();

  class ActorObject *getActorObject(const std::string &ActorName);
  class UiObject *getUiObject(const std::string &UiName);
};
