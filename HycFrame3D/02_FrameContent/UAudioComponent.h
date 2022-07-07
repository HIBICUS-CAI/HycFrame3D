#pragma once

#include "UiComponent.h"

#include "SoundHelper.h"

#include <unordered_map>

class UAudioComponent : public UiComponent {
private:
  std::unordered_map<std::string, SOUND_HANDLE> mAudioMap;

public:
  UAudioComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~UAudioComponent();

  UAudioComponent &operator=(const UAudioComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    mAudioMap = Source.mAudioMap;
    UiComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(Timer &Timer);
  virtual void destory();

public:
  void addAudio(const std::string &AudioName, class SceneNode &Scene);

  void playBgm(const std::string &BgmName, float Volume);
  void playSe(const std::string &SeName, float Volume);

  void stopBgm();
  void stopBgm(const std::string &BgmName);
};
