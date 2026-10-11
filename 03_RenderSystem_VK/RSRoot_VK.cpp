//---------------------------------------------------------------
// File: RSRoot_VK.cpp
// Proj: RenderSystem_VK
// Info: 保存并提供此RenderSystem相关的重要内容引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSRoot_VK.h"

#include "RSCamerasContainer.h"
#include "RSDevices.h"
#include "RSDrawCallsPool.h"
#include "RSLightsContainer.h"
#include "RSMeshHelper.h"
#include "RSParticlesContainer.h"
#include "RSPipelinesManager.h"
#include "RSResourceManager.h"
#include "RSShaderCompile.h"
#include "RSStaticResources.h"

#include <cassert>

static RSRoot_VK *G_RSRoot_VK_Instance = nullptr;

RSRoot_VK *getRSVKRootInstance() {
  if (!G_RSRoot_VK_Instance) {
    assert(false && "rs_root_dx11_hasnt_been_built");
  }
  return G_RSRoot_VK_Instance;
}

RSRoot_VK::RSRoot_VK()
    : DevicesPtr(nullptr), PipelinesManagerPtr(nullptr),
      DrawCallsPoolPtr(nullptr), ResourceManagerPtr(nullptr),
      StaticResourcesPtr(nullptr), CamerasContainerPtr(nullptr),
      LightsContainerPtr(nullptr), ParticlesContainerPtr(nullptr),
      MeshHelperPtr(nullptr) {}

RSRoot_VK::~RSRoot_VK() {}

bool RSRoot_VK::startUp(SDL_Window *WndHandle) {
  DevicesPtr = new RSDevices();
  if (!DevicesPtr->startUp(this, WndHandle)) {
    return false;
  }

  PipelinesManagerPtr = new RSPipelinesManager();
  if (!PipelinesManagerPtr->startUp(this)) {
    return false;
  }

  DrawCallsPoolPtr = new RSDrawCallsPool();
  if (!DrawCallsPoolPtr->startUp(this)) {
    return false;
  }

  ResourceManagerPtr = new RSResourceManager();
  if (!ResourceManagerPtr->startUp(this)) {
    return false;
  }

  StaticResourcesPtr = new RSStaticResources();
  if (!StaticResourcesPtr->startUp(this)) {
    return false;
  }

  LightsContainerPtr = new RSLightsContainer();
  if (!LightsContainerPtr->startUp(this)) {
    return false;
  }

  CamerasContainerPtr = new RSCamerasContainer();
  if (!CamerasContainerPtr->startUp(this)) {
    return false;
  }

  ParticlesContainerPtr = new RSParticlesContainer();
  if (!ParticlesContainerPtr->startUp(this)) {
    return false;
  }

  MeshHelperPtr = new RSMeshHelper();
  if (!MeshHelperPtr->startUp(this, ResourceManagerPtr)) {
    return false;
  }

  if (G_RSRoot_VK_Instance) {
    assert(false && "rs_root_dx11_should_be_singleton");
    return false;
  }
  G_RSRoot_VK_Instance = this;

  return true;
}

void RSRoot_VK::cleanAndStop() {
  // command buffers and resources below may still be used by the gpu
  if (DevicesPtr) {
    DevicesPtr->waitIdle();
  }

  if (PipelinesManagerPtr) {
    PipelinesManagerPtr->cleanAndStop();
    delete PipelinesManagerPtr;
    PipelinesManagerPtr = nullptr;
  }

  if (MeshHelperPtr) {
    MeshHelperPtr->cleanAndStop();
    delete MeshHelperPtr;
    MeshHelperPtr = nullptr;
  }

  if (CamerasContainerPtr) {
    CamerasContainerPtr->cleanAndStop();
    delete CamerasContainerPtr;
    CamerasContainerPtr = nullptr;
  }

  if (LightsContainerPtr) {
    LightsContainerPtr->cleanAndStop();
    delete LightsContainerPtr;
    LightsContainerPtr = nullptr;
  }

  if (ParticlesContainerPtr) {
    ParticlesContainerPtr->cleanAndStop();
    delete ParticlesContainerPtr;
    ParticlesContainerPtr = nullptr;
  }

  if (StaticResourcesPtr) {
    StaticResourcesPtr->cleanAndStop();
    delete StaticResourcesPtr;
    StaticResourcesPtr = nullptr;
  }

  if (ResourceManagerPtr) {
    ResourceManagerPtr->cleanAndStop();
    delete ResourceManagerPtr;
    ResourceManagerPtr = nullptr;
  }

  if (DrawCallsPoolPtr) {
    DrawCallsPoolPtr->cleanAndStop();
    delete DrawCallsPoolPtr;
    DrawCallsPoolPtr = nullptr;
  }

  if (DevicesPtr) {
    DevicesPtr->cleanAndStop();
    delete DevicesPtr;
    DevicesPtr = nullptr;
  }

  rs_tool::finalizeShaderCompiler();

  if (G_RSRoot_VK_Instance == this) {
    G_RSRoot_VK_Instance = nullptr;
  }
}

RSDevices *RSRoot_VK::getDevices() const { return DevicesPtr; }

RSPipelinesManager *RSRoot_VK::getPipelinesManager() const {
  return PipelinesManagerPtr;
}

RSDrawCallsPool *RSRoot_VK::getDrawCallsPool() const {
  return DrawCallsPoolPtr;
}

RSResourceManager *RSRoot_VK::getResourceManager() const {
  return ResourceManagerPtr;
}

RSStaticResources *RSRoot_VK::getStaticResources() const {
  return StaticResourcesPtr;
}

RSCamerasContainer *RSRoot_VK::getCamerasContainer() const {
  return CamerasContainerPtr;
}

RSLightsContainer *RSRoot_VK::getLightsContainer() const {
  return LightsContainerPtr;
}

RSParticlesContainer *RSRoot_VK::getParticlesContainer() const {
  return ParticlesContainerPtr;
}

RSMeshHelper *RSRoot_VK::getMeshHelper() const { return MeshHelperPtr; }
