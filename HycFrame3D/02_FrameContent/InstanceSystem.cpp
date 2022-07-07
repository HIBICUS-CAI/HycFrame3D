#include "InstanceSystem.h"

#include "ALightComponent.h"
#include "AMeshComponent.h"
#include "AParticleComponent.h"
#include "ASpriteComponent.h"
#include "ATransformComponent.h"
#include "ComponentContainer.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SystemExecutive.h"
#include "UAnimateComponent.h"
#include "USpriteComponent.h"
#include "UTransformComponent.h"

InstanceSystem::InstanceSystem(SystemExecutive *SysExecutive)
    : System("instance-system", SysExecutive), ATransArrayPtr(nullptr),
      UTransArrayPtr(nullptr), AMeshArrayPtr(nullptr), ALightArrayPtr(nullptr),
      AParitcleArrayPtr(nullptr), ASpriteArrayPtr(nullptr),
      USpriteArrayPtr(nullptr), UAnimateArrayPtr(nullptr) {}

InstanceSystem::~InstanceSystem() {}

bool
InstanceSystem::init() {
#ifdef _DEBUG
  assert(getSystemExecutive());
#endif // _DEBUG

  ATransArrayPtr = static_cast<std::vector<ATransformComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_TRANSFORM));
  UTransArrayPtr = static_cast<std::vector<UTransformComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::U_TRANSFORM));
  AMeshArrayPtr = static_cast<std::vector<AMeshComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_MESH));
  ALightArrayPtr = static_cast<std::vector<ALightComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_LIGHT));
  AParitcleArrayPtr = static_cast<std::vector<AParticleComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_PARTICLE));
  ASpriteArrayPtr = static_cast<std::vector<ASpriteComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_SPRITE));
  USpriteArrayPtr = static_cast<std::vector<USpriteComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::U_SPRITE));
  UAnimateArrayPtr = static_cast<std::vector<UAnimateComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::U_ANIMATE));

  if (!(ATransArrayPtr && UTransArrayPtr && AMeshArrayPtr && ASpriteArrayPtr &&
        ALightArrayPtr && AParitcleArrayPtr && USpriteArrayPtr &&
        UAnimateArrayPtr)) {
    return false;
  }

  return true;
}

void
InstanceSystem::run(Timer &Timer) {
  for (auto &Atc : *ATransArrayPtr) {
    if (Atc.getCompStatus() == STATUS::ACTIVE) {
      Atc.update(Timer);
    }
  }

  for (auto &Utc : *UTransArrayPtr) {
    if (Utc.getCompStatus() == STATUS::ACTIVE) {
      Utc.update(Timer);
    }
  }

  for (auto &Amc : *AMeshArrayPtr) {
    if (Amc.getCompStatus() == STATUS::ACTIVE) {
      Amc.update(Timer);
    }
  }

  for (auto &Alc : *ALightArrayPtr) {
    if (Alc.getCompStatus() == STATUS::ACTIVE) {
      Alc.update(Timer);
    }
  }

  for (auto &Apc : *AParitcleArrayPtr) {
    if (Apc.getCompStatus() == STATUS::ACTIVE) {
      Apc.update(Timer);
    }
  }

  for (auto &Asc : *ASpriteArrayPtr) {
    if (Asc.getCompStatus() == STATUS::ACTIVE) {
      Asc.update(Timer);
    }
  }

  for (auto &Usc : *USpriteArrayPtr) {
    if (Usc.getCompStatus() == STATUS::ACTIVE) {
      Usc.update(Timer);
    }
  }

  for (auto &Uac : *UAnimateArrayPtr) {
    if (Uac.getCompStatus() == STATUS::ACTIVE) {
      Uac.update(Timer);
    }
  }
}

void
InstanceSystem::destory() {}
