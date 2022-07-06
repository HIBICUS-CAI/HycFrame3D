#include "UAudioComponent.h"

#include "AssetsPool.h"
#include "SceneNode.h"
#include "UiObject.h"

UAudioComponent::UAudioComponent(const std::string &CompName, UiObject *UiOwner)
    : UiComponent(CompName, UiOwner), mAudioMap({}) {}

UAudioComponent::~UAudioComponent() {}

bool
UAudioComponent::init() {
  if (mAudioMap.size()) {
    return true;
  } else {
    return false;
  }
}

void
UAudioComponent::update(Timer &Timer) {}

void
UAudioComponent::destory() {
  mAudioMap.clear();
}

void
UAudioComponent::addAudio(const std::string &AudioName, SceneNode &Scene) {
  SOUND_HANDLE Audio = Scene.getAssetsPool()->getSoundIfExisted(AudioName);
#ifdef _DEBUG
  assert(Audio);
#endif // _DEBUG
  mAudioMap.insert({AudioName, Audio});
}

void
UAudioComponent::playBgm(const std::string &BgmName, float Volume) {
  setVolume(BgmName, Volume);
  playBGM(BgmName);
}

void
UAudioComponent::playSe(const std::string &SeName, float Volume) {
  setVolume(SeName, Volume);
  playSE(SeName);
}

void
UAudioComponent::stopBgm() {
  stopBGM();
}

void
UAudioComponent::stopBgm(const std::string &BgmName) {
  stopBGM(BgmName);
}
