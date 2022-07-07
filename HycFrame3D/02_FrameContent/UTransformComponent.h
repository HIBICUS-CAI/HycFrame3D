#pragma once

#include "UiComponent.h"

#include <DirectXMath.h>

namespace dx = DirectX;

class UTransformComponent : public UiComponent {
private:
  dx::XMFLOAT3 Position;
  dx::XMFLOAT3 Rotation;
  dx::XMFLOAT3 Scaling;

  dx::XMFLOAT3 ProcessingPosition;
  dx::XMFLOAT3 ProcessingRotation;
  dx::XMFLOAT3 ProcessingScaling;

  bool PositionDirtyFlg;
  bool RotationDirtyFlg;
  bool ScalingDirtyFlg;

public:
  UTransformComponent(const std::string &CompName, class UiObject *UiOwner);
  virtual ~UTransformComponent();

  UTransformComponent &operator=(const UTransformComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    Position = Source.Position;
    Rotation = Source.Rotation;
    Scaling = Source.Scaling;
    ProcessingPosition = Source.ProcessingPosition;
    ProcessingRotation = Source.ProcessingRotation;
    ProcessingScaling = Source.ProcessingScaling;
    PositionDirtyFlg = Source.PositionDirtyFlg;
    RotationDirtyFlg = Source.RotationDirtyFlg;
    ScalingDirtyFlg = Source.ScalingDirtyFlg;
    UiComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool init();
  virtual void update(Timer &Timer);
  virtual void destory();

public:
  void setPosition(const dx::XMFLOAT3 &Pos);
  void forcePosition(const dx::XMFLOAT3 &Pos);
  void setRotation(const dx::XMFLOAT3 &Angle);
  void forceRotation(const dx::XMFLOAT3 &Angle);
  void setScaling(const dx::XMFLOAT3 &Factor);
  void forceScaling(const dx::XMFLOAT3 &Factor);

  void translate(const dx::XMFLOAT3 &DeltaPos);
  void translateXAsix(float DeltaPosX);
  void translateYAsix(float DeltaPosY);
  void translateZAsix(float DeltaPosZ);

  void rotate(const dx::XMFLOAT3 &DeltaAngle);
  void rotateXAsix(float DeltaAngleX);
  void rotateYAsix(float DeltaAngleY);
  void rotateZAsix(float DeltaAngleZ);

  void scale(const dx::XMFLOAT3 &Factor);
  void scaleXAsix(float FactorX);
  void scaleYAsix(float FactorY);
  void scaleZAsix(float FactorZ);

  const dx::XMFLOAT3 &getPosition() const;
  const dx::XMFLOAT3 &getRotation() const;
  const dx::XMFLOAT3 &getScaling() const;
  const dx::XMFLOAT3 &getProcessingPosition() const;
  const dx::XMFLOAT3 &getProcessingRotation() const;
  const dx::XMFLOAT3 &getProcessingScaling() const;

private:
  void applyProcessingData();
};
