#pragma once

#include "UiComponent.h"
#include <DirectXMath.h>

class UTransformComponent :public UiComponent
{
public:
    UTransformComponent(std::string&& _compName, class UiObject* _uiOwner);
    UTransformComponent(std::string& _compName, class UiObject* _uiOwner);
    virtual ~UTransformComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void SetPosition(DirectX::XMFLOAT3 _pos);
    void ForcePosition(DirectX::XMFLOAT3 _pos);
    void SetRotation(DirectX::XMFLOAT3 _angle);
    void ForceRotation(DirectX::XMFLOAT3 _angle);
    void SetScaling(DirectX::XMFLOAT3 _factor);
    void ForceScaling(DirectX::XMFLOAT3 _factor);

    void Translate(DirectX::XMFLOAT3 _deltaPos);
    void TranslateXAsix(float _deltaPosX);
    void TranslateYAsix(float _deltaPosY);
    void TranslateZAsix(float _deltaPosZ);

    void Rotate(DirectX::XMFLOAT3 _deltaAngle);
    void RotateXAsix(float _deltaAngleX);
    void RotateYAsix(float _deltaAngleY);
    void RotateZAsix(float _deltaAngleZ);

    void Scale(DirectX::XMFLOAT3 _factor);
    void ScaleXAsix(float _factorX);
    void ScaleYAsix(float _factorY);
    void ScaleZAsix(float _factorZ);

    const DirectX::XMFLOAT3& GetPosition() const;
    const DirectX::XMFLOAT3& GetRotation() const;
    const DirectX::XMFLOAT3& GetScaling() const;
    const DirectX::XMFLOAT3& GetProcessingPosition() const;
    const DirectX::XMFLOAT3& GetProcessingRotation() const;
    const DirectX::XMFLOAT3& GetProcessingScaling() const;

private:
    void ApplyProcessingData();

private:
    DirectX::XMFLOAT3 mPosition;
    DirectX::XMFLOAT3 mRotation;
    DirectX::XMFLOAT3 mScaling;

    DirectX::XMFLOAT3 mProcessingPosition;
    DirectX::XMFLOAT3 mProcessingRotation;
    DirectX::XMFLOAT3 mProcessingScaling;

    bool mPositionDirtyFlg;
    bool mRotationDirtyFlg;
    bool mScalingDirtyFlg;
};
