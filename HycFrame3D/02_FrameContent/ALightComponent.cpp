#include "ALightComponent.h"
#include "ActorObject.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "ATransformComponent.h"
#include <vector>
#include "RSLight.h"
#include "RSRoot_DX11.h"
#include "RSLightsContainer.h"
#include "RSMeshHelper.h"

ALightComponent::ALightComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mRSLightPtr(nullptr),
    mLightName(""), mLightInfoForInit({}), mLightCamInfoForInit({}),
    mIsBloom(false), mIsCamera(false), mCanCreateLight(false)
{

}

ALightComponent::ALightComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mRSLightPtr(nullptr),
    mLightName(""), mLightInfoForInit({}), mLightCamInfoForInit({}),
    mIsBloom(false), mIsCamera(false), mCanCreateLight(false)
{

}

ALightComponent::~ALightComponent()
{

}

bool ALightComponent::Init()
{
    if (!mCanCreateLight) { return false; }

    CreateLight();

    if (mRSLightPtr) { return true; }
    else { return false; }
}

void ALightComponent::Update(Timer& _timer)
{
    SyncDataFromTransform();
}

void ALightComponent::Destory()
{
    GetRSRoot_DX11_Singleton()->LightsContainer()->DeleteRSLight(mLightName);
}

void ALightComponent::CreateLight()
{
    mLightName = GetCompName();
    mRSLightPtr = GetRSRoot_DX11_Singleton()->LightsContainer()->
        CreateRSLight(mLightName, &mLightInfoForInit);
#ifdef _DEBUG
    assert(mRSLightPtr);
#endif // _DEBUG

    if (mIsBloom)
    {
        MESH_DATA* mesh = GetActorOwner()->GetSceneNode().GetAssetsPool()->
            GetMeshIfExisted(BOX_BLOOM_MESH_NAME);
        if (!mesh)
        {
            RS_SUBMESH_DATA boxBloom = GetRSRoot_DX11_Singleton()->
                MeshHelper()->GeoGenerate()->
                CreateBox(1.f, 1.f, 1.f, 0, LAYOUT_TYPE::NORMAL_COLOR);
            GetActorOwner()->GetSceneNode().GetAssetsPool()->InsertNewMesh(
                BOX_BLOOM_MESH_NAME, boxBloom, MESH_TYPE::LIGHT);
            mesh = GetActorOwner()->GetSceneNode().GetAssetsPool()->
                GetMeshIfExisted(BOX_BLOOM_MESH_NAME);
        }
        mRSLightPtr->SetLightBloom(mesh->mMeshData);
    }

    if (mIsCamera)
    {
        bool cam_create = GetRSRoot_DX11_Singleton()->LightsContainer()->
            CreateLightCameraFor(mLightName, &mLightCamInfoForInit);
#ifdef _DEBUG
        assert(cam_create);
#endif // _DEBUG
    }
}

void ALightComponent::ResetLight(LIGHT_INFO* _lightInfo)
{
    mRSLightPtr->ResetRSLight(_lightInfo);
}

RSLight* ALightComponent::GetLightInfo()
{
    return mRSLightPtr;
}

void ALightComponent::SyncDataFromTransform()
{
    ATransformComponent* atc = GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
#ifdef _DEBUG
    assert(atc);
#endif // _DEBUG

    DirectX::XMFLOAT3 world = atc->GetPosition();
    DirectX::XMFLOAT3 angle = atc->GetRotation();
    DirectX::XMFLOAT3 scale = atc->GetScaling();

    mRSLightPtr->SetRSLightPosition(world);

    if (mIsBloom)
    {
        DirectX::XMMATRIX mat = {};
        mat = DirectX::XMMatrixMultiply(
            DirectX::XMMatrixScaling(scale.x, scale.y, scale.z),
            DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z));
        mat = DirectX::XMMatrixMultiply(mat,
            DirectX::XMMatrixTranslation(world.x, world.y, world.z));
        DirectX::XMStoreFloat4x4(mRSLightPtr->GetLightWorldMat(), mat);
    }
}

void ALightComponent::AddLight(LIGHT_INFO& _lightInfo,
    bool _setBloom, bool _setCamera, CAM_INFO& _camInfo)
{
    mLightInfoForInit = _lightInfo;
    mLightCamInfoForInit = _camInfo;
    mIsBloom = _setBloom;
    mIsCamera = _setCamera;
    mCanCreateLight = true;
}

void ALightComponent::AddLight(LIGHT_INFO& _lightInfo,
    bool _setBloom, bool _setCamera, CAM_INFO&& _camInfo)
{
    mLightInfoForInit = _lightInfo;
    mLightCamInfoForInit = _camInfo;
    mIsBloom = _setBloom;
    mIsCamera = _setCamera;
    mCanCreateLight = true;
}
