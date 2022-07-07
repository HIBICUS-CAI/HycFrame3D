#include "ComponentContainer.h"

#include "AAnimateComponent.h"
#include "AAudioComponent.h"
#include "ACollisionComponent.h"
#include "AInputComponent.h"
#include "AInteractComponent.h"
#include "ALightComponent.h"
#include "AMeshComponent.h"
#include "AParticleComponent.h"
#include "ASpriteComponent.h"
#include "ATimerComponent.h"
#include "ATransformComponent.h"
#include "SceneNode.h"
#include "UAnimateComponent.h"
#include "UAudioComponent.h"
#include "UButtonComponent.h"
#include "UInputComponent.h"
#include "UInteractComponent.h"
#include "USpriteComponent.h"
#include "UTimerComponent.h"
#include "UTransformComponent.h"

ComponentContainer::ComponentContainer(SceneNode &SceneNode)
    : SceneNodeOwner(SceneNode), ATransformCompVector({}), AInputCompVector({}),
      AInteractCompVector({}), ATimerCompVector({}), ACollisionCompVector({}),
      AMeshCompVector({}), ALightCompVector({}), AAudioCompVector({}),
      AParticleCompVector({}), AAnimateCompVector({}), ASpriteCompVector({}),
      UTransformCompVector({}), USpriteCompVector({}), UAnimateCompVector({}),
      UTimerCompVector({}), UInputCompVector({}), UInteractCompVector({}),
      UButtonCompVector({}), UAudioCompVector({}), InsertFlag({}), CompMap({}) {
  ATransformCompVector.reserve(1024);
  AInputCompVector.reserve(1024);
  AInteractCompVector.reserve(1024);
  ATimerCompVector.reserve(1024);
  ACollisionCompVector.reserve(1024);
  AMeshCompVector.reserve(1024);
  ALightCompVector.reserve(1024);
  AAudioCompVector.reserve(1024);
  AParticleCompVector.reserve(1024);
  AAnimateCompVector.reserve(1024);
  ASpriteCompVector.reserve(1024);
  UTransformCompVector.reserve(1024);
  USpriteCompVector.reserve(1024);
  UAnimateCompVector.reserve(1024);
  UTimerCompVector.reserve(1024);
  UInputCompVector.reserve(1024);
  UInteractCompVector.reserve(1024);
  UButtonCompVector.reserve(1024);
  UAudioCompVector.reserve(1024);
}

ComponentContainer::~ComponentContainer() {}

Component *ComponentContainer::getComponent(const std::string &CompName) {
  if (CompMap.find(CompName) != CompMap.end()) {
    return CompMap[CompName];
  } else {
    P_LOG(LOG_WARNING, "cannot find component name : %s\n", CompName.c_str());
    return nullptr;
  }
}

void ComponentContainer::addComponent(COMP_TYPE Type, const Component &Comp) {
  if (InsertFlag[static_cast<size_t>(Type)].empty()) {
    switch (Type) {
    case COMP_TYPE::A_TRANSFORM:
      ATransformCompVector.emplace_back(
          static_cast<const ATransformComponent &>(Comp));
      CompMap.insert({ATransformCompVector.back().getCompName(),
                      &(ATransformCompVector.back())});
      break;
    case COMP_TYPE::A_INPUT:
      AInputCompVector.emplace_back(static_cast<const AInputComponent &>(Comp));
      CompMap.insert(
          {AInputCompVector.back().getCompName(), &(AInputCompVector.back())});
      break;
    case COMP_TYPE::A_INTERACT:
      AInteractCompVector.emplace_back(
          static_cast<const AInteractComponent &>(Comp));
      CompMap.insert({AInteractCompVector.back().getCompName(),
                      &(AInteractCompVector.back())});
      break;
    case COMP_TYPE::A_TIMER:
      ATimerCompVector.emplace_back(static_cast<const ATimerComponent &>(Comp));
      CompMap.insert(
          {ATimerCompVector.back().getCompName(), &(ATimerCompVector.back())});
      break;
    case COMP_TYPE::A_COLLISION:
      ACollisionCompVector.emplace_back(
          static_cast<const ACollisionComponent &>(Comp));
      CompMap.insert({ACollisionCompVector.back().getCompName(),
                      &(ACollisionCompVector.back())});
      break;
    case COMP_TYPE::A_MESH:
      AMeshCompVector.emplace_back(static_cast<const AMeshComponent &>(Comp));
      CompMap.insert(
          {AMeshCompVector.back().getCompName(), &(AMeshCompVector.back())});
      break;
    case COMP_TYPE::A_LIGHT:
      ALightCompVector.emplace_back(static_cast<const ALightComponent &>(Comp));
      CompMap.insert(
          {ALightCompVector.back().getCompName(), &(ALightCompVector.back())});
      break;
    case COMP_TYPE::A_AUDIO:
      AAudioCompVector.emplace_back(static_cast<const AAudioComponent &>(Comp));
      CompMap.insert(
          {AAudioCompVector.back().getCompName(), &(AAudioCompVector.back())});
      break;
    case COMP_TYPE::A_PARTICLE:
      AParticleCompVector.emplace_back(
          static_cast<const AParticleComponent &>(Comp));
      CompMap.insert({AParticleCompVector.back().getCompName(),
                      &(AParticleCompVector.back())});
      break;
    case COMP_TYPE::A_ANIMATE:
      AAnimateCompVector.emplace_back(
          static_cast<const AAnimateComponent &>(Comp));
      CompMap.insert({AAnimateCompVector.back().getCompName(),
                      &(AAnimateCompVector.back())});
      break;
    case COMP_TYPE::A_SPRITE:
      ASpriteCompVector.emplace_back(
          static_cast<const ASpriteComponent &>(Comp));
      CompMap.insert({ASpriteCompVector.back().getCompName(),
                      &(ASpriteCompVector.back())});
      break;
    case COMP_TYPE::U_TRANSFORM:
      UTransformCompVector.emplace_back(
          static_cast<const UTransformComponent &>(Comp));
      CompMap.insert({UTransformCompVector.back().getCompName(),
                      &(UTransformCompVector.back())});
      break;
    case COMP_TYPE::U_SPRITE:
      USpriteCompVector.emplace_back(
          static_cast<const USpriteComponent &>(Comp));
      CompMap.insert({USpriteCompVector.back().getCompName(),
                      &(USpriteCompVector.back())});
      break;
    case COMP_TYPE::U_ANIMATE:
      UAnimateCompVector.emplace_back(
          static_cast<const UAnimateComponent &>(Comp));
      CompMap.insert({UAnimateCompVector.back().getCompName(),
                      &(UAnimateCompVector.back())});
      break;
    case COMP_TYPE::U_TIMER:
      UTimerCompVector.emplace_back(static_cast<const UTimerComponent &>(Comp));
      CompMap.insert(
          {UTimerCompVector.back().getCompName(), &(UTimerCompVector.back())});
      break;
    case COMP_TYPE::U_INPUT:
      UInputCompVector.emplace_back(static_cast<const UInputComponent &>(Comp));
      CompMap.insert(
          {UInputCompVector.back().getCompName(), &(UInputCompVector.back())});
      break;
    case COMP_TYPE::U_INTERACT:
      UInteractCompVector.emplace_back(
          static_cast<const UInteractComponent &>(Comp));
      CompMap.insert({UInteractCompVector.back().getCompName(),
                      &(UInteractCompVector.back())});
      break;
    case COMP_TYPE::U_BUTTON:
      UButtonCompVector.emplace_back(
          static_cast<const UButtonComponent &>(Comp));
      CompMap.insert({UButtonCompVector.back().getCompName(),
                      &(UButtonCompVector.back())});
      break;
    case COMP_TYPE::U_AUDIO:
      UAudioCompVector.emplace_back(static_cast<const UAudioComponent &>(Comp));
      CompMap.insert(
          {UAudioCompVector.back().getCompName(), &(UAudioCompVector.back())});
      break;
    default:
      break;
    }
  } else {
    UINT Index = InsertFlag[static_cast<size_t>(Type)].front();
    InsertFlag[static_cast<size_t>(Type)].pop();

    switch (Type) {
    case COMP_TYPE::A_TRANSFORM:
      ATransformCompVector[Index] =
          static_cast<const ATransformComponent &>(Comp);
      CompMap.insert({ATransformCompVector[Index].getCompName(),
                      &(ATransformCompVector[Index])});
      break;
    case COMP_TYPE::A_INPUT:
      AInputCompVector[Index] = static_cast<const AInputComponent &>(Comp);
      CompMap.insert(
          {AInputCompVector[Index].getCompName(), &(AInputCompVector[Index])});
      break;
    case COMP_TYPE::A_INTERACT:
      AInteractCompVector[Index] =
          static_cast<const AInteractComponent &>(Comp);
      CompMap.insert({AInteractCompVector[Index].getCompName(),
                      &(AInteractCompVector[Index])});
      break;
    case COMP_TYPE::A_TIMER:
      ATimerCompVector[Index] = static_cast<const ATimerComponent &>(Comp);
      CompMap.insert(
          {ATimerCompVector[Index].getCompName(), &(ATimerCompVector[Index])});
      break;
    case COMP_TYPE::A_COLLISION:
      ACollisionCompVector[Index] =
          static_cast<const ACollisionComponent &>(Comp);
      CompMap.insert({ACollisionCompVector[Index].getCompName(),
                      &(ACollisionCompVector[Index])});
      break;
    case COMP_TYPE::A_MESH:
      AMeshCompVector[Index] = static_cast<const AMeshComponent &>(Comp);
      CompMap.insert(
          {AMeshCompVector[Index].getCompName(), &(AMeshCompVector[Index])});
      break;
    case COMP_TYPE::A_LIGHT:
      ALightCompVector[Index] = static_cast<const ALightComponent &>(Comp);
      CompMap.insert(
          {ALightCompVector[Index].getCompName(), &(ALightCompVector[Index])});
      break;
    case COMP_TYPE::A_AUDIO:
      AAudioCompVector[Index] = static_cast<const AAudioComponent &>(Comp);
      CompMap.insert(
          {AAudioCompVector[Index].getCompName(), &(AAudioCompVector[Index])});
      break;
    case COMP_TYPE::A_PARTICLE:
      AParticleCompVector[Index] =
          static_cast<const AParticleComponent &>(Comp);
      CompMap.insert({AParticleCompVector[Index].getCompName(),
                      &(AParticleCompVector[Index])});
      break;
    case COMP_TYPE::A_ANIMATE:
      AAnimateCompVector[Index] = static_cast<const AAnimateComponent &>(Comp);
      CompMap.insert({AAnimateCompVector[Index].getCompName(),
                      &(AAnimateCompVector[Index])});
      break;
    case COMP_TYPE::A_SPRITE:
      ASpriteCompVector[Index] = static_cast<const ASpriteComponent &>(Comp);
      CompMap.insert({ASpriteCompVector[Index].getCompName(),
                      &(ASpriteCompVector[Index])});
      break;
    case COMP_TYPE::U_TRANSFORM:
      UTransformCompVector[Index] =
          static_cast<const UTransformComponent &>(Comp);
      CompMap.insert({UTransformCompVector[Index].getCompName(),
                      &(UTransformCompVector[Index])});
      break;
    case COMP_TYPE::U_SPRITE:
      USpriteCompVector[Index] = static_cast<const USpriteComponent &>(Comp);
      CompMap.insert({USpriteCompVector[Index].getCompName(),
                      &(USpriteCompVector[Index])});
      break;
    case COMP_TYPE::U_ANIMATE:
      UAnimateCompVector[Index] = static_cast<const UAnimateComponent &>(Comp);
      CompMap.insert({UAnimateCompVector[Index].getCompName(),
                      &(UAnimateCompVector[Index])});
      break;
    case COMP_TYPE::U_TIMER:
      UTimerCompVector[Index] = static_cast<const UTimerComponent &>(Comp);
      CompMap.insert(
          {UTimerCompVector[Index].getCompName(), &(UTimerCompVector[Index])});
      break;
    case COMP_TYPE::U_INPUT:
      UInputCompVector[Index] = static_cast<const UInputComponent &>(Comp);
      CompMap.insert(
          {UInputCompVector[Index].getCompName(), &(UInputCompVector[Index])});
      break;
    case COMP_TYPE::U_INTERACT:
      UInteractCompVector[Index] =
          static_cast<const UInteractComponent &>(Comp);
      CompMap.insert({UInteractCompVector[Index].getCompName(),
                      &(UInteractCompVector[Index])});
      break;
    case COMP_TYPE::U_BUTTON:
      UButtonCompVector[Index] = static_cast<const UButtonComponent &>(Comp);
      CompMap.insert({UButtonCompVector[Index].getCompName(),
                      &(UButtonCompVector[Index])});
      break;
    case COMP_TYPE::U_AUDIO:
      UAudioCompVector[Index] = static_cast<const UAudioComponent &>(Comp);
      CompMap.insert(
          {UAudioCompVector[Index].getCompName(), &(UAudioCompVector[Index])});
      break;
    default:
      break;
    }
  }
}

void ComponentContainer::deleteComponent(COMP_TYPE Type,
                                         const std::string &CompName) {
  auto Found = CompMap.find(CompName);
  if (Found == CompMap.end()) {
    P_LOG(LOG_WARNING, "cannot find component name : %s\n", CompName.c_str());
    return;
  }

  Component *Dele = Found->second;
  UINT Offset = 0;

  switch (Type) {
  case COMP_TYPE::A_TRANSFORM: {
    std::vector<ATransformComponent>::size_type VOffset =
        static_cast<ATransformComponent *>(Dele) - &ATransformCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_INPUT: {
    std::vector<AInputComponent>::size_type VOffset =
        static_cast<AInputComponent *>(Dele) - &AInputCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_INTERACT: {
    std::vector<AInteractComponent>::size_type VOffset =
        static_cast<AInteractComponent *>(Dele) - &AInteractCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_TIMER: {
    std::vector<ATimerComponent>::size_type VOffset =
        static_cast<ATimerComponent *>(Dele) - &ATimerCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_COLLISION: {
    std::vector<ACollisionComponent>::size_type VOffset =
        static_cast<ACollisionComponent *>(Dele) - &ACollisionCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_MESH: {
    std::vector<AMeshComponent>::size_type VOffset =
        static_cast<AMeshComponent *>(Dele) - &AMeshCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_LIGHT: {
    std::vector<ALightComponent>::size_type VOffset =
        static_cast<ALightComponent *>(Dele) - &ALightCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_AUDIO: {
    std::vector<AAudioComponent>::size_type VOffset =
        static_cast<AAudioComponent *>(Dele) - &AAudioCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_PARTICLE: {
    std::vector<AParticleComponent>::size_type VOffset =
        static_cast<AParticleComponent *>(Dele) - &AParticleCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_ANIMATE: {
    std::vector<AAnimateComponent>::size_type VOffset =
        static_cast<AAnimateComponent *>(Dele) - &AAnimateCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::A_SPRITE: {
    std::vector<ASpriteComponent>::size_type VOffset =
        static_cast<ASpriteComponent *>(Dele) - &ASpriteCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::U_TRANSFORM: {
    std::vector<UTransformComponent>::size_type VOffset =
        static_cast<UTransformComponent *>(Dele) - &UTransformCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::U_SPRITE: {
    std::vector<USpriteComponent>::size_type VOffset =
        static_cast<USpriteComponent *>(Dele) - &USpriteCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::U_ANIMATE: {
    std::vector<UAnimateComponent>::size_type VOffset =
        static_cast<UAnimateComponent *>(Dele) - &UAnimateCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::U_TIMER: {
    std::vector<UTimerComponent>::size_type VOffset =
        static_cast<UTimerComponent *>(Dele) - &UTimerCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::U_INPUT: {
    std::vector<UInputComponent>::size_type VOffset =
        static_cast<UInputComponent *>(Dele) - &UInputCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::U_INTERACT: {
    std::vector<UInteractComponent>::size_type VOffset =
        static_cast<UInteractComponent *>(Dele) - &UInteractCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::U_BUTTON: {
    std::vector<UButtonComponent>::size_type VOffset =
        static_cast<UButtonComponent *>(Dele) - &UButtonCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  case COMP_TYPE::U_AUDIO: {
    std::vector<UAudioComponent>::size_type VOffset =
        static_cast<UAudioComponent *>(Dele) - &UAudioCompVector[0];
    Offset = static_cast<UINT>(VOffset);
    break;
  }
  default: {
    assert(false && "invalid comp to delete");
    break;
  }
  }

  CompMap.erase(Found);
  InsertFlag[static_cast<size_t>(Type)].push(Offset);
}

void ComponentContainer::deleteAllComponent() {
  ATransformCompVector.clear();
  AInputCompVector.clear();
  AInteractCompVector.clear();
  ATimerCompVector.clear();
  ACollisionCompVector.clear();
  AMeshCompVector.clear();
  ALightCompVector.clear();
  AAudioCompVector.clear();
  AParticleCompVector.clear();
  AAnimateCompVector.clear();
  ASpriteCompVector.clear();
  UTransformCompVector.clear();
  USpriteCompVector.clear();
  UAnimateCompVector.clear();
  UTimerCompVector.clear();
  UInputCompVector.clear();
  UInteractCompVector.clear();
  UButtonCompVector.clear();
  UAudioCompVector.clear();
  CompMap.clear();
  for (auto &FlgArray : InsertFlag) {
    while (!FlgArray.empty()) {
      FlgArray.pop();
    }
  }
}

void *ComponentContainer::getCompVecPtr(COMP_TYPE Type) {
  switch (Type) {
  case COMP_TYPE::A_TRANSFORM:
    return static_cast<void *>(&ATransformCompVector);
  case COMP_TYPE::A_INPUT:
    return static_cast<void *>(&AInputCompVector);
  case COMP_TYPE::A_INTERACT:
    return static_cast<void *>(&AInteractCompVector);
  case COMP_TYPE::A_TIMER:
    return static_cast<void *>(&ATimerCompVector);
  case COMP_TYPE::A_COLLISION:
    return static_cast<void *>(&ACollisionCompVector);
  case COMP_TYPE::A_MESH:
    return static_cast<void *>(&AMeshCompVector);
  case COMP_TYPE::A_LIGHT:
    return static_cast<void *>(&ALightCompVector);
  case COMP_TYPE::A_AUDIO:
    return static_cast<void *>(&AAudioCompVector);
  case COMP_TYPE::A_PARTICLE:
    return static_cast<void *>(&AParticleCompVector);
  case COMP_TYPE::A_ANIMATE:
    return static_cast<void *>(&AAnimateCompVector);
  case COMP_TYPE::A_SPRITE:
    return static_cast<void *>(&ASpriteCompVector);
  case COMP_TYPE::U_TRANSFORM:
    return static_cast<void *>(&UTransformCompVector);
  case COMP_TYPE::U_SPRITE:
    return static_cast<void *>(&USpriteCompVector);
  case COMP_TYPE::U_ANIMATE:
    return static_cast<void *>(&UAnimateCompVector);
  case COMP_TYPE::U_TIMER:
    return static_cast<void *>(&UTimerCompVector);
  case COMP_TYPE::U_INPUT:
    return static_cast<void *>(&UInputCompVector);
  case COMP_TYPE::U_INTERACT:
    return static_cast<void *>(&UInteractCompVector);
  case COMP_TYPE::U_BUTTON:
    return static_cast<void *>(&UButtonCompVector);
  case COMP_TYPE::U_AUDIO:
    return static_cast<void *>(&UAudioCompVector);
  default:
    return nullptr;
  }
}
