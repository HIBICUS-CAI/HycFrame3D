#pragma once

#include "ActorComponent.h"

#include "SoundHelper.h"

#include <unordered_map>

class AAudioComponent : public ActorComponent {
private:
  std::unordered_map<std::string, SOUND_HANDLE> AudioMap;

public:
  AAudioComponent(const std::string &CompName, class ActorObject *ActorOwner);
  virtual ~AAudioComponent();

  AAudioComponent &
  operator=(const AAudioComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    AudioMap = Source.AudioMap;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool
  init();
  virtual void
  update(Timer &Timer);
  virtual void
  destory();

public:
  void
  addAudio(const std::string &AudioName, class SceneNode &Scene);

  void
  playBgm(const std::string &BgmName, float Volume);
  void
  playSe(const std::string &SeName, float Volume);

  void
  stopBgm();
  void
  stopBgm(const std::string &BgmName);
};
