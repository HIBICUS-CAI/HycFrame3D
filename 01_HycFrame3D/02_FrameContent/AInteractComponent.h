#pragma once

#include "ActorComponent.h"

using ActorInteractInitFuncType = bool (*)(class AInteractComponent *);
using ActorInteractUpdateFuncType = void (*)(class AInteractComponent *,
                                             const Timer &);
using ActorInteractDestoryFuncType = void (*)(class AInteractComponent *);

class AInteractComponent : public ActorComponent {
private:
  ActorInteractInitFuncType InitProcessFunctionPtr;
  ActorInteractUpdateFuncType UpdateProcessFunctionPtr;
  ActorInteractDestoryFuncType DestoryProcessFunctionPtr;

public:
  AInteractComponent(const std::string &CompName,
                     class ActorObject *ActorOwner);
  virtual ~AInteractComponent();

  AInteractComponent &operator=(const AInteractComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    InitProcessFunctionPtr = Source.InitProcessFunctionPtr;
    UpdateProcessFunctionPtr = Source.UpdateProcessFunctionPtr;
    DestoryProcessFunctionPtr = Source.DestoryProcessFunctionPtr;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(const Timer &Timer);
  virtual void destory();

public:
  void setInitFunction(ActorInteractInitFuncType InitFuncPtr);
  void setUpdateFunction(ActorInteractUpdateFuncType UpdateFuncPtr);
  void setDestoryFunction(ActorInteractDestoryFuncType DestoryFuncPtr);
  void clearInitFunction();
  void clearUpdateFunction();
  void clearDestoryFunction();

  class ActorObject *getActorObject(const std::string &ActorName);
  class UiObject *getUiObject(const std::string &UiName);
};
