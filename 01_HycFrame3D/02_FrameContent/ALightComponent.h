#pragma once

#include "ActorComponent.h"

#include <RSCommon.h>

#include <DirectXMath.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif // __clang__

constexpr auto BOX_BLOOM_MESH_NAME = "box-bloom-mesh";

#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

class ALightComponent : public ActorComponent {
private:
  std::string LightName;
  class RSLight *RSLightPtr;

  bool CanCreateLight;
  LIGHT_INFO LightInfoForInit;
  CAM_INFO LightCamInfoForInit;
  bool IsBloom;
  bool IsCamera;

public:
  ALightComponent(const std::string &CompName, class ActorObject *ActorOwner);
  virtual ~ALightComponent();

  ALightComponent &operator=(const ALightComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    CanCreateLight = Source.CanCreateLight;
    LightInfoForInit = Source.LightInfoForInit;
    LightCamInfoForInit = Source.LightCamInfoForInit;
    IsBloom = Source.IsBloom;
    IsCamera = Source.IsCamera;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(const Timer &Timer);
  virtual void destory();

public:
  void addLight(const LIGHT_INFO &LightInfo,
                bool SetBloom,
                bool SetCamera,
                const CAM_INFO &CamInfo);

  void resetLight(const LIGHT_INFO *LightInfo);

  class RSLight *getLightInfo();

private:
  void createLight();
  void syncDataFromTransform();
};
