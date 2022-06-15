#include "ASpriteComponent.h"
#include "ActorObject.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "RSRoot_DX11.h"
#include "RSMeshHelper.h"
#include "ATransformComponent.h"

ASpriteComponent::ASpriteComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mGeoPointName(""), mTextureName(""), mIsBillboard(false),
    mSize({ 10.f,10.f }), mTexCoord({ 0.f,0.f,1.f,1.f }),
    mWithAnimation(false), mStride({ 0.f,0.f }), mMaxCut(0),
    mRepeatFlg(false), mSwitchTime(0.f)
{

}

ASpriteComponent::ASpriteComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mGeoPointName(""), mTextureName(""), mIsBillboard(false),
    mSize({ 10.f,10.f }), mTexCoord({ 0.f,0.f,1.f,1.f }),
    mWithAnimation(false), mStride({ 0.f,0.f }), mMaxCut(0),
    mRepeatFlg(false), mSwitchTime(0.f)
{

}

ASpriteComponent::~ASpriteComponent()
{

}

bool ASpriteComponent::Init()
{
    return true;
}

void ASpriteComponent::Update(Timer& _timer)
{
    SyncTransformDataToInstance();
}

void ASpriteComponent::Destory()
{
    SUBMESH_DATA* mesh = GetActorOwner()->GetSceneNode().GetAssetsPool()->
        GetSubMeshIfExisted(mGeoPointName);
    if (mesh) { mesh->mInstanceMap.erase(GetCompName()); }
}

bool ASpriteComponent::CreateGeoPointWithTexture(SceneNode* _scene,
    std::string& _texName)
{
    if (!_scene) { return false; }

    RS_SUBMESH_DATA point = GetRSRoot_DX11_Singleton()->
        MeshHelper()->GeoGenerate()->
        CreatePointWithTexture(LAYOUT_TYPE::NORMAL_TEX,
            _texName.c_str());
    mGeoPointName = GetCompName();
    mTextureName = _texName;
    _scene->GetAssetsPool()->InsertNewSubMesh(mGeoPointName,
        point,
        MESH_TYPE::TRANSPARENCY);

    SUBMESH_DATA* spriteRect = _scene->GetAssetsPool()->
        GetSubMeshIfExisted(mGeoPointName);
    if (!spriteRect) { return false; }

    RS_INSTANCE_DATA id = {};
    id.mCustomizedData1 = { mSize.x, mSize.y, (mIsBillboard ? 1.f : 0.f), 0.f };
    id.mCustomizedData2 = mTexCoord;
    spriteRect->mInstanceMap.insert({ mGeoPointName,id });

    return true;
}

bool ASpriteComponent::CreateGeoPointWithTexture(SceneNode* _scene,
    std::string&& _texName)
{
    if (!_scene) { return false; }

    RS_SUBMESH_DATA point = GetRSRoot_DX11_Singleton()->
        MeshHelper()->GeoGenerate()->
        CreatePointWithTexture(LAYOUT_TYPE::NORMAL_TEX,
            _texName.c_str());
    mGeoPointName = GetCompName();
    mTextureName = _texName;
    _scene->GetAssetsPool()->InsertNewSubMesh(mGeoPointName,
        point,
        MESH_TYPE::TRANSPARENCY);

    SUBMESH_DATA* spriteRect = _scene->GetAssetsPool()->
        GetSubMeshIfExisted(mGeoPointName);
    if (!spriteRect) { return false; }

    RS_INSTANCE_DATA id = {};
    id.mCustomizedData1 = { mSize.x, mSize.y, (mIsBillboard ? 1.f : 0.f), 0.f };
    id.mCustomizedData2 = mTexCoord;
    spriteRect->mInstanceMap.insert({ mGeoPointName,id });

    return true;
}

void ASpriteComponent::SetSpriteProperty(DirectX::XMFLOAT2 _size,
    DirectX::XMFLOAT4 _texCoord, bool _isBillboard)
{
    mIsBillboard = _isBillboard;
    mSize = _size;
    mTexCoord = _texCoord;
}

void ASpriteComponent::SetAnimationProperty(DirectX::XMFLOAT2 _stride,
    UINT _maxCut, bool _repeatFlg, float _switchTime)
{
    mWithAnimation = true;
    mStride = _stride;
    mMaxCut = _maxCut;
    mSwitchTime = _switchTime;
    mRepeatFlg = _repeatFlg;
}

void ASpriteComponent::SyncTransformDataToInstance()
{
    ATransformComponent* atc = GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
#ifdef _DEBUG
    assert(atc);
#endif // _DEBUG

    DirectX::XMFLOAT3 world = atc->GetPosition();
    DirectX::XMFLOAT3 angle = atc->GetRotation();
    DirectX::XMFLOAT3 scale = atc->GetScaling();

    std::string compName = GetCompName();
    auto& map = GetActorOwner()->GetSceneNode().GetAssetsPool()->
        GetSubMeshIfExisted(compName)->mInstanceMap;

    for (auto& ins : map)
    {
        auto& ins_data = ins.second;

        DirectX::XMMATRIX mat = {};
        mat = DirectX::XMMatrixMultiply(
            DirectX::XMMatrixScaling(scale.x, scale.y, scale.z),
            DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z));
        mat = DirectX::XMMatrixMultiply(mat,
            DirectX::XMMatrixTranslation(world.x, world.y, world.z));
        DirectX::XMStoreFloat4x4(&(ins_data.mWorldMat), mat);

        break;
    }
}
