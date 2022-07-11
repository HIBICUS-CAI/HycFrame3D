#pragma once

#include "ActorComponent.h"

#include <RSCommon.h>

#include <vector>

class ASpriteComponent : public ActorComponent {
private:
  std::string GeoPointName;
  std::string TextureName;
  bool EnabledBillboardFlag;

  dx::XMFLOAT2 Size;
  dx::XMFLOAT4 TexCoord; // origin u & v, offset length & width

  bool EnabledAnimationFlag;
  dx::XMFLOAT2 Stride;
  UINT MaxCut;
  UINT CurrentAnimateCut;
  bool RepeatFlag;
  float SwitchTime;
  float TimeCounter;

public:
  ASpriteComponent(const std::string &CompName, class ActorObject *ActorOwner);
  virtual ~ASpriteComponent();

  ASpriteComponent &operator=(const ASpriteComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    GeoPointName = Source.GeoPointName;
    TextureName = Source.TextureName;
    EnabledBillboardFlag = Source.EnabledBillboardFlag;
    Size = Source.Size;
    TexCoord = Source.TexCoord;
    EnabledAnimationFlag = Source.EnabledAnimationFlag;
    Stride = Source.Stride;
    MaxCut = Source.MaxCut;
    CurrentAnimateCut = Source.CurrentAnimateCut;
    RepeatFlag = Source.RepeatFlag;
    SwitchTime = Source.SwitchTime;
    TimeCounter = Source.TimeCounter;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(const Timer &Timer);
  virtual void destory();

public:
  bool createGeoPointWithTexture(class SceneNode *Scene,
                                 const std::string &TexName);

  void setSpriteProperty(const dx::XMFLOAT2 &SpriteSize,
                         const dx::XMFLOAT4 &SpriteTexCoord,
                         bool IsBillboard);

  void setAnimationProperty(const dx::XMFLOAT2 &Stride,
                            UINT MaxCut,
                            bool RepeatFlag,
                            float WwitchTime);

private:
  void syncTransformDataToInstance();
};
