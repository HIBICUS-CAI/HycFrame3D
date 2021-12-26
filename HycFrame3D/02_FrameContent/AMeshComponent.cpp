#include "AMeshComponent.h"
#include "ActorComponent.h"
#include "AssetsPool.h"
#include "SceneNode.h"
#include "ActorObject.h"
#include "ATransformComponent.h"

AMeshComponent::AMeshComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mMeshesName({}), mOffsetPosition({})
{

}

AMeshComponent::AMeshComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mMeshesName({}), mOffsetPosition({})
{

}

AMeshComponent::~AMeshComponent()
{

}

bool AMeshComponent::Init()
{
    for (auto& meshName : mMeshesName)
    {
        if (!BindInstanceToAssetsPool(meshName)) { return false; }
    }

    return true;
}

void AMeshComponent::Update(Timer& _timer)
{
    SyncTransformDataToInstance();
}

void AMeshComponent::Destory()
{
    for (auto& meshName : mMeshesName)
    {
        MESH_DATA* mesh = GetActorOwner()->GetSceneNode().GetAssetsPool()->
            GetMeshIfExisted(meshName);
        if (mesh) { mesh->mInstanceMap.erase(GetCompName()); }
    }
}

void AMeshComponent::AddMeshInfo(std::string&& _meshName, DirectX::XMFLOAT3 _offset)
{
    mMeshesName.push_back(_meshName);
    mOffsetPosition.push_back(_offset);
}

void AMeshComponent::AddMeshInfo(std::string& _meshName, DirectX::XMFLOAT3 _offset)
{
    mMeshesName.push_back(_meshName);
    mOffsetPosition.push_back(_offset);
}

bool AMeshComponent::BindInstanceToAssetsPool(std::string& _meshName)
{
    MESH_DATA* mesh = GetActorOwner()->GetSceneNode().GetAssetsPool()->
        GetMeshIfExisted(_meshName);
    if (!mesh) { return false; }

    mesh->mInstanceMap.insert({ GetCompName(),{} });
    mesh->mInstanceMap[GetCompName()].mMaterialData = mesh->mMeshData.mMaterial;
    if (mesh->mMeshData.mTextures.size() > 1)
    {
        mesh->mInstanceMap[GetCompName()].mCustomizedData1.x = 1.f;
    }
    else
    {
        mesh->mInstanceMap[GetCompName()].mCustomizedData1.x = -1.f;
    }

    return true;
}

void AMeshComponent::SyncTransformDataToInstance()
{
    ATransformComponent* atc = GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
#ifdef _DEBUG
    assert(atc);
#endif // _DEBUG
    size_t index = 0;

    for (auto& meshName : mMeshesName)
    {
        auto& ins_data = GetActorOwner()->GetSceneNode().GetAssetsPool()->
            GetMeshIfExisted(meshName)->mInstanceMap[GetCompName()];
        DirectX::XMFLOAT3 delta = mOffsetPosition[index];
        DirectX::XMFLOAT3 world = atc->GetPosition();
        DirectX::XMFLOAT3 angle = atc->GetRotation();
        DirectX::XMFLOAT3 scale = atc->GetScaling();

        DirectX::XMMATRIX mat = {};
        mat = DirectX::XMMatrixMultiply(
            DirectX::XMMatrixTranslation(delta.x, delta.y, delta.z),
            DirectX::XMMatrixScaling(scale.x, scale.y, scale.z));
        mat = DirectX::XMMatrixMultiply(mat,
            DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z));
        mat = DirectX::XMMatrixMultiply(mat,
            DirectX::XMMatrixTranslation(world.x, world.y, world.z));
        DirectX::XMStoreFloat4x4(&(ins_data.mWorldMat), mat);

        ++index;
    }
}
