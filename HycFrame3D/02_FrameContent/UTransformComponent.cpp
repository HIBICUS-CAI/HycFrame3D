#include "UTransformComponent.h"
#include "UiObject.h"

using namespace DirectX;

UTransformComponent::UTransformComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mPosition({ 0.f,0.f,0.f }), mProcessingPosition({ 0.f,0.f,0.f }),
    mRotation({ 0.f,0.f,0.f }), mProcessingRotation({ 0.f,0.f,0.f }),
    mScaling({ 0.f,0.f,0.f }), mProcessingScaling({ 0.f,0.f,0.f }),
    mPositionDirtyFlg(false), mRotationDirtyFlg(false), mScalingDirtyFlg(false)
{

}

UTransformComponent::UTransformComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mPosition({ 0.f,0.f,0.f }), mProcessingPosition({ 0.f,0.f,0.f }),
    mRotation({ 0.f,0.f,0.f }), mProcessingRotation({ 0.f,0.f,0.f }),
    mScaling({ 0.f,0.f,0.f }), mProcessingScaling({ 0.f,0.f,0.f }),
    mPositionDirtyFlg(false), mRotationDirtyFlg(false), mScalingDirtyFlg(false)
{

}

UTransformComponent::~UTransformComponent()
{

}

bool UTransformComponent::Init()
{
    return true;
}

void UTransformComponent::Update(Timer& _timer)
{
    ApplyProcessingData();
}

void UTransformComponent::Destory()
{

}

void UTransformComponent::SetPosition(DirectX::XMFLOAT3 _pos)
{
    mProcessingPosition = _pos;
    mPositionDirtyFlg = true;
}

void UTransformComponent::ForcePosition(DirectX::XMFLOAT3 _pos)
{
    mPosition = _pos;
}

void UTransformComponent::SetRotation(DirectX::XMFLOAT3 _angle)
{
    mProcessingRotation = _angle;
    mRotationDirtyFlg = true;
}

void UTransformComponent::ForceRotation(DirectX::XMFLOAT3 _angle)
{
    mRotation = _angle;
}

void UTransformComponent::SetScaling(DirectX::XMFLOAT3 _factor)
{
    mProcessingScaling = _factor;
    mScalingDirtyFlg = true;
}

void UTransformComponent::ForceScaling(DirectX::XMFLOAT3 _factor)
{
    mScaling = _factor;
}

void UTransformComponent::Translate(DirectX::XMFLOAT3 _deltaPos)
{
    DirectX::XMVECTOR point = DirectX::XMLoadFloat3(&mProcessingPosition);
    DirectX::XMVECTOR delta = DirectX::XMLoadFloat3(&_deltaPos);
    point += delta;
    DirectX::XMStoreFloat3(&mProcessingPosition, point);
    mPositionDirtyFlg = true;
}

void UTransformComponent::TranslateXAsix(float _deltaPosX)
{
    mProcessingPosition.x += _deltaPosX;
    mPositionDirtyFlg = true;
}

void UTransformComponent::TranslateYAsix(float _deltaPosY)
{
    mProcessingPosition.y += _deltaPosY;
    mPositionDirtyFlg = true;
}

void UTransformComponent::TranslateZAsix(float _deltaPosZ)
{
    mProcessingPosition.z += _deltaPosZ;
    mPositionDirtyFlg = true;
}

void UTransformComponent::Rotate(DirectX::XMFLOAT3 _deltaAngle)
{
    DirectX::XMVECTOR angle = DirectX::XMLoadFloat3(&mProcessingRotation);
    DirectX::XMVECTOR delta = DirectX::XMLoadFloat3(&_deltaAngle);
    angle += delta;
    DirectX::XMStoreFloat3(&mProcessingRotation, angle);
    mRotationDirtyFlg = true;
}

void UTransformComponent::RotateXAsix(float _deltaAngleX)
{
    mProcessingRotation.x += _deltaAngleX;
    mRotationDirtyFlg = true;
}

void UTransformComponent::RotateYAsix(float _deltaAngleY)
{
    mProcessingRotation.y += _deltaAngleY;
    mRotationDirtyFlg = true;
}

void UTransformComponent::RotateZAsix(float _deltaAngleZ)
{
    mProcessingRotation.z += _deltaAngleZ;
    mRotationDirtyFlg = true;
}

void UTransformComponent::Scale(DirectX::XMFLOAT3 _factor)
{
    mProcessingScaling = _factor;
    mScalingDirtyFlg = true;
}

void UTransformComponent::ScaleXAsix(float _factorX)
{
    mProcessingScaling.x = _factorX;
    mScalingDirtyFlg = true;
}

void UTransformComponent::ScaleYAsix(float _factorY)
{
    mProcessingScaling.y = _factorY;
    mScalingDirtyFlg = true;
}

void UTransformComponent::ScaleZAsix(float _factorZ)
{
    mProcessingScaling.z = _factorZ;
    mScalingDirtyFlg = true;
}

const DirectX::XMFLOAT3& UTransformComponent::GetPosition() const
{
    return mPosition;
}

const DirectX::XMFLOAT3& UTransformComponent::GetRotation() const
{
    return mRotation;
}

const DirectX::XMFLOAT3& UTransformComponent::GetScaling() const
{
    return mScaling;
}

const DirectX::XMFLOAT3& UTransformComponent::GetProcessingPosition() const
{
    return mProcessingPosition;
}

const DirectX::XMFLOAT3& UTransformComponent::GetProcessingRotation() const
{
    return mProcessingRotation;
}

const DirectX::XMFLOAT3& UTransformComponent::GetProcessingScaling() const
{
    return mProcessingScaling;
}

void UTransformComponent::ApplyProcessingData()
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
