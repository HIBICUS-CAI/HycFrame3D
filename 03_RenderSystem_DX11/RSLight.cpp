//---------------------------------------------------------------
// File: RSLight.h
// Proj: RenderSystem_DX11
// Info: 对一个光源的基本内容进行描述及相关处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSLight.h"
#include "RSCamerasContainer.h"
#include "RSRoot_DX11.h"
#include "RSResourceManager.h"
#include "RSDrawCallsPool.h"
#include "RSMeshHelper.h"
#include "RSCamera.h"

RSLight::RSLight(LIGHT_INFO* _info) :
    mLightType(_info->Type),
    mWithShadow(_info->ShadowFlag),
    mIntensity(_info->Intensity),
    mLightStrength(_info->Albedo),
    mLightDirection(_info->Direction),
    mLightPosition(_info->Position),
    mLightFallOffStart(_info->FalloffStart),
    mLightFallOffEnd(_info->FalloffEnd),
    mLightSpotPower(_info->SpotPower),
    mRSLightInfo({
        mIntensity, mLightStrength, mLightFallOffStart, mLightDirection,
        mLightFallOffEnd, mLightPosition, mLightSpotPower
        }),
    mRSLightCamera(nullptr), mBloomLightFlg(false),
    mLightMeshData({}), mLightInstanceData({}),
    mLightDrawCallData({})
{

}

RSLight::~RSLight()
{

}

RS_LIGHT_INFO* RSLight::GetRSLightInfo()
{
    return &mRSLightInfo;
}

LIGHT_TYPE RSLight::GetRSLightType()
{
    return mLightType;
}

void RSLight::ResetRSLight(LIGHT_INFO* _info)
{
    mLightType = _info->Type;
    SetRSLightIntensity(_info->Intensity);
    SetRSLightStrength(_info->Albedo);
    SetRSLightDirection(_info->Direction);
    SetRSLightPosition(_info->Position);
    SetRSLightFallOff(_info->FalloffStart, _info->FalloffEnd);
    SetRSLightSpotPower(_info->SpotPower);
}

void RSLight::SetRSLightStrength(DirectX::XMFLOAT3 _strength)
{
    mLightStrength = _strength;
    mRSLightInfo.Albedo = _strength;
}

void RSLight::SetRSLightDirection(DirectX::XMFLOAT3 _direction)
{
    mLightDirection = _direction;
    mRSLightInfo.Direction = _direction;
}

void RSLight::SetRSLightPosition(DirectX::XMFLOAT3 _position)
{
    mLightPosition = _position;
    mRSLightInfo.Position = _position;

    if (mRSLightCamera)
    {
        mRSLightCamera->ChangeRSCameraPosition(_position);
    }
    if (mBloomLightFlg)
    {
        static DirectX::XMMATRIX mat = {};
        mat = DirectX::XMMatrixTranslation(
            mLightPosition.x, mLightPosition.y, mLightPosition.z);
        DirectX::XMStoreFloat4x4(&(mLightInstanceData[0].WorldMatrix), mat);
    }
}

void RSLight::SetRSLightFallOff(float _start, float _end)
{
    mLightFallOffStart = _start;
    mLightFallOffEnd = _end;
    mRSLightInfo.FalloffStart = _start;
    mRSLightInfo.FalloffEnd = _end;
}

void RSLight::SetRSLightSpotPower(float _power)
{
    mLightSpotPower = _power;
    mRSLightInfo.SpotPower = _power;
}

void RSLight::SetRSLightIntensity(float _power)
{
    mIntensity = _power;
    mRSLightInfo.Intensity = _power;
}

RSCamera* RSLight::CreateLightCamera(std::string& _lightName,
    CAM_INFO* _info, RSCamerasContainer* _camContainer)
{
    if (!_info || !_camContainer) { return nullptr; }

    std::string name = _lightName + "-light-cam";
    mRSLightCamera = _camContainer->CreateRSCamera(name, _info);

    return mRSLightCamera;
}

RSCamera* RSLight::GetRSLightCamera()
{
    return mRSLightCamera;
}

void RSLight::SetLightBloom(RS_SUBMESH_DATA& _meshData)
{
    mBloomLightFlg = true;
    mLightMeshData = _meshData;
    mLightInstanceData.resize(1);
    mLightInstanceData[0].CustomizedData1.x = mLightStrength.x;
    mLightInstanceData[0].CustomizedData1.y = mLightStrength.y;
    mLightInstanceData[0].CustomizedData1.z = mLightStrength.z;
    mLightInstanceData[0].CustomizedData1.w = mIntensity;
    static DirectX::XMMATRIX mat = {};
    mat = DirectX::XMMatrixTranslation(
        mLightPosition.x, mLightPosition.y, mLightPosition.z);
    DirectX::XMStoreFloat4x4(&(mLightInstanceData[0].WorldMatrix),
        mat);
    mLightDrawCallData.InstanceData.DataArrayPtr = &mLightInstanceData;
    mLightDrawCallData.MeshData.IndexBuffer =
        mLightMeshData.IndexBuffer;
    mLightDrawCallData.MeshData.VertexBuffer =
        mLightMeshData.VertexBuffer;
    mLightDrawCallData.MeshData.IndexSize =
        mLightMeshData.IndexSize;
    mLightDrawCallData.MeshData.InputLayout =
        mLightMeshData.InputLayout;
    mLightDrawCallData.MeshData.TopologyType =
        mLightMeshData.TopologyType;
}

void RSLight::UpdateBloomColor()
{
    if (mBloomLightFlg)
    {
        mLightInstanceData[0].CustomizedData1.x = mLightStrength.x;
        mLightInstanceData[0].CustomizedData1.y = mLightStrength.y;
        mLightInstanceData[0].CustomizedData1.z = mLightStrength.z;
        mLightInstanceData[0].CustomizedData1.w = mIntensity;
    }
}

void RSLight::UploadLightDrawCall()
{
    static auto pool = getRSDX11RootInstance()->
        getDrawCallsPool();
    if (mBloomLightFlg)
    {
        pool->AddDrawCallToPipe(DRAWCALL_TYPE::LIGHT,
            mLightDrawCallData);
    }
}

void RSLight::ReleaseLightBloom(bool _deleteByFrame)
{
    if (mBloomLightFlg && !_deleteByFrame)
    {
        getRSDX11RootInstance()->getMeshHelper()->ReleaseSubMesh(
            mLightMeshData);
    }
}

DirectX::XMFLOAT4X4* RSLight::GetLightWorldMat()
{
    if (!mLightInstanceData.size()) { return nullptr; }
    return &(mLightInstanceData[0].WorldMatrix);
}
