#include "ALightComponent.h"

#include "ATransformComponent.h"
#include "ActorObject.h"
#include "AssetsPool.h"
#include "SceneNode.h"

#include <RSLight.h>
#include <RSLightsContainer.h>
#include <RSMeshHelper.h>
#include <RSRoot_DX11.h>

#include <vector>

ALightComponent::ALightComponent(const std::string &CompName,
                                 ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), LightName(""), RSLightPtr(nullptr),
      CanCreateLight(false), LightInfoForInit({}), LightCamInfoForInit({}),
      IsBloom(false), IsCamera(false) {}

ALightComponent::~ALightComponent() {}

bool
ALightComponent::init() {
  if (!CanCreateLight) {
    return false;
  }

  createLight();

  if (RSLightPtr) {
    return true;
  } else {
    return false;
  }
}

void
ALightComponent::update(Timer &Timer) {
  syncDataFromTransform();
}

void
ALightComponent::destory() {
  getRSDX11RootInstance()->getLightsContainer()->deleteRSLight(LightName, true);
}

void
ALightComponent::createLight() {
  LightName = getCompName();
  RSLightPtr = getRSDX11RootInstance()->getLightsContainer()->createRSLight(
      LightName, &LightInfoForInit);
#ifdef _DEBUG
  assert(RSLightPtr);
#endif // _DEBUG

  if (IsBloom) {
    SUBMESH_DATA *MeshPtr =
        getActorOwner()->getSceneNode().GetAssetsPool()->getSubMeshIfExisted(
            BOX_BLOOM_MESH_NAME);
    if (!MeshPtr) {
      RS_SUBMESH_DATA BoxBloom =
          getRSDX11RootInstance()
              ->getMeshHelper()
              ->getGeoGenerator()
              ->createBox(1.f, 1.f, 1.f, 0, LAYOUT_TYPE::NORMAL_COLOR);
      getActorOwner()->getSceneNode().GetAssetsPool()->insertNewSubMesh(
          BOX_BLOOM_MESH_NAME, BoxBloom, MESH_TYPE::LIGHT);
      MeshPtr =
          getActorOwner()->getSceneNode().GetAssetsPool()->getSubMeshIfExisted(
              BOX_BLOOM_MESH_NAME);
    }
    RSLightPtr->setLightBloom(MeshPtr->MeshData);
  }

  if (IsCamera) {
    bool CamCreate =
        getRSDX11RootInstance()->getLightsContainer()->createLightCameraFor(
            LightName, &LightCamInfoForInit);
#ifdef _DEBUG
    assert(CamCreate);
#endif // _DEBUG
    (void)CamCreate;
  }
}

void
ALightComponent::resetLight(const LIGHT_INFO *LightInfo) {
  RSLightPtr->resetRSLight(LightInfo);
}

RSLight *
ALightComponent::getLightInfo() {
  return RSLightPtr;
}

void
ALightComponent::syncDataFromTransform() {
  ATransformComponent *Atc =
      getActorOwner()->getComponent<ATransformComponent>();
#ifdef _DEBUG
  assert(Atc);
#endif // _DEBUG

  dx::XMFLOAT3 World = Atc->getPosition();
  dx::XMFLOAT3 Angle = Atc->getRotation();
  dx::XMFLOAT3 Scale = Atc->getScaling();

  RSLightPtr->setRSLightPosition(World);

  if (IsBloom) {
    dx::XMMATRIX Matrix = {};
    Matrix = dx::XMMatrixMultiply(
        dx::XMMatrixScaling(Scale.x, Scale.y, Scale.z),
        dx::XMMatrixRotationRollPitchYaw(Angle.x, Angle.y, Angle.z));
    Matrix = dx::XMMatrixMultiply(
        Matrix, dx::XMMatrixTranslation(World.x, World.y, World.z));
    dx::XMStoreFloat4x4(RSLightPtr->getLightWorldMat(), Matrix);
  }
}

void
ALightComponent::addLight(const LIGHT_INFO &LightInfo,
                          bool SetBloom,
                          bool SetCamera,
                          const CAM_INFO &CamInfo) {
  LightInfoForInit = LightInfo;
  LightCamInfoForInit = CamInfo;
  IsBloom = SetBloom;
  IsCamera = SetCamera;
  CanCreateLight = true;
}
