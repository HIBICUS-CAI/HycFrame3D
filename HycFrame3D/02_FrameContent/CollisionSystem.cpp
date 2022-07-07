#include "CollisionSystem.h"

#include "ACollisionComponent.h"
#include "ComponentContainer.h"
#include "PhysicsWorld.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SystemExecutive.h"

CollisionSystem::CollisionSystem(class SystemExecutive *SysExecutive)
    : System("collision-system", SysExecutive), ACollisionArrayPtr(nullptr) {}

CollisionSystem::~CollisionSystem() {}

bool
CollisionSystem::init() {
#ifdef _DEBUG
  assert(getSystemExecutive());
#endif // _DEBUG

  ACollisionArrayPtr = static_cast<std::vector<ACollisionComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_COLLISION));

  if (!(ACollisionArrayPtr)) {
    return false;
  }

  return true;
}

void
CollisionSystem::run(Timer &Timer) {
  for (auto &Acc : *ACollisionArrayPtr) {
    if (Acc.getCompStatus() == STATUS::ACTIVE) {
      Acc.update(Timer);
    }
  }

  getSystemExecutive()
      ->getSceneManager()
      ->getCurrentSceneNode()
      ->getPhysicsWorld()
      ->detectCollision();
}

void
CollisionSystem::destory() {}
