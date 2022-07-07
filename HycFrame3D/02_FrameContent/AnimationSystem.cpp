#include "AnimationSystem.h"

#include "AAnimateComponent.h"
#include "AssetsPool.h"
#include "ComponentContainer.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SystemExecutive.h"

AnimationSystem::AnimationSystem(SystemExecutive *SysExecutive)
    : System("animation-system", SysExecutive), AAnimateArrayPtr(nullptr) {}

AnimationSystem::~AnimationSystem() {}

bool AnimationSystem::init() {
#ifdef _DEBUG
  assert(getSystemExecutive());
#endif // _DEBUG

  AAnimateArrayPtr = static_cast<std::vector<AAnimateComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_ANIMATE));

  if (!AAnimateArrayPtr) {
    return false;
  }

  return true;
}

void AnimationSystem::run(const Timer &Timer) {
  for (auto &Aac : *AAnimateArrayPtr) {
    if (Aac.getCompStatus() == STATUS::ACTIVE) {
      Aac.update(Timer);
    }
  }
}

void AnimationSystem::destory() {}
