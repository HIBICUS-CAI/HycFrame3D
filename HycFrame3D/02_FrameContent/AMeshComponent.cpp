#include "AMeshComponent.h"
#include "ActorComponent.h"
#include "AssetsPool.h"
#include "SceneNode.h"
#include "ActorObject.h"
#include "ATransformComponent.h"
#include <set>

AMeshComponent::AMeshComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mMeshesName({}),
    mSubMeshesName({}),
    mOffsetPosition({}),
    mEmissiveIntensity(0.f)
{

}

AMeshComponent::AMeshComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mMeshesName({}),
    mSubMeshesName({}),
    mOffsetPosition({}),
    mEmissiveIntensity(0.f)
{

}

AMeshComponent::~AMeshComponent()
{

}

bool AMeshComponent::Init()
{
    int meshIndex = 0;
    std::vector<DirectX::XMFLOAT3>* tempOffset = new std::vector<DirectX::XMFLOAT3>;
    if (!tempOffset) { return false; }
    tempOffset->reserve(256);
    for (auto& meshName : mMeshesName)
    {
        auto subVec = GetActorOwner()->GetSceneNode().GetAssetsPool()->
            getMeshIfExisted(meshName);
#ifdef _DEBUG
        assert(subVec);
#endif // _DEBUG
        auto subSize = subVec->size();
        for (size_t i = 0; i < subSize; i++)
        {
            mSubMeshesName.push_back((*subVec)[i]);
            tempOffset->push_back(mOffsetPosition[meshIndex]);
        }
        ++meshIndex;
    }
    auto allOffset = tempOffset->size();
    mOffsetPosition.clear(); mOffsetPosition.resize(allOffset);
    for (size_t i = 0; i < allOffset; i++)
    {
        mOffsetPosition[i] = tempOffset->at(i);
    }
    tempOffset->clear();
    delete tempOffset;

    for (auto& meshName : mSubMeshesName)
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
    for (auto& meshName : mSubMeshesName)
    {
        SUBMESH_DATA* mesh = GetActorOwner()->GetSceneNode().GetAssetsPool()->
            getSubMeshIfExisted(meshName);
        if (mesh) { mesh->InstanceMap.erase(GetCompName()); }
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

void AMeshComponent::SetEmissiveIntensity(float _intensity)
{
    mEmissiveIntensity = _intensity;
    if (mEmissiveIntensity > 255.f) { mEmissiveIntensity = 255.f; }
}

float AMeshComponent::GetEmissiveIntensity()
{
    return mEmissiveIntensity;
}

bool AMeshComponent::BindInstanceToAssetsPool(std::string& _meshName)
{
    SUBMESH_DATA* mesh = GetActorOwner()->GetSceneNode().GetAssetsPool()->
        getSubMeshIfExisted(_meshName);
    if (!mesh) { return false; }

    RS_INSTANCE_DATA id = {};
    id.MaterialData = mesh->MeshData.Material;
    if (mesh->MeshData.Textures[1] != "") { id.CustomizedData1.x = 1.f; }
    else { id.CustomizedData1.x = -1.f; }
    if (mesh->MeshData.Textures[2] != "") { id.CustomizedData1.y = 1.f; }
    else { id.CustomizedData1.y = -1.f; }
    if (mesh->MeshData.Textures[3] != "") { id.CustomizedData1.z = 1.f; }
    else { id.CustomizedData1.z = -1.f; }
    if (mesh->MeshData.Textures[4] != "") { id.CustomizedData1.w = 1.f; }
    else { id.CustomizedData1.w = -1.f; }
    if (mesh->MeshData.Textures[4] != "") { id.CustomizedData2.x = mEmissiveIntensity; }
    else { id.CustomizedData2.x = 0.f; }

    mesh->InstanceMap.insert({ GetCompName(),id });

    return true;
}

void AMeshComponent::SyncTransformDataToInstance()
{
    ATransformComponent* atc = GetActorOwner()->
        GetComponent<ATransformComponent>();
#ifdef _DEBUG
    assert(atc);
#endif // _DEBUG
    size_t index = 0;
    std::set<std::string> hasChecked = {};

    for (auto& meshName : mSubMeshesName)
    {
        if (hasChecked.find(meshName) != hasChecked.end()) { continue; }

        auto& ins_map = GetActorOwner()->GetSceneNode().GetAssetsPool()->
            getSubMeshIfExisted(meshName)->InstanceMap;
        std::pair<
            std::unordered_multimap<std::string, RS_INSTANCE_DATA>::iterator,
            std::unordered_multimap<std::string, RS_INSTANCE_DATA>::iterator>
            myRange;
        myRange = ins_map.equal_range(GetCompName());

        for (auto& it = myRange.first; it != myRange.second; ++it)
        {
            auto& ins_data = it->second;
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
            DirectX::XMStoreFloat4x4(&(ins_data.WorldMatrix), mat);

            ins_data.CustomizedData2.x = mEmissiveIntensity;

            ++index;
        }

        hasChecked.insert(meshName);
    }
}
