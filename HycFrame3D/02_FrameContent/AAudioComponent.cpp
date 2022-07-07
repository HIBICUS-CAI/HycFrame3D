#include "AAudioComponent.h"

#include "ActorObject.h"
#include "AssetsPool.h"
#include "SceneNode.h"

AAudioComponent::AAudioComponent(const std::string &CompName,
                                 ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), AudioMap({}) {}

AAudioComponent::~AAudioComponent() {}

bool AAudioComponent::init() {
  if (AudioMap.size()) {
    return true;
  } else {
    return false;
  }
}

void AAudioComponent::update(Timer &Timer) {}

void AAudioComponent::destory() { AudioMap.clear(); }

void AAudioComponent::addAudio(const std::string &AudioName, SceneNode &Scene) {
  SOUND_HANDLE Audio = Scene.getAssetsPool()->getSoundIfExisted(AudioName);
#ifdef _DEBUG
  assert(Audio);
#endif // _DEBUG
  AudioMap.insert({AudioName, Audio});
}

void AAudioComponent::playBgm(const std::string &BgmName, float Volume) {
  setVolume(BgmName, Volume);
  playBGM(BgmName);
}

void AAudioComponent::playSe(const std::string &SeName, float Volume) {
  setVolume(SeName, Volume);
  playSE(SeName);
}

void AAudioComponent::stopBgm() { stopBGM(); }

void AAudioComponent::stopBgm(const std::string &BgmName) { stopBGM(BgmName); }
