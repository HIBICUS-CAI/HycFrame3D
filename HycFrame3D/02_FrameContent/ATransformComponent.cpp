#include "ATransformComponent.h"

#include "ActorObject.h"

using namespace dx;

ATransformComponent::ATransformComponent(const std::string &CompName,
                                         ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), Position({0.f, 0.f, 0.f}),
      Rotation({0.f, 0.f, 0.f}), Scaling({0.f, 0.f, 0.f}),
      ProcessingPosition({0.f, 0.f, 0.f}), ProcessingRotation({0.f, 0.f, 0.f}),
      ProcessingScaling({0.f, 0.f, 0.f}), PositionDirtyFlag(false),
      RotationDirtyFlag(false), ScalingDirtyFlag(false) {}

ATransformComponent::~ATransformComponent() {}

bool
ATransformComponent::init() {
  return true;
}

void
ATransformComponent::update(Timer &Timer) {
  applyProcessingData();
}

void
ATransformComponent::destory() {}

void
ATransformComponent::setPosition(const dx::XMFLOAT3 &Pos) {
  ProcessingPosition = Pos;
  PositionDirtyFlag = true;
}

void
ATransformComponent::forcePosition(const dx::XMFLOAT3 &Pos) {
  Position = Pos;
  ProcessingPosition = Pos;
}

void
ATransformComponent::setRotation(const dx::XMFLOAT3 &Angle) {
  ProcessingRotation = Angle;
  RotationDirtyFlag = true;
}

void
ATransformComponent::forceRotation(const dx::XMFLOAT3 &Angle) {
  Rotation = Angle;
  ProcessingRotation = Angle;
}

void
ATransformComponent::setScaling(const dx::XMFLOAT3 &Factor) {
  ProcessingScaling = Factor;
  ScalingDirtyFlag = true;
}

void
ATransformComponent::forceScaling(const dx::XMFLOAT3 &Factor) {
  Scaling = Factor;
  ProcessingScaling = Factor;
}

void
ATransformComponent::translate(const dx::XMFLOAT3 &DeltaPos) {
  dx::XMVECTOR Point = dx::XMLoadFloat3(&ProcessingPosition);
  dx::XMVECTOR Delta = dx::XMLoadFloat3(&DeltaPos);
  Point += Delta;
  dx::XMStoreFloat3(&ProcessingPosition, Point);
  PositionDirtyFlag = true;
}

void
ATransformComponent::translateXAsix(float DeltaPosX) {
  ProcessingPosition.x += DeltaPosX;
  PositionDirtyFlag = true;
}

void
ATransformComponent::translateYAsix(float DeltaPosY) {
  ProcessingPosition.y += DeltaPosY;
  PositionDirtyFlag = true;
}

void
ATransformComponent::translateZAsix(float DeltaPosZ) {
  ProcessingPosition.z += DeltaPosZ;
  PositionDirtyFlag = true;
}

void
ATransformComponent::rotate(const dx::XMFLOAT3 &DeltaAngle) {
  dx::XMVECTOR Angle = dx::XMLoadFloat3(&ProcessingRotation);
  dx::XMVECTOR Delta = dx::XMLoadFloat3(&DeltaAngle);
  Angle += Delta;
  dx::XMStoreFloat3(&ProcessingRotation, Angle);
  RotationDirtyFlag = true;
}

void
ATransformComponent::rotateXAsix(float DeltaAngleX) {
  ProcessingRotation.x += DeltaAngleX;
  RotationDirtyFlag = true;
}

void
ATransformComponent::rotateYAsix(float DeltaAngleY) {
  ProcessingRotation.y += DeltaAngleY;
  RotationDirtyFlag = true;
}

void
ATransformComponent::rotateZAsix(float DeltaAngleZ) {
  ProcessingRotation.z += DeltaAngleZ;
  RotationDirtyFlag = true;
}

void
ATransformComponent::scale(const dx::XMFLOAT3 &Factor) {
  ProcessingScaling = Factor;
  ScalingDirtyFlag = true;
}

void
ATransformComponent::scaleXAsix(float FactorX) {
  ProcessingScaling.x = FactorX;
  ScalingDirtyFlag = true;
}

void
ATransformComponent::scaleYAsix(float FactorY) {
  ProcessingScaling.y = FactorY;
  ScalingDirtyFlag = true;
}

void
ATransformComponent::scaleZAsix(float FactorZ) {
  ProcessingScaling.z = FactorZ;
  ScalingDirtyFlag = true;
}

void
ATransformComponent::rollBackPosition() {
  ProcessingPosition = Position;
  PositionDirtyFlag = false;
}

void
ATransformComponent::rollBackRotation() {
  ProcessingRotation = Rotation;
  RotationDirtyFlag = false;
}

void
ATransformComponent::rollBackScaling() {
  ProcessingScaling = Scaling;
  ScalingDirtyFlag = false;
}

const dx::XMFLOAT3 &
ATransformComponent::getPosition() const {
  return Position;
}

const dx::XMFLOAT3 &
ATransformComponent::getRotation() const {
  return Rotation;
}

const dx::XMFLOAT3 &
ATransformComponent::getScaling() const {
  return Scaling;
}

const dx::XMFLOAT3 &
ATransformComponent::getProcessingPosition() const {
  return ProcessingPosition;
}

const dx::XMFLOAT3 &
ATransformComponent::getProcessingRotation() const {
  return ProcessingRotation;
}

const dx::XMFLOAT3 &
ATransformComponent::getProcessingScaling() const {
  return ProcessingScaling;
}

void
ATransformComponent::applyProcessingData() {
  if (PositionDirtyFlag) {
    Position = ProcessingPosition;
    PositionDirtyFlag = false;
  }

  if (RotationDirtyFlag) {
    Rotation = ProcessingRotation;
    RotationDirtyFlag = false;
  }
  if (PositionDirtyFlag) {
    Scaling = ProcessingScaling;
    PositionDirtyFlag = false;
  }
}

void
ATransformComponent::rollBackPositionX() {
  ProcessingPosition.x = Position.x;
}

void
ATransformComponent::rollBackPositionY() {
  ProcessingPosition.y = Position.y;
}

void
ATransformComponent::rollBackPositionZ() {
  ProcessingPosition.z = Position.z;
}

void
ATransformComponent::rollBackRotationX() {
  ProcessingRotation.x = Rotation.x;
}

void
ATransformComponent::rollBackRotationY() {
  ProcessingRotation.y = Rotation.y;
}

void
ATransformComponent::rollBackRotationZ() {
  ProcessingRotation.z = Rotation.z;
}

void
ATransformComponent::rollBackScalingX() {
  ProcessingScaling.x = Scaling.x;
}

void
ATransformComponent::rollBackScalingY() {
  ProcessingScaling.y = Scaling.y;
}

void
ATransformComponent::rollBackScalingZ() {
  ProcessingScaling.z = Scaling.z;
}
