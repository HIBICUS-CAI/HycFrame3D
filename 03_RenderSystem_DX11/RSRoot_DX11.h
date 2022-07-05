//---------------------------------------------------------------
// File: RSRoot_DX11.h
// Proj: RenderSystem_DX11
// Info: 保存并提供此RenderSystem相关的重要内容引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

class RSRoot_DX11 *
getRSDX11RootInstance();

class RSRoot_DX11 {
private:
  class RSDevices *DevicesPtr;
  class RSPipelinesManager *PipelinesManagerPtr;
  class RSDrawCallsPool *DrawCallsPoolPtr;
  class RSResourceManager *ResourceManagerPtr;
  class RSStaticResources *StaticResourcesPtr;
  class RSCamerasContainer *CamerasContainerPtr;
  class RSLightsContainer *LightsContainerPtr;
  class RSParticlesContainer *ParticlesContainerPtr;
  class RSMeshHelper *MeshHelperPtr;

public:
  RSRoot_DX11();
  ~RSRoot_DX11();

  bool
  startUp(HWND WndHandle);

  void
  cleanAndStop();

  class RSDevices *
  getDevices() const;

  class RSPipelinesManager *
  getPipelinesManager() const;

  class RSDrawCallsPool *
  getDrawCallsPool() const;

  class RSResourceManager *
  getResourceManager() const;

  class RSStaticResources *
  getStaticResources() const;

  class RSCamerasContainer *
  getCamerasContainer() const;

  class RSLightsContainer *
  getLightsContainer() const;

  class RSParticlesContainer *
  getParticlesContainer() const;

  class RSMeshHelper *
  getMeshHelper() const;
};
