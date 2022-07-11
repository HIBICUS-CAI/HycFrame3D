#include "ASpriteComponent.h"

#include "ATransformComponent.h"
#include "ActorObject.h"
#include "AssetsPool.h"
#include "SceneNode.h"

#include <RSMeshHelper.h>
#include <RSRoot_DX11.h>

ASpriteComponent::ASpriteComponent(const std::string &CompName,
                                   ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), GeoPointName(""), TextureName(""),
      EnabledBillboardFlag(false), Size({10.f, 10.f}),
      TexCoord({0.f, 0.f, 1.f, 1.f}), EnabledAnimationFlag(false),
      Stride({0.f, 0.f}), MaxCut(0), CurrentAnimateCut(0), RepeatFlag(false),
      SwitchTime(0.f), TimeCounter(0.f) {}

ASpriteComponent::~ASpriteComponent() {}

bool ASpriteComponent::init() { return true; }

void ASpriteComponent::update(const Timer &Timer) {
  if (EnabledAnimationFlag && TimeCounter > SwitchTime) {
    TimeCounter = 0.f;
    ++CurrentAnimateCut;
    if (CurrentAnimateCut >= MaxCut) {
      if (RepeatFlag) {
        CurrentAnimateCut = 0;
      } else {
        --CurrentAnimateCut;
      }
    }
    float StartX = 0.f;
    float StartY = 0.f;
    unsigned int MaxX = (int)(1.f / Stride.x);
    MaxX = (((1.f / Stride.x) - MaxX) > 0.5f) ? (MaxX + 1) : MaxX;
    StartX = (float)(CurrentAnimateCut % MaxX) * Stride.x;
    StartY = (float)(CurrentAnimateCut / MaxX) * Stride.y;
    TexCoord = {StartX, StartY, Stride.x, Stride.y};
  }
  TimeCounter += Timer.floatDeltaTime() / 1000.f;

  syncTransformDataToInstance();
}

void ASpriteComponent::destory() {
  SUBMESH_DATA *MeshPtr =
      getActorOwner()->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
          GeoPointName);
  if (MeshPtr) {
    MeshPtr->InstanceMap.erase(getCompName());
  }
}

bool ASpriteComponent::createGeoPointWithTexture(SceneNode *Scene,
                                                 const std::string &TexName) {
  if (!Scene) {
    return false;
  }

  RS_SUBMESH_DATA Point =
      getRSDX11RootInstance()
          ->getMeshHelper()
          ->getGeoGenerator()
          ->createPointWithTexture(LAYOUT_TYPE::NORMAL_TANGENT_TEX,
                                   TexName.c_str());
  GeoPointName = getCompName();
  TextureName = TexName;
  Scene->getAssetsPool()->insertNewSubMesh(GeoPointName, Point,
                                           MESH_TYPE::TRANSPARENCY);

  SUBMESH_DATA *SpriteRect =
      Scene->getAssetsPool()->getSubMeshIfExisted(GeoPointName);
  if (!SpriteRect) {
    return false;
  }

  RS_INSTANCE_DATA InsData = {};
  InsData.CustomizedData1 = {Size.x, Size.y, (EnabledBillboardFlag ? 1.f : 0.f),
                             0.f};
  InsData.CustomizedData2 = TexCoord;
  SpriteRect->InstanceMap.insert({GeoPointName, InsData});

  return true;
}

void ASpriteComponent::setSpriteProperty(
    const DirectX::XMFLOAT2 &SpriteSize,
    const DirectX::XMFLOAT4 &SpriteTexCoord,
    bool IsBillboard) {
  EnabledBillboardFlag = IsBillboard;
  Size = SpriteSize;
  TexCoord = SpriteTexCoord;
}

void ASpriteComponent::setAnimationProperty(const DirectX::XMFLOAT2 &AniStride,
                                            UINT AniMaxCut,
                                            bool AniRepeatFlg,
                                            float AniSwitchTime) {
  EnabledAnimationFlag = true;
  Stride = AniStride;
  MaxCut = AniMaxCut;
  SwitchTime = AniSwitchTime;
  RepeatFlag = AniRepeatFlg;
  CurrentAnimateCut = 0;
  TimeCounter = 0.f;
  TexCoord.z = Stride.x;
  TexCoord.w = Stride.y;
}

void ASpriteComponent::syncTransformDataToInstance() {
  ATransformComponent *Atc =
      getActorOwner()->getComponent<ATransformComponent>();
#ifdef _DEBUG
  assert(Atc);
#endif // _DEBUG

  DirectX::XMFLOAT3 World = Atc->getPosition();
  DirectX::XMFLOAT3 Angle = Atc->getRotation();
  DirectX::XMFLOAT3 Scale = Atc->getScaling();

  auto &InsMap = getActorOwner()
                     ->getSceneNode()
                     .getAssetsPool()
                     ->getSubMeshIfExisted(getCompName())
                     ->InstanceMap;

  for (auto &Ins : InsMap) {
    auto &InsData = Ins.second;

    DirectX::XMMATRIX Matrix = {};
    Matrix = DirectX::XMMatrixMultiply(
        DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z),
        DirectX::XMMatrixRotationRollPitchYaw(Angle.x, Angle.y, Angle.z));
    Matrix = DirectX::XMMatrixMultiply(
        Matrix, DirectX::XMMatrixTranslation(World.x, World.y, World.z));
    DirectX::XMStoreFloat4x4(&(InsData.WorldMatrix), Matrix);

    InsData.CustomizedData2 = TexCoord;

    break;
  }
}
