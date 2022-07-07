#pragma once

#include "Hyc3DCommon.h"

#include <array>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

class ComponentContainer {
private:
  const class SceneNode &SceneNodeOwner;

  std::vector<class ATransformComponent> ATransformCompVector;
  std::vector<class AInputComponent> AInputCompVector;
  std::vector<class AInteractComponent> AInteractCompVector;
  std::vector<class ATimerComponent> ATimerCompVector;
  std::vector<class ACollisionComponent> ACollisionCompVector;
  std::vector<class AMeshComponent> AMeshCompVector;
  std::vector<class ALightComponent> ALightCompVector;
  std::vector<class AAudioComponent> AAudioCompVector;
  std::vector<class AParticleComponent> AParticleCompVector;
  std::vector<class AAnimateComponent> AAnimateCompVector;
  std::vector<class ASpriteComponent> ASpriteCompVector;

  std::vector<class UTransformComponent> UTransformCompVector;
  std::vector<class USpriteComponent> USpriteCompVector;
  std::vector<class UAnimateComponent> UAnimateCompVector;
  std::vector<class UTimerComponent> UTimerCompVector;
  std::vector<class UInputComponent> UInputCompVector;
  std::vector<class UInteractComponent> UInteractCompVector;
  std::vector<class UButtonComponent> UButtonCompVector;
  std::vector<class UAudioComponent> UAudioCompVector;

  std::array<std::queue<UINT>, static_cast<size_t>(COMP_TYPE::SIZE)> InsertFlag;

  std::unordered_map<std::string, class Component *> CompMap;

public:
  ComponentContainer(class SceneNode &SceneNode);
  ~ComponentContainer();

  class Component *getComponent(const std::string &CompName);

  void addComponent(COMP_TYPE Type, const class Component &Comp);
  void deleteComponent(COMP_TYPE Type, const std::string &CompName);

  void deleteAllComponent();

  void *getCompVecPtr(COMP_TYPE Type);
};
