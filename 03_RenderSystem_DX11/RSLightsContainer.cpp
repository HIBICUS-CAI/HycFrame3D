//---------------------------------------------------------------
// File: RSLightsContainer.cpp
// Proj: RenderSystem_DX11
// Info: 对所有的光源进行引用及简单处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSLightsContainer.h"

#include "RSCamerasContainer.h"
#include "RSLight.h"
#include "RSRoot_DX11.h"

#include <algorithm>

#define LOCK EnterCriticalSection(&DataLock)
#define UNLOCK LeaveCriticalSection(&DataLock)

RSLightsContainer::RSLightsContainer()
    : RenderSystemRoot(nullptr), LightsMap({}), AmbientLightsMap({}),
      CurrentAmbient({0.f, 0.f, 0.f, 0.f}), LightsArray({}),
      ShadowLightsArray({}), ShadowLightIndeicesArray({}), DataLock({}) {}

RSLightsContainer::~RSLightsContainer() {}

bool
RSLightsContainer::startUp(RSRoot_DX11 *RootPtr) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;
  InitializeCriticalSection(&DataLock);

  return true;
}

void
RSLightsContainer::cleanAndStop() {
  for (auto &Light : LightsMap) {
    delete Light.second;
  }
  LightsMap.clear();
  LightsArray.clear();
  ShadowLightsArray.clear();
  ShadowLightIndeicesArray.clear();
  AmbientLightsMap.clear();
  DeleteCriticalSection(&DataLock);
}

bool
LightLessCompare(RSLight *A, RSLight *B) {
  return static_cast<UINT>(A->getRSLightType()) <
         static_cast<UINT>(B->getRSLightType());
}

RSLight *
RSLightsContainer::createRSLight(const std::string &Name,
                                 const LIGHT_INFO *Info) {
  if (!Info) {
    return nullptr;
  }

  LOCK;
  if (LightsMap.find(Name) == LightsMap.end()) {
    UNLOCK;
    RSLight *Light = new RSLight(Info);
    LOCK;
    LightsMap.insert({Name, Light});
    LightsArray.emplace_back(Light);
    std::sort(LightsArray.begin(), LightsArray.end(), LightLessCompare);
    if (Info->ShadowFlag) {
      ShadowLightsArray.emplace_back(Light);
      ShadowLightIndeicesArray.emplace_back(
          static_cast<UINT>(ShadowLightsArray.size() - 1));
    }
  }
  auto Light = LightsMap[Name];
  UNLOCK;

  return Light;
}

RSLight *
RSLightsContainer::getRSLight(const std::string &Name) {
  LOCK;
  auto Found = LightsMap.find(Name);
  if (Found != LightsMap.end()) {
    auto Light = Found->second;
    UNLOCK;
    return Light;
  } else {
    UNLOCK;
    return nullptr;
  }
}

RS_LIGHT_INFO *
RSLightsContainer::getRSLightInfo(const std::string &Name) {
  LOCK;
  auto Found = LightsMap.find(Name);
  if (Found != LightsMap.end()) {
    auto Light = Found->second;
    UNLOCK;
    return Light->getRSLightInfo();
  } else {
    UNLOCK;
    return nullptr;
  }
}

void
RSLightsContainer::deleteRSLight(const std::string &Name,
                                 bool DeleteByFrameWorkFlag) {
  LOCK;
  auto Found = LightsMap.find(Name);
  if (Found != LightsMap.end()) {
    for (auto I = LightsArray.begin(), E = LightsArray.end(); I != E; I++) {
      if ((*I) == Found->second) {
        LightsArray.erase(I);
        break;
      }
    }

    for (auto I = ShadowLightIndeicesArray.begin(),
              E = ShadowLightIndeicesArray.end();
         I != E; I++) {
      if (ShadowLightsArray[(*I)] == Found->second) {
        for (auto &index : ShadowLightIndeicesArray) {
          if (index > (*I)) {
            --index;
          }
        }
        ShadowLightIndeicesArray.erase(I);
        std::string camName = Name + "-light-Cam";
        RenderSystemRoot->getCamerasContainer()->deleteRSCamera(camName);
        break;
      }
    }
    for (auto I = ShadowLightsArray.begin(), E = ShadowLightsArray.end();
         I != E; I++) {
      if ((*I) == Found->second) {
        ShadowLightsArray.erase(I);
        break;
      }
    }

    Found->second->releaseLightBloom(DeleteByFrameWorkFlag);
    delete Found->second;
    LightsMap.erase(Found);
  }
  UNLOCK;
}

bool
RSLightsContainer::createLightCameraFor(const std::string &Name,
                                        const CAM_INFO *Info) {
  LOCK;
  auto Found = LightsMap.find(Name);
  if (Found != LightsMap.end()) {
    auto Light = Found->second;
    UNLOCK;
    auto Cam = Light->createLightCamera(
        Name, Info, RenderSystemRoot->getCamerasContainer());
    if (Cam) {
      return true;
    } else {
      return false;
    }
  } else {
    UNLOCK;
    return false;
  }
}

std::vector<RSLight *> *
RSLightsContainer::getLightsArray() {
  return &LightsArray;
}

std::vector<RSLight *> *
RSLightsContainer::getShadowLightsArray() {
  return &ShadowLightsArray;
}

std::vector<INT> *
RSLightsContainer::getShadowLightIndeicesArray() {
  return &ShadowLightIndeicesArray;
}

void
RSLightsContainer::insertAmbientLight(const std::string &Name,
                                      const dx::XMFLOAT4 &Ambient) {
  LOCK;
  auto Found = AmbientLightsMap.find(Name);
  if (Found == AmbientLightsMap.end()) {
    AmbientLightsMap.insert({Name, Ambient});
  }
  UNLOCK;
}

void
RSLightsContainer::eraseAmbientLight(const std::string &Name) {
  LOCK;
  auto Found = AmbientLightsMap.find(Name);
  if (Found != AmbientLightsMap.end()) {
    AmbientLightsMap.erase(Name);
  }
  UNLOCK;
}

const dx::XMFLOAT4 &
RSLightsContainer::getAmbientLight(const std::string &Name) {
  LOCK;
  auto Found = AmbientLightsMap.find(Name);
  static dx::XMFLOAT4 ambient = {};
  if (Found != AmbientLightsMap.end()) {
    dx::XMFLOAT4 &refAmb = AmbientLightsMap[Name];
    UNLOCK;
    return refAmb;
  }
  UNLOCK;
  return ambient;
}

void
RSLightsContainer::setCurrentAmbientLight(const std::string &Name) {
  CurrentAmbient = getAmbientLight(Name);
}

void
RSLightsContainer::forceCurrentAmbientLight(const dx::XMFLOAT4 &Ambient) {
  CurrentAmbient = Ambient;
}

const dx::XMFLOAT4 &
RSLightsContainer::getCurrentAmbientLight() {
  return CurrentAmbient;
}

void
RSLightsContainer::uploadLightBloomDrawCall() {
  LOCK;
  for (auto &Light : LightsArray) {
    Light->uploadLightDrawCall();
  }
  UNLOCK;
}

void
RSLightsContainer::createLightBloom(const std::string &Name,
                                    const RS_SUBMESH_DATA &MeshData) {
  getRSLight(Name)->setLightBloom(MeshData);
}
