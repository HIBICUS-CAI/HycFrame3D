#include "AudioSystem.h"

#include "AAudioComponent.h"
#include "AssetsPool.h"
#include "ComponentContainer.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SoundHelper.h"
#include "SystemExecutive.h"
#include "UAudioComponent.h"

AudioSystem::AudioSystem(SystemExecutive *_sysExecutive)
    : System("audio-system", _sysExecutive), AAudioArrayPtr(nullptr),
      UAudioArrayPtr(nullptr) {}

AudioSystem::~AudioSystem() {}

bool
AudioSystem::init() {
  if (!soundHasInited()) {
    if (!initSound()) {
      return false;
    }
  }

#ifdef _DEBUG
  assert(getSystemExecutive());
#endif // _DEBUG

  AAudioArrayPtr = static_cast<std::vector<AAudioComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_AUDIO));
  UAudioArrayPtr = static_cast<std::vector<UAudioComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::U_AUDIO));

  if (!(AAudioArrayPtr && UAudioArrayPtr)) {
    return false;
  }

  return true;
}

void
AudioSystem::run(Timer &Timer) {
  for (auto &Aauc : *AAudioArrayPtr) {
    if (Aauc.getCompStatus() == STATUS::ACTIVE) {
      Aauc.update(Timer);
    }
  }

  for (auto &Uauc : *UAudioArrayPtr) {
    if (Uauc.getCompStatus() == STATUS::ACTIVE) {
      Uauc.update(Timer);
    }
  }
}

void
AudioSystem::destory() {
  uninitSound();
}
