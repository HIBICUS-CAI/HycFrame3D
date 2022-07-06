#pragma once

#include "ActorComponent.h"

#include <directxmath.h>
#include <vector>

class AMeshComponent : public ActorComponent {
private:
  std::vector<std::string> MeshesNameArray;
  std::vector<std::string> SubMeshesNameArray;
  std::vector<DirectX::XMFLOAT3> OffsetPositionArray;
  float EmissiveIntensity;

public:
  AMeshComponent(const std::string &CompName, class ActorObject *ActorOwner);
  virtual ~AMeshComponent();

  AMeshComponent &
  operator=(const AMeshComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    MeshesNameArray = Source.MeshesNameArray;
    SubMeshesNameArray = Source.SubMeshesNameArray;
    OffsetPositionArray = Source.OffsetPositionArray;
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
  addMeshInfo(const std::string &MeshName,
              DirectX::XMFLOAT3 Offset = {0.f, 0.f, 0.f});

  void
  setEmissiveIntensity(float Intensity);
  float
  getEmissiveIntensity();

private:
  bool
  bindInstanceToAssetsPool(const std::string &MeshName);
  void
  syncTransformDataToInstance();

  friend class AAnimateComponent;
};
