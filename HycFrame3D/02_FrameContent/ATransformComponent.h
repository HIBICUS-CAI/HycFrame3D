#pragma once

#include "ActorComponent.h"

#include <DirectXMath.h>

namespace dx = DirectX;

class ATransformComponent : public ActorComponent {
private:
  dx::XMFLOAT3 Position;
  dx::XMFLOAT3 Rotation;
  dx::XMFLOAT3 Scaling;

  dx::XMFLOAT3 ProcessingPosition;
  dx::XMFLOAT3 ProcessingRotation;
  dx::XMFLOAT3 ProcessingScaling;

  bool PositionDirtyFlag;
  bool RotationDirtyFlag;
  bool ScalingDirtyFlag;

public:
  ATransformComponent(const std::string &CompName,
                      class ActorObject *ActorOwner);
  virtual ~ATransformComponent();

  ATransformComponent &operator=(const ATransformComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    Position = Source.Position;
    ProcessingPosition = Source.ProcessingPosition;
    PositionDirtyFlag = Source.PositionDirtyFlag;
    Rotation = Source.Rotation;
    ProcessingRotation = Source.ProcessingRotation;
    RotationDirtyFlag = Source.RotationDirtyFlag;
    Scaling = Source.Scaling;
    ProcessingScaling = Source.ProcessingScaling;
    ScalingDirtyFlag = Source.ScalingDirtyFlag;
    ActorComponent::operator=(Source);
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

  void rollBackPosition();
  void rollBackPositionX();
  void rollBackPositionY();
  void rollBackPositionZ();
  void rollBackRotation();
  void rollBackRotationX();
  void rollBackRotationY();
  void rollBackRotationZ();
  void rollBackScaling();
  void rollBackScalingX();
  void rollBackScalingY();
  void rollBackScalingZ();

  const dx::XMFLOAT3 &getPosition() const;
  const dx::XMFLOAT3 &getRotation() const;
  const dx::XMFLOAT3 &getScaling() const;
  const dx::XMFLOAT3 &getProcessingPosition() const;
  const dx::XMFLOAT3 &getProcessingRotation() const;
  const dx::XMFLOAT3 &getProcessingScaling() const;

private:
  void applyProcessingData();
};
