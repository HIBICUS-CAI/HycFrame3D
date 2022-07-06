#include "USpriteComponent.h"

#include "AssetsPool.h"
#include "SceneNode.h"
#include "UAnimateComponent.h"
#include "UTransformComponent.h"
#include "UiObject.h"

#include <RSMeshHelper.h>
#include <RSRoot_DX11.h>

USpriteComponent::USpriteComponent(const std::string &CompName,
                                   UiObject *UiOwner)
    : UiComponent(CompName, UiOwner), MeshesName(""), OriginTextureName(""),
      OffsetColor({1.f, 1.f, 1.f, 1.f}) {}

USpriteComponent::~USpriteComponent() {}

bool
USpriteComponent::init() {
  if (MeshesName == "") {
    P_LOG(LOG_WARNING, "this sprite hasnt been built : %s\n",
          getCompName().c_str());
    return false;
  }

  return true;
}

void
USpriteComponent::update(Timer &Timer) {
  syncTransformDataToInstance();
}

void
USpriteComponent::destory() {
  SUBMESH_DATA *MeshPtr =
      getUiOwner()->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
          MeshesName);
  if (MeshPtr) {
    MeshPtr->InstanceMap.erase(getCompName());
  }
}

bool
USpriteComponent::createSpriteMesh(SceneNode *Scene,
                                   const DirectX::XMFLOAT4 &OffsetColorSpr,
                                   const std::string &TexName) {
  if (!Scene) {
    return false;
  }

  RS_SUBMESH_DATA Sprite =
      getRSDX11RootInstance()
          ->getMeshHelper()
          ->getGeoGenerator()
          ->createSpriteRect(LAYOUT_TYPE::NORMAL_TANGENT_TEX, TexName);

  MeshesName = getCompName();
  Scene->getAssetsPool()->insertNewSubMesh(MeshesName, Sprite,
                                           MESH_TYPE::UI_SPRITE);

  SUBMESH_DATA *SpriteRect =
      Scene->getAssetsPool()->getSubMeshIfExisted(MeshesName);
  if (!SpriteRect) {
    return false;
  }

  RS_INSTANCE_DATA InsData = {};
  InsData.CustomizedData2 = {0.f, 0.f, 1.f, 1.f};
  SpriteRect->InstanceMap.insert({MeshesName, InsData});

  OffsetColor = OffsetColorSpr;
  OriginTextureName = TexName;

  return true;
}

const DirectX::XMFLOAT4 &
USpriteComponent::getOffsetColor() const {
  return OffsetColor;
}

void
USpriteComponent::setOffsetColor(const DirectX::XMFLOAT4 &OffsetColorSpr) {
  OffsetColor = OffsetColorSpr;
}

void
USpriteComponent::syncTransformDataToInstance() {
  UTransformComponent *Utc = getUiOwner()->getComponent<UTransformComponent>();
#ifdef _DEBUG
  assert(Utc);
#endif // _DEBUG

  DirectX::XMFLOAT3 World = Utc->getPosition();
  DirectX::XMFLOAT3 Angle = Utc->getRotation();
  DirectX::XMFLOAT3 Scale = Utc->getScaling();

  auto &Map = getUiOwner()
                  ->getSceneNode()
                  .getAssetsPool()
                  ->getSubMeshIfExisted(getCompName())
                  ->InstanceMap;

  for (auto &Ins : Map) {
    auto &InsData = Ins.second;

    DirectX::XMMATRIX Mat = {};
    Mat = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z),
        DirectX::XMMatrixRotationRollPitchYaw(Angle.x, Angle.y, Angle.z));
    Mat = DirectX::XMMatrixMultiply(
        Mat, DirectX::XMMatrixTranslation(World.x, World.y, World.z));
    DirectX::XMStoreFloat4x4(&(InsData.WorldMatrix), Mat);
    InsData.CustomizedData1 = OffsetColor;

    break;
  }
}

void
USpriteComponent::resetTexture() {
  auto MeshPtr =
      getUiOwner()->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
          getCompName());

  MeshPtr->MeshData.Textures[0] = OriginTextureName;

  for (auto &Ins : MeshPtr->InstanceMap) {
    Ins.second.CustomizedData2 = {0.f, 0.f, 1.f, 1.f};
    break;
  }

  auto Uamc = getUiOwner()->getComponent<UAnimateComponent>();
  if (Uamc) {
    Uamc->clearCurrentAnimate();
  }
}
