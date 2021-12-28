#include "USpriteComponent.h"
#include "UiObject.h"
#include "AssetsPool.h"
#include "SceneNode.h"
#include "UTransformComponent.h"
#include "RSMeshHelper.h"
#include "RSRoot_DX11.h"

USpriteComponent::USpriteComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mMeshesName(""), mOffsetColor({ 1.f,1.f,1.f,1.f })
{

}

USpriteComponent::USpriteComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mMeshesName(""), mOffsetColor({ 1.f,1.f,1.f,1.f })
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
    MESH_DATA* mesh = GetUiOwner()->GetSceneNode().GetAssetsPool()->
        GetMeshIfExisted(mMeshesName);
    if (mesh) { mesh->mInstanceMap.erase(GetCompName()); }
}

bool USpriteComponent::CreateSpriteMesh(SceneNode* _scene,
    DirectX::XMFLOAT4 _offsetColor, std::string& _texName)
{
    if (!_scene) { return false; }

    RS_SUBMESH_DATA sprite = GetRSRoot_DX11_Singleton()->
        MeshHelper()->GeoGenerate()->
        CreateSpriteRect(LAYOUT_TYPE::NORMAL_TANGENT_TEX, _texName);

    mMeshesName = GetCompName();
    _scene->GetAssetsPool()->InsertNewMesh(mMeshesName, sprite,
        MESH_TYPE::UI_SPRITE);

    MESH_DATA* spriteRect = _scene->GetAssetsPool()->
        GetMeshIfExisted(mMeshesName);
    if (!spriteRect) { return false; }

    RS_INSTANCE_DATA id = {};
    id.mCustomizedData2 = { 0.f,0.f,1.f,1.f };
    spriteRect->mInstanceMap.insert({ mMeshesName,id });

    return true;
}

bool USpriteComponent::CreateSpriteMesh(SceneNode* _scene,
    DirectX::XMFLOAT4 _offsetColor, std::string&& _texName)
{
    if (!_scene) { return false; }

    RS_SUBMESH_DATA sprite = GetRSRoot_DX11_Singleton()->
        MeshHelper()->GeoGenerate()->
        CreateSpriteRect(LAYOUT_TYPE::NORMAL_TANGENT_TEX, _texName);

    mMeshesName = GetCompName();
    _scene->GetAssetsPool()->InsertNewMesh(mMeshesName, sprite,
        MESH_TYPE::UI_SPRITE);

    MESH_DATA* spriteRect = _scene->GetAssetsPool()->
        GetMeshIfExisted(mMeshesName);
    if (!spriteRect) { return false; }

    RS_INSTANCE_DATA id = {};
    id.mCustomizedData2 = { 0.f,0.f,1.f,1.f };
    spriteRect->mInstanceMap.insert({ mMeshesName,id });

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
        GetUComponent<UTransformComponent>(COMP_TYPE::U_TRANSFORM);
#ifdef _DEBUG
    assert(utc);
#endif // _DEBUG

    DirectX::XMFLOAT3 world = utc->GetPosition();
    DirectX::XMFLOAT3 angle = utc->GetRotation();
    DirectX::XMFLOAT3 scale = utc->GetScaling();

    std::string compName = GetCompName();
    auto& map = GetUiOwner()->GetSceneNode().GetAssetsPool()->
        GetMeshIfExisted(compName)->mInstanceMap;

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
        ins_data.mCustomizedData1 = mOffsetColor;

        break;
    }
}
