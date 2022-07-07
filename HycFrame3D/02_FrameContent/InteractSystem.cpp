#include "InteractSystem.h"

#include "AInteractComponent.h"
#include "ComponentContainer.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SystemExecutive.h"
#include "UInteractComponent.h"

InteractSystem::InteractSystem(SystemExecutive *SysExecutive)
    : System("interact-system", SysExecutive), AInterArrayPtr(nullptr),
      UInterArrayPtr(nullptr) {}

InteractSystem::~InteractSystem() {}

bool InteractSystem::init() {
#ifdef _DEBUG
  assert(getSystemExecutive());
#endif // _DEBUG

  AInterArrayPtr = static_cast<std::vector<AInteractComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_INTERACT));
  UInterArrayPtr = static_cast<std::vector<UInteractComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::U_INTERACT));

  if (!(AInterArrayPtr && UInterArrayPtr)) {
    return false;
  }

  return true;
}

void InteractSystem::run(Timer &Timer) {
  for (auto &Aitc : *AInterArrayPtr) {
    if (Aitc.getCompStatus() == STATUS::ACTIVE) {
      Aitc.update(Timer);
    }
  }

  for (auto &Uitc : *UInterArrayPtr) {
    if (Uitc.getCompStatus() == STATUS::ACTIVE) {
      Uitc.update(Timer);
    }
  }
}

void InteractSystem::destory() {}
