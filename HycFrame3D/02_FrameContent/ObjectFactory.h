#pragma once

#include "Hyc3DCommon.h"

#include "AInputComponent.h"
#include "AInteractComponent.h"
#include "UInputComponent.h"
#include "UInteractComponent.h"

#include <TextUtility.h>

#include <string>
#include <unordered_map>

class ObjectFactory {
private:
  class SceneManager *SceneManagerPtr;

  std::unordered_map<std::string, ActorInputProcessFuncType>
      ActorInputFuncPtrMap;
  std::unordered_map<std::string, ActorInteractInitFuncType>
      ActorInteractInitFuncPtrMap;
  std::unordered_map<std::string, ActorInteractUpdateFuncType>
      ActorInteractUpdateFuncPtrMap;
  std::unordered_map<std::string, ActorInteractDestoryFuncType>
      ActorInteractDestoryFuncPtrMap;

  std::unordered_map<std::string, UiInputProcessFuncType> UiInputFuncPtrMap;
  std::unordered_map<std::string, UiInteractInitFuncType>
      UiInteractInitFuncPtrMap;
  std::unordered_map<std::string, UiInteractUpdateFuncType>
      UiInteractUpdateFuncPtrMap;
  std::unordered_map<std::string, UiInteractDestoryFuncType>
      UiInteractDestoryFuncPtrMap;

public:
  ObjectFactory();
  ~ObjectFactory();

  bool
  startUp(class SceneManager *SceneManager);
  void
  cleanAndStop();

  class SceneNode *
  createSceneNode(const std::string &Name, const std::string &File);

  std::unordered_map<std::string, ActorInputProcessFuncType> &
  getAInputMapPtr() {
    return ActorInputFuncPtrMap;
  }
  std::unordered_map<std::string, ActorInteractInitFuncType> &
  getAInitMapPtr() {
    return ActorInteractInitFuncPtrMap;
  }
  std::unordered_map<std::string, ActorInteractUpdateFuncType> &
  getAUpdateMapPtr() {
    return ActorInteractUpdateFuncPtrMap;
  }
  std::unordered_map<std::string, ActorInteractDestoryFuncType> &
  getADestoryMapPtr() {
    return ActorInteractDestoryFuncPtrMap;
  }
  std::unordered_map<std::string, UiInputProcessFuncType> &
  getUInputMapPtr() {
    return UiInputFuncPtrMap;
  }
  std::unordered_map<std::string, UiInteractInitFuncType> &
  getUInitMapPtr() {
    return UiInteractInitFuncPtrMap;
  }
  std::unordered_map<std::string, UiInteractUpdateFuncType> &
  getUUpdateMapPtr() {
    return UiInteractUpdateFuncPtrMap;
  }
  std::unordered_map<std::string, UiInteractDestoryFuncType> &
  getUDestoryMapPtr() {
    return UiInteractDestoryFuncPtrMap;
  }

private:
  void
  createSceneAssets(class SceneNode *Scene, hyc::text::JsonFile &Json);

  void
  createActorObject(class SceneNode *Scene,
                    hyc::text::JsonFile &Json,
                    const std::string &JsonPath);
  void
  createUiObject(class SceneNode *Scene,
                 hyc::text::JsonFile &Json,
                 const std::string &JsonPath);

  void
  createActorComp(class SceneNode *Scene,
                  class ActorObject *Actor,
                  hyc::text::JsonFile &Json,
                  const std::string &JsonPath);
  void
  createUiComp(class SceneNode *Scene,
               class UiObject *Ui,
               hyc::text::JsonFile &Json,
               const std::string &JsonPath);
};
