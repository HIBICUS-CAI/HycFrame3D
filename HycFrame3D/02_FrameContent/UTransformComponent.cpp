#include "UTransformComponent.h"

#include "UiObject.h"

using namespace dx;

UTransformComponent::UTransformComponent(const std::string &CompName,
                                         UiObject *UiOwner)
    : UiComponent(CompName, UiOwner), Position({0.f, 0.f, 0.f}),
      Rotation({0.f, 0.f, 0.f}), Scaling({0.f, 0.f, 0.f}),
      ProcessingPosition({0.f, 0.f, 0.f}), ProcessingRotation({0.f, 0.f, 0.f}),
      ProcessingScaling({0.f, 0.f, 0.f}), PositionDirtyFlg(false),
      RotationDirtyFlg(false), ScalingDirtyFlg(false) {}

UTransformComponent::~UTransformComponent() {}

bool
UTransformComponent::init() {
  return true;
}

void
UTransformComponent::update(Timer &Timer) {
  applyProcessingData();
}

void
UTransformComponent::destory() {}

void
UTransformComponent::setPosition(const dx::XMFLOAT3 &Pos) {
  ProcessingPosition = Pos;
  PositionDirtyFlg = true;
}

void
UTransformComponent::forcePosition(const dx::XMFLOAT3 &Pos) {
  Position = Pos;
  ProcessingPosition = Pos;
}

void
UTransformComponent::setRotation(const dx::XMFLOAT3 &Angle) {
  ProcessingRotation = Angle;
  RotationDirtyFlg = true;
}

void
UTransformComponent::forceRotation(const dx::XMFLOAT3 &Angle) {
  Rotation = Angle;
  ProcessingRotation = Angle;
}

void
UTransformComponent::setScaling(const dx::XMFLOAT3 &Factor) {
  ProcessingScaling = Factor;
  ScalingDirtyFlg = true;
}

void
UTransformComponent::forceScaling(const dx::XMFLOAT3 &Factor) {
  Scaling = Factor;
  ProcessingScaling = Factor;
}

void
UTransformComponent::translate(const dx::XMFLOAT3 &DeltaPos) {
  dx::XMVECTOR Point = dx::XMLoadFloat3(&ProcessingPosition);
  dx::XMVECTOR Delta = dx::XMLoadFloat3(&DeltaPos);
  Point += Delta;
  dx::XMStoreFloat3(&ProcessingPosition, Point);
  PositionDirtyFlg = true;
}

void
UTransformComponent::translateXAsix(float DeltaPosX) {
  ProcessingPosition.x += DeltaPosX;
  PositionDirtyFlg = true;
}

void
UTransformComponent::translateYAsix(float DeltaPosY) {
  ProcessingPosition.y += DeltaPosY;
  PositionDirtyFlg = true;
}

void
UTransformComponent::translateZAsix(float DeltaPosZ) {
  ProcessingPosition.z += DeltaPosZ;
  PositionDirtyFlg = true;
}

void
UTransformComponent::rotate(const dx::XMFLOAT3 &DeltaAngle) {
  dx::XMVECTOR Angle = dx::XMLoadFloat3(&ProcessingRotation);
  dx::XMVECTOR Delta = dx::XMLoadFloat3(&DeltaAngle);
  Angle += Delta;
  dx::XMStoreFloat3(&ProcessingRotation, Angle);
  RotationDirtyFlg = true;
}

void
UTransformComponent::rotateXAsix(float DeltaAngleX) {
  ProcessingRotation.x += DeltaAngleX;
  RotationDirtyFlg = true;
}

void
UTransformComponent::rotateYAsix(float DeltaAngleY) {
  ProcessingRotation.y += DeltaAngleY;
  RotationDirtyFlg = true;
}

void
UTransformComponent::rotateZAsix(float DeltaAngleZ) {
  ProcessingRotation.z += DeltaAngleZ;
  RotationDirtyFlg = true;
}

void
UTransformComponent::scale(const dx::XMFLOAT3 &Factor) {
  ProcessingScaling = Factor;
  ScalingDirtyFlg = true;
}

void
UTransformComponent::scaleXAsix(float FactorX) {
  ProcessingScaling.x = FactorX;
  ScalingDirtyFlg = true;
}

void
UTransformComponent::scaleYAsix(float FactorY) {
  ProcessingScaling.y = FactorY;
  ScalingDirtyFlg = true;
}

void
UTransformComponent::scaleZAsix(float FactorZ) {
  ProcessingScaling.z = FactorZ;
  ScalingDirtyFlg = true;
}

const dx::XMFLOAT3 &
UTransformComponent::getPosition() const {
  return Position;
}

const dx::XMFLOAT3 &
UTransformComponent::getRotation() const {
  return Rotation;
}

const dx::XMFLOAT3 &
UTransformComponent::getScaling() const {
  return Scaling;
}

const dx::XMFLOAT3 &
UTransformComponent::getProcessingPosition() const {
  return ProcessingPosition;
}

const dx::XMFLOAT3 &
UTransformComponent::getProcessingRotation() const {
  return ProcessingRotation;
}

const dx::XMFLOAT3 &
UTransformComponent::getProcessingScaling() const {
  return ProcessingScaling;
}

void
UTransformComponent::applyProcessingData() {
  if (PositionDirtyFlg) {
    Position = ProcessingPosition;
    PositionDirtyFlg = false;
  }

  if (RotationDirtyFlg) {
    Rotation = ProcessingRotation;
    RotationDirtyFlg = false;
  }
  if (PositionDirtyFlg) {
    Scaling = ProcessingScaling;
    PositionDirtyFlg = false;
  }
}
