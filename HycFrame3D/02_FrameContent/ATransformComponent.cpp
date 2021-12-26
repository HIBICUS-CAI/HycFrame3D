#include "ATransformComponent.h"
#include "ActorObject.h"

using namespace DirectX;

ATransformComponent::ATransformComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mPosition({ 0.f,0.f,0.f }), mProcessingPosition({ 0.f,0.f,0.f }),
    mRotation({ 0.f,0.f,0.f }), mProcessingRotation({ 0.f,0.f,0.f }),
    mScaling({ 0.f,0.f,0.f }), mProcessingScaling({ 0.f,0.f,0.f }),
    mPositionDirtyFlg(false), mRotationDirtyFlg(false), mScalingDirtyFlg(false)
{

}

ATransformComponent::ATransformComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mPosition({ 0.f,0.f,0.f }), mProcessingPosition({ 0.f,0.f,0.f }),
    mRotation({ 0.f,0.f,0.f }), mProcessingRotation({ 0.f,0.f,0.f }),
    mScaling({ 0.f,0.f,0.f }), mProcessingScaling({ 0.f,0.f,0.f }),
    mPositionDirtyFlg(false), mRotationDirtyFlg(false), mScalingDirtyFlg(false)
{

}

ATransformComponent::~ATransformComponent()
{

}

bool ATransformComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void ATransformComponent::Update(Timer& _timer)
{
    ApplyProcessingData();
}

void ATransformComponent::Destory()
{

}

void ATransformComponent::SetPosition(DirectX::XMFLOAT3 _pos)
{
    mProcessingPosition = _pos;
    mPositionDirtyFlg = true;
}

void ATransformComponent::ForcePosition(DirectX::XMFLOAT3 _pos)
{
    mPosition = _pos;
}

void ATransformComponent::SetRotation(DirectX::XMFLOAT3 _angle)
{
    mProcessingRotation = _angle;
    mRotationDirtyFlg = true;
}

void ATransformComponent::ForceRotation(DirectX::XMFLOAT3 _angle)
{
    mRotation = _angle;
}

void ATransformComponent::SetScaling(DirectX::XMFLOAT3 _factor)
{
    mProcessingScaling = _factor;
    mScalingDirtyFlg = true;
}

void ATransformComponent::ForceScaling(DirectX::XMFLOAT3 _factor)
{
    mScaling = _factor;
}

void ATransformComponent::Translate(DirectX::XMFLOAT3 _deltaPos)
{
    DirectX::XMVECTOR point = DirectX::XMLoadFloat3(&mProcessingPosition);
    DirectX::XMVECTOR delta = DirectX::XMLoadFloat3(&_deltaPos);
    point += delta;
    DirectX::XMStoreFloat3(&mProcessingPosition, point);
    mPositionDirtyFlg = true;
}

void ATransformComponent::TranslateXAsix(float _deltaPosX)
{
    mProcessingPosition.x += _deltaPosX;
    mPositionDirtyFlg = true;
}

void ATransformComponent::TranslateYAsix(float _deltaPosY)
{
    mProcessingPosition.y += _deltaPosY;
    mPositionDirtyFlg = true;
}

void ATransformComponent::TranslateZAsix(float _deltaPosZ)
{
    mProcessingPosition.z += _deltaPosZ;
    mPositionDirtyFlg = true;
}

void ATransformComponent::Rotate(DirectX::XMFLOAT3 _deltaAngle)
{
    DirectX::XMVECTOR angle = DirectX::XMLoadFloat3(&mProcessingRotation);
    DirectX::XMVECTOR delta = DirectX::XMLoadFloat3(&_deltaAngle);
    angle += delta;
    DirectX::XMStoreFloat3(&mProcessingRotation, angle);
    mRotationDirtyFlg = true;
}

void ATransformComponent::RotateXAsix(float _deltaAngleX)
{
    mProcessingRotation.x += _deltaAngleX;
    mRotationDirtyFlg = true;
}

void ATransformComponent::RotateYAsix(float _deltaAngleY)
{
    mProcessingRotation.y += _deltaAngleY;
    mRotationDirtyFlg = true;
}

void ATransformComponent::RotateZAsix(float _deltaAngleZ)
{
    mProcessingRotation.z += _deltaAngleZ;
    mRotationDirtyFlg = true;
}

void ATransformComponent::Scale(DirectX::XMFLOAT3 _factor)
{
    mProcessingScaling = _factor;
    mScalingDirtyFlg = true;
}

void ATransformComponent::ScaleXAsix(float _factorX)
{
    mProcessingScaling.x = _factorX;
    mScalingDirtyFlg = true;
}

void ATransformComponent::ScaleYAsix(float _factorY)
{
    mProcessingScaling.y = _factorY;
    mScalingDirtyFlg = true;
}

void ATransformComponent::ScaleZAsix(float _factorZ)
{
    mProcessingScaling.z = _factorZ;
    mScalingDirtyFlg = true;
}

const DirectX::XMFLOAT3& ATransformComponent::GetPosition() const
{
    return mPosition;
}

const DirectX::XMFLOAT3& ATransformComponent::GetRotation() const
{
    return mRotation;
}

const DirectX::XMFLOAT3& ATransformComponent::GetScaling() const
{
    return mScaling;
}

const DirectX::XMFLOAT3& ATransformComponent::GetProcessingPosition() const
{
    return mProcessingPosition;
}

const DirectX::XMFLOAT3& ATransformComponent::GetProcessingRotation() const
{
    return mProcessingRotation;
}

const DirectX::XMFLOAT3& ATransformComponent::GetProcessingScaling() const
{
    return mProcessingScaling;
}

void ATransformComponent::ApplyProcessingData()
{
    if (mPositionDirtyFlg)
    {
        mPosition = mProcessingPosition;
        mPositionDirtyFlg = false;
    }

    if (mRotationDirtyFlg)
    {
        mRotation = mProcessingRotation;
        mRotationDirtyFlg = false;
    }
    if (mPositionDirtyFlg)
    {
        mScaling = mProcessingScaling;
        mPositionDirtyFlg = false;
    }
}
