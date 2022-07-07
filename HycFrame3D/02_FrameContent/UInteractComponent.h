#pragma once

#include "UiComponent.h"

using UiInteractInitFuncType = bool (*)(class UInteractComponent *);
using UiInteractUpdateFuncType = void (*)(class UInteractComponent *, Timer &);
using UiInteractDestoryFuncType = void (*)(class UInteractComponent *);

class UInteractComponent : public UiComponent {
private:
  UiInteractInitFuncType InitProcessFunctionPtr;
  UiInteractUpdateFuncType UpdateProcessFunctionPtr;
  UiInteractDestoryFuncType DestoryProcessFunctionPtr;

public:
  UInteractComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~UInteractComponent();

  UInteractComponent &operator=(const UInteractComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    InitProcessFunctionPtr = Source.InitProcessFunctionPtr;
    UpdateProcessFunctionPtr = Source.UpdateProcessFunctionPtr;
    DestoryProcessFunctionPtr = Source.DestoryProcessFunctionPtr;
    UiComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(Timer &Timer);
  virtual void destory();

public:
  void setInitFunction(UiInteractInitFuncType InitFuncPtr);
  void setUpdateFunction(UiInteractUpdateFuncType UpdateFuncPtr);
  void setDestoryFunction(UiInteractDestoryFuncType DestoryFuncPtr);
  void clearInitFunction();
  void clearUpdateFunction();
  void clearDestoryFunction();

  class ActorObject *getActorObject(const std::string &ActorName);
  class UiObject *getUiObject(const std::string &UiName);
};
