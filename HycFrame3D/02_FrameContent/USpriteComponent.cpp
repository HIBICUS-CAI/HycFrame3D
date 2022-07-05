#include "USpriteComponent.h"
#include "UiObject.h"
#include "AssetsPool.h"
#include "SceneNode.h"
#include "UTransformComponent.h"
#include "UAnimateComponent.h"
#include "RSMeshHelper.h"
#include "RSRoot_DX11.h"

USpriteComponent::USpriteComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mMeshesName(""),
    mOriginTextureName(""),
    mOffsetColor({ 1.f,1.f,1.f,1.f })
{

}

USpriteComponent::USpriteComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mMeshesName(""),
    mOriginTextureName(""),
    mOffsetColor({ 1.f,1.f,1.f,1.f })
{

}

USpriteComponent::~USpriteComponent()
{

}

bool USpriteComponent::Init()
{
    if (mMeshesName == "")
    {
        P_LOG(LOG_WARNING, "this sprite hasnt been built : %s\n",
            GetCompName().c_str());
        return false;
    }

    return true;
}

void USpriteComponent::Update(Timer& _timer)
{
    SyncTransformDataToInstance();
}

void USpriteComponent::Destory()
{
    SUBMESH_DATA* mesh = GetUiOwner()->GetSceneNode().GetAssetsPool()->
        GetSubMeshIfExisted(mMeshesName);
    if (mesh) { mesh->mInstanceMap.erase(GetCompName()); }
}

bool USpriteComponent::CreateSpriteMesh(SceneNode* _scene,
    DirectX::XMFLOAT4 _offsetColor, std::string& _texName)
{
    if (!_scene) { return false; }

    RS_SUBMESH_DATA sprite = getRSDX11RootInstance()->
        getMeshHelper()->GeoGenerate()->
        CreateSpriteRect(LAYOUT_TYPE::NORMAL_TANGENT_TEX, _texName);

    mMeshesName = GetCompName();
    _scene->GetAssetsPool()->InsertNewSubMesh(mMeshesName, sprite,
        MESH_TYPE::UI_SPRITE);

    SUBMESH_DATA* spriteRect = _scene->GetAssetsPool()->
        GetSubMeshIfExisted(mMeshesName);
    if (!spriteRect) { return false; }

    RS_INSTANCE_DATA id = {};
    id.CustomizedData2 = { 0.f,0.f,1.f,1.f };
    spriteRect->mInstanceMap.insert({ mMeshesName,id });

    mOffsetColor = _offsetColor;
    mOriginTextureName = _texName;

    return true;
}

bool USpriteComponent::CreateSpriteMesh(SceneNode* _scene,
    DirectX::XMFLOAT4 _offsetColor, std::string&& _texName)
{
    if (!_scene) { return false; }

    RS_SUBMESH_DATA sprite = getRSDX11RootInstance()->
        getMeshHelper()->GeoGenerate()->
        CreateSpriteRect(LAYOUT_TYPE::NORMAL_TANGENT_TEX, _texName);

    mMeshesName = GetCompName();
    _scene->GetAssetsPool()->InsertNewSubMesh(mMeshesName, sprite,
        MESH_TYPE::UI_SPRITE);

    SUBMESH_DATA* spriteRect = _scene->GetAssetsPool()->
        GetSubMeshIfExisted(mMeshesName);
    if (!spriteRect) { return false; }

    RS_INSTANCE_DATA id = {};
    id.CustomizedData2 = { 0.f,0.f,1.f,1.f };
    spriteRect->mInstanceMap.insert({ mMeshesName,id });

    mOffsetColor = _offsetColor;
    mOriginTextureName = _texName;

    return true;
}

const DirectX::XMFLOAT4& USpriteComponent::GetOffsetColor() const
{
    return mOffsetColor;
}

void USpriteComponent::SetOffsetColor(DirectX::XMFLOAT4& _offsetColor)
{
    mOffsetColor = _offsetColor;
}

void USpriteComponent::SetOffsetColor(DirectX::XMFLOAT4&& _offsetColor)
{
    mOffsetColor = _offsetColor;
}

void USpriteComponent::SyncTransformDataToInstance()
{
    UTransformComponent* utc = GetUiOwner()->
        GetComponent<UTransformComponent>();
#ifdef _DEBUG
    assert(utc);
#endif // _DEBUG

    DirectX::XMFLOAT3 world = utc->GetPosition();
    DirectX::XMFLOAT3 angle = utc->GetRotation();
    DirectX::XMFLOAT3 scale = utc->GetScaling();

    std::string compName = GetCompName();
    auto& map = GetUiOwner()->GetSceneNode().GetAssetsPool()->
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
        DirectX::XMStoreFloat4x4(&(ins_data.WorldMatrix), mat);
        ins_data.CustomizedData1 = mOffsetColor;

        break;
    }
}

void USpriteComponent::ResetTexture()
{
    std::string compName = GetCompName();
    auto mesh = GetUiOwner()->GetSceneNode().GetAssetsPool()->
        GetSubMeshIfExisted(compName);

    mesh->mMeshData.Textures[0] = mOriginTextureName;

    for (auto& ins : mesh->mInstanceMap)
    {
        ins.second.CustomizedData2 = { 0.f,0.f,1.f,1.f };
        break;
    }

    auto uamc = GetUiOwner()->GetComponent<UAnimateComponent>();
    if (uamc) { uamc->ClearCurrentAnimate(); }
}
