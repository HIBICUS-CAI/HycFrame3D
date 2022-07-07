#include "RenderSystem.h"

#include "AssetsPool.h"
#include "BasicRSPipeline.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SystemExecutive.h"
#include "UButtonComponent.h"

#include <RSCamera.h>
#include <RSCamerasContainer.h>
#include <RSDevices.h>
#include <RSDrawCallsPool.h>
#include <RSLightsContainer.h>
#include <RSParticlesContainer.h>
#include <RSPipelinesManager.h>
#include <RSResourceManager.h>
#include <RSRoot_DX11.h>
#include <WM_Interface.h>

RenderSystem::RenderSystem(SystemExecutive *SysExecutive)
    : System("render-system", SysExecutive), RenderSystemRoot(nullptr),
      AssetsPool(nullptr), IblEnvTex(nullptr), IblDiffTex(nullptr),
      IblSpecTex(nullptr) {}

RenderSystem::~RenderSystem() {}

bool RenderSystem::init() {
  if (!RenderSystemRoot) {
    RenderSystemRoot = new RSRoot_DX11();
    if (!RenderSystemRoot->startUp(window::getWindowPtr()->getWndHandle())) {
      return false;
    }

    CAM_INFO ci = {};
    ci.Type = LENS_TYPE::PERSPECTIVE;
    ci.Position = {0.f, 0.f, 0.f};
    ci.LookAtVector = {0.f, 0.f, 1.f};
    ci.UpVector = {0.f, 1.f, 0.f};
    ci.NearFarZ = {1.f, 800.f};
    ci.PerspFovYRatio = {DirectX::XM_PIDIV4, 16.f / 9.f};
    ci.OrthoWidthHeight = {12.8f, 7.2f};
    RenderSystemRoot->getCamerasContainer()->createRSCamera("temp-cam", &ci);

    ci = {};
    ci.Type = LENS_TYPE::ORTHOGRAPHIC;
    ci.Position = {0.f, 0.f, 0.f};
    ci.LookAtVector = {0.f, 0.f, 1.f};
    ci.UpVector = {0.f, 1.f, 0.f};
    ci.NearFarZ = {1.f, 100.f};
    ci.PerspFovYRatio = {DirectX::XM_PIDIV4, 16.f / 9.f};
    ci.OrthoWidthHeight = {1280.f, 720.f};
    RenderSystemRoot->getCamerasContainer()->createRSCamera("temp-ui-cam", &ci);

    if (!createBasicPipeline()) {
      return false;
    }
  }

  AssetsPool = nullptr;
  AssetsPool = getSystemExecutive()
                   ->getSceneManager()
                   ->getCurrentSceneNode()
                   ->getAssetsPool();
  if (!AssetsPool) {
    return false;
  }

  IblEnvTex = getSystemExecutive()
                  ->getSceneManager()
                  ->getCurrentSceneNode()
                  ->getIBLEnvironment();
  IblDiffTex = getSystemExecutive()
                   ->getSceneManager()
                   ->getCurrentSceneNode()
                   ->getIBLDiffuse();
  IblSpecTex = getSystemExecutive()
                   ->getSceneManager()
                   ->getCurrentSceneNode()
                   ->getIBLSpecular();

  getRSDX11RootInstance()->getParticlesContainer()->resetRSParticleSystem();

  return true;
}

void RenderSystem::run(const Timer &Timer) {
  RenderSystemRoot = getRSDX11RootInstance();
#ifdef _DEBUG
  assert(RenderSystemRoot);
#endif // _DEBUG

  static RS_DRAWCALL_DATA DrawCall = {};
  auto &MeshPool = AssetsPool->SubMeshPool;
  RS_DRAWCALL_DATA BtnSelectTex = {};
  bool HasBtnSelectFlag = false;
  for (auto &Mesh : MeshPool) {
    Mesh.second.InstanceVector.clear();
    for (auto &Instance : Mesh.second.InstanceMap) {
      Mesh.second.InstanceVector.emplace_back(Instance.second);
    }
    if (!Mesh.second.InstanceVector.size()) {
      continue;
    }

    Mesh.second.BonesVector.clear();
    for (auto &Bone : Mesh.second.BonesMap) {
      Mesh.second.BonesVector.emplace_back(Bone.second);
    }

    auto DrawCallPool = RenderSystemRoot->getDrawCallsPool();
    DRAWCALL_TYPE DType = DRAWCALL_TYPE::MAX;
    MESH_TYPE MType = Mesh.second.MeshType;
    switch (MType) {
    case MESH_TYPE::OPACITY:
      DType = DRAWCALL_TYPE::OPACITY;
      break;
    case MESH_TYPE::TRANSPARENCY:
      DType = DRAWCALL_TYPE::TRANSPARENCY;
      break;
    case MESH_TYPE::LIGHT:
      DType = DRAWCALL_TYPE::LIGHT;
      break;
    case MESH_TYPE::UI_SPRITE:
      DType = DRAWCALL_TYPE::UI_SPRITE;
      break;
    default:
      break;
    }

    DrawCall = {};
    DrawCall.MeshData.InputLayout = Mesh.second.MeshData.InputLayout;
    DrawCall.MeshData.TopologyType = Mesh.second.MeshData.TopologyType;
    DrawCall.MeshData.VertexBuffer = Mesh.second.MeshData.VertexBuffer;
    DrawCall.MeshData.IndexBuffer = Mesh.second.MeshData.IndexBuffer;
    DrawCall.MeshData.IndexSize = Mesh.second.MeshData.IndexSize;
    DrawCall.InstanceData.DataArrayPtr = &(Mesh.second.InstanceVector);
    DrawCall.InstanceData.BonesArrayPtr = &(Mesh.second.BonesVector);
    auto TexSize = Mesh.second.MeshData.Textures.size();
    auto &TexArray = Mesh.second.MeshData.Textures;
    for (size_t I = 0; I < TexSize; I++) {
      if (TexArray[I] != "") {
        DrawCall.TextureData[I].EnabledFlag = true;
        DrawCall.TextureData[I].Srv =
            RenderSystemRoot->getResourceManager()->getMeshSrv(TexArray[I]);
      }
    }

    DrawCallPool->addDrawCallToPipe(DType, DrawCall);
    if (Mesh.first == SELECTED_BTN_SPRITE_NAME) {
      HasBtnSelectFlag = true;
      BtnSelectTex = DrawCall;
    }
  }
  if (HasBtnSelectFlag) {
    RenderSystemRoot->getDrawCallsPool()->addDrawCallToPipe(
        DRAWCALL_TYPE::UI_SPRITE, BtnSelectTex);
  }
  RenderSystemRoot->getLightsContainer()->uploadLightBloomDrawCall();

  RenderSystemRoot->getLightsContainer()->forceCurrentAmbientLight(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getCurrentAmbientFactor());

  setPipelineIBLTextures(IblEnvTex, IblDiffTex, IblSpecTex);
  setPipelineDeltaTime(Timer.floatDeltaTime());

  RenderSystemRoot->getPipelinesManager()->changeToNextPipeline();
  RenderSystemRoot->getPipelinesManager()->execuateCurrentPipeline();
  RenderSystemRoot->getDevices()->presentSwapChain();
  RenderSystemRoot->getDrawCallsPool()->clearAllDrawCalls();
}

void RenderSystem::destory() {
  RenderSystemRoot->cleanAndStop();
  delete RenderSystemRoot;
}
