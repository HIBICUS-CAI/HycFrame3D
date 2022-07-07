//---------------------------------------------------------------
// File: RSCamerasContainer.cpp
// Proj: RenderSystem_DX11
// Info: 对所有的摄像机进行引用及简单处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSCamerasContainer.h"

#include "RSCamera.h"
#include "RSRoot_DX11.h"

#define LOCK EnterCriticalSection(&(this->DataLock))
#define UNLOCK LeaveCriticalSection(&(this->DataLock))

RSCamerasContainer::RSCamerasContainer()
    : RenderSystemRoot(nullptr), DataLock({}), CameraMap({}) {}

RSCamerasContainer::~RSCamerasContainer() {}

bool RSCamerasContainer::startUp(RSRoot_DX11 *RootPtr) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;
  InitializeCriticalSection(&DataLock);

  return true;
}

void RSCamerasContainer::cleanAndStop() {
  for (auto &Cam : CameraMap) {
    delete Cam.second;
  }
  CameraMap.clear();
  DeleteCriticalSection(&DataLock);
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
