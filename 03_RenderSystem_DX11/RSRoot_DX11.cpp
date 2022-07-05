//---------------------------------------------------------------
// File: RSRoot_DX11.cpp
// Proj: RenderSystem_DX11
// Info: 保存并提供此RenderSystem相关的重要内容引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#include "RSRoot_DX11.h"

#include "RSCamerasContainer.h"
#include "RSDevices.h"
#include "RSDrawCallsPool.h"
#include "RSLightsContainer.h"
#include "RSMeshHelper.h"
#include "RSParticlesContainer.h"
#include "RSPipelinesManager.h"
#include "RSResourceManager.h"
#include "RSStaticResources.h"

#include <cassert>

static RSRoot_DX11 *G_RSRoot_DX11_Instance = nullptr;

RSRoot_DX11 *
getRSDX11RootInstance() {
  if (!G_RSRoot_DX11_Instance) {
    assert(false && "rs_root_dx11_hasnt_been_built");
  }
  return G_RSRoot_DX11_Instance;
}

RSRoot_DX11::RSRoot_DX11()
    : DevicesPtr(nullptr), PipelinesManagerPtr(nullptr),
      DrawCallsPoolPtr(nullptr), ResourceManagerPtr(nullptr),
      StaticResourcesPtr(nullptr), CamerasContainerPtr(nullptr),
      LightsContainerPtr(nullptr), ParticlesContainerPtr(nullptr),
      MeshHelperPtr(nullptr) {}

RSRoot_DX11::~RSRoot_DX11() {}

bool
RSRoot_DX11::startUp(HWND WndHandle) {
  DevicesPtr = new RSDevices();
  if (!DevicesPtr->StartUp(this, WndHandle)) {
    return false;
  }

  PipelinesManagerPtr = new RSPipelinesManager();
  if (!PipelinesManagerPtr->StartUp(this)) {
    return false;
  }

  DrawCallsPoolPtr = new RSDrawCallsPool();
  if (!DrawCallsPoolPtr->StartUp(this)) {
    return false;
  }

  ResourceManagerPtr = new RSResourceManager();
  if (!ResourceManagerPtr->StartUp(this)) {
    return false;
  }

  StaticResourcesPtr = new RSStaticResources();
  if (!StaticResourcesPtr->StartUp(this)) {
    return false;
  }

  LightsContainerPtr = new RSLightsContainer();
  if (!LightsContainerPtr->StartUp(this)) {
    return false;
  }

  CamerasContainerPtr = new RSCamerasContainer();
  if (!CamerasContainerPtr->StartUp(this)) {
    return false;
  }

  ParticlesContainerPtr = new RSParticlesContainer();
  if (!ParticlesContainerPtr->StartUp(this)) {
    return false;
  }

  MeshHelperPtr = new RSMeshHelper();
  if (!MeshHelperPtr->StartUp(this, ResourceManagerPtr)) {
    return false;
  }

  if (G_RSRoot_DX11_Instance) {
    assert(false && "rs_root_dx11_should_be_singleton");
    return false;
  }
  G_RSRoot_DX11_Instance = this;

  return true;
}

void
RSRoot_DX11::cleanAndStop() {
  if (PipelinesManagerPtr) {
    PipelinesManagerPtr->CleanAndStop();
    delete PipelinesManagerPtr;
    PipelinesManagerPtr = nullptr;
  }

  if (MeshHelperPtr) {
    MeshHelperPtr->CleanAndStop();
    delete MeshHelperPtr;
    MeshHelperPtr = nullptr;
  }

  if (CamerasContainerPtr) {
    CamerasContainerPtr->CleanAndStop();
    delete CamerasContainerPtr;
    CamerasContainerPtr = nullptr;
  }

  if (LightsContainerPtr) {
    LightsContainerPtr->CleanAndStop();
    delete LightsContainerPtr;
    LightsContainerPtr = nullptr;
  }

  if (ParticlesContainerPtr) {
    ParticlesContainerPtr->CleanAndStop();
    delete ParticlesContainerPtr;
    ParticlesContainerPtr = nullptr;
  }

  if (StaticResourcesPtr) {
    StaticResourcesPtr->CleanAndStop();
    delete StaticResourcesPtr;
    StaticResourcesPtr = nullptr;
  }

  if (ResourceManagerPtr) {
    ResourceManagerPtr->CleanAndStop();
    delete ResourceManagerPtr;
    ResourceManagerPtr = nullptr;
  }

  if (DrawCallsPoolPtr) {
    DrawCallsPoolPtr->CleanAndStop();
    delete DrawCallsPoolPtr;
    DrawCallsPoolPtr = nullptr;
  }

  if (DevicesPtr) {
    DevicesPtr->CleanAndStop();
    delete DevicesPtr;
    DevicesPtr = nullptr;
  }
}

RSDevices *
RSRoot_DX11::getDevices() const {
  return DevicesPtr;
}

RSPipelinesManager *
RSRoot_DX11::getPipelinesManager() const {
  return PipelinesManagerPtr;
}

RSDrawCallsPool *
RSRoot_DX11::getDrawCallsPool() const {
  return DrawCallsPoolPtr;
}

RSResourceManager *
RSRoot_DX11::getResourceManager() const {
  return ResourceManagerPtr;
}

RSStaticResources *
RSRoot_DX11::getStaticResources() const {
  return StaticResourcesPtr;
}

RSCamerasContainer *
RSRoot_DX11::getCamerasContainer() const {
  return CamerasContainerPtr;
}

RSLightsContainer *
RSRoot_DX11::getLightsContainer() const {
  return LightsContainerPtr;
}

RSParticlesContainer *
RSRoot_DX11::getParticlesContainer() const {
  return ParticlesContainerPtr;
}

RSMeshHelper *
RSRoot_DX11::getMeshHelper() const {
  return MeshHelperPtr;
}
