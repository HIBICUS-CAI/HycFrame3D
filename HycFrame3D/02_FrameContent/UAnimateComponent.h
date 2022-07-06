#pragma once

#include "UiComponent.h"

#include <directxmath.h>
#include <unordered_map>

struct ANIMATE_INFO {
  std::string TexName = "";
  DirectX::XMFLOAT2 Stride = {0.f, 0.f};
  unsigned int MaxCut = 0;
  bool RepeatFlg = false;
  float SwitchTime = 0.f;
};

class UAnimateComponent : public UiComponent {
private:
  std::unordered_map<std::string, ANIMATE_INFO *> AnimateMap;
  UINT CurrentAnimateCut;
  ANIMATE_INFO *CurrentAnimate;
  bool AnimateChangedFlg;
  float TimeCounter;

public:
  UAnimateComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~UAnimateComponent();

  UAnimateComponent &
  operator=(const UAnimateComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    AnimateMap = Source.AnimateMap;
    CurrentAnimateCut = Source.CurrentAnimateCut;
    CurrentAnimate = Source.CurrentAnimate;
    AnimateChangedFlg = Source.AnimateChangedFlg;
    TimeCounter = Source.TimeCounter;
    UiComponent::operator=(Source);
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
  bool
  loadAnimate(const std::string &AniName,
              const std::string &AniPath,
              const DirectX::XMFLOAT2 &Stride,
              UINT MaxCount,
              bool RepeatFlg,
              float SwitchTime);
  void
  deleteAnimate(const std::string &AniName);

  void
  resetCurrentAnimate();
  void
  clearCurrentAnimate();

  void
  changeAnimateTo(const std::string &AniName);

private:
  void
  syncAniInfoToSprite();
};
