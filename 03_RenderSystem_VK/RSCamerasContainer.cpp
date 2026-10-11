//---------------------------------------------------------------
// File: RSCamerasContainer.cpp
// Proj: RenderSystem_VK
// Info: 对所有的摄像机进行引用及简单处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSCamerasContainer.h"

#include "RSCamera.h"
#include "RSRoot_VK.h"

#define LOCK DataLock.lock()
#define UNLOCK DataLock.unlock()

RSCamerasContainer::RSCamerasContainer()
    : RenderSystemRoot(nullptr), CameraMap({}) {}

RSCamerasContainer::~RSCamerasContainer() {}

bool RSCamerasContainer::startUp(RSRoot_VK *RootPtr) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;

  return true;
}

void RSCamerasContainer::cleanAndStop() {
  for (auto &Cam : CameraMap) {
    delete Cam.second;
  }
  CameraMap.clear();
}

RSCamera *RSCamerasContainer::createRSCamera(const std::string &Name,
                                             const CAM_INFO *Info) {
  if (!Info) {
    return nullptr;
  }

  LOCK;
  if (CameraMap.find(Name) == CameraMap.end()) {
    RSCamera *Cam = new RSCamera(Info);
    CameraMap.insert({Name, Cam});
  }
  auto Cam = CameraMap[Name];
  UNLOCK;

  return Cam;
}

RSCamera *RSCamerasContainer::getRSCamera(const std::string &Name) {
  LOCK;
  auto Found = CameraMap.find(Name);
  if (Found != CameraMap.end()) {
    auto Cam = Found->second;
    UNLOCK;
    return Cam;
  } else {
    UNLOCK;
    return nullptr;
  }
}

RS_CAM_INFO *RSCamerasContainer::getRSCameraInfo(const std::string &Name) {
  LOCK;
  auto Found = CameraMap.find(Name);
  if (Found != CameraMap.end()) {
    auto Cam = Found->second;
    UNLOCK;
    return Cam->getRSCameraInfo();
  } else {
    UNLOCK;
    return nullptr;
  }
}

void RSCamerasContainer::deleteRSCamera(const std::string &Name) {
  LOCK;
  auto Found = CameraMap.find(Name);
  if (Found != CameraMap.end()) {
    delete Found->second;
    CameraMap.erase(Found);
  }
  UNLOCK;
}
