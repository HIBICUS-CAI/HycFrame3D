#pragma once

#include "ActorComponent.h"

using ActorInputProcessFuncType = void (*)(class AInputComponent *, Timer &);

class AInputComponent : public ActorComponent {
private:
  ActorInputProcessFuncType InputPrecessFunctionPtr;

public:
  AInputComponent(const std::string &CompName, class ActorObject *ActorOwner);
  virtual ~AInputComponent();

  AInputComponent &operator=(const AInputComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    InputPrecessFunctionPtr = Source.InputPrecessFunctionPtr;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(Timer &Timer);
  virtual void destory();

public:
  void setInputFunction(ActorInputProcessFuncType FuncPtr);
  void clearInputFunction();

  class ActorObject *getActorObject(const std::string &ActorName);
  class UiObject *getUiObject(const std::string &UiName);
};
