//---------------------------------------------------------------
// File: RSCamerasContainer.h
// Proj: RenderSystem_DX11
// Info: 对所有的摄像机进行引用及简单处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include <unordered_map>

class RSCamerasContainer {
private:
  class RSRoot_DX11 *RenderSystemRoot;

  CRITICAL_SECTION DataLock;

  std::unordered_map<std::string, class RSCamera *> CameraMap;

public:
  RSCamerasContainer();
  ~RSCamerasContainer();

  bool
  startUp(class RSRoot_DX11 *RootPtr);

  void
  cleanAndStop();

  class RSCamera *
  createRSCamera(const std::string &Name, const CAM_INFO *Info);

  class RSCamera *
  getRSCamera(const std::string &Name);

  RS_CAM_INFO *
  getRSCameraInfo(const std::string &Name);

  void
  deleteRSCamera(const std::string &Name);
};
