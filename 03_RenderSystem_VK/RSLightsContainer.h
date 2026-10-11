//---------------------------------------------------------------
// File: RSLightsContainer.h
// Proj: RenderSystem_VK
// Info: 对所有的光源进行引用及简单处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include <mutex>

#include <unordered_map>

class RSLightsContainer {
private:
  class RSRoot_VK *RenderSystemRoot;

  std::unordered_map<std::string, class RSLight *> LightsMap;
  std::unordered_map<std::string, DirectX::XMFLOAT4> AmbientLightsMap;

  DirectX::XMFLOAT4 CurrentAmbient;

  std::vector<class RSLight *> LightsArray;
  std::vector<class RSLight *> ShadowLightsArray;
  std::vector<INT> ShadowLightIndeicesArray;

  std::recursive_mutex DataLock;

public:
  RSLightsContainer();
  ~RSLightsContainer();

  bool startUp(class RSRoot_VK *RootPtr);
  void cleanAndStop();

  class RSLight *createRSLight(const std::string &Name, const LIGHT_INFO *Info);
  class RSLight *getRSLight(const std::string &Name);
  RS_LIGHT_INFO *getRSLightInfo(const std::string &Name);
  void deleteRSLight(const std::string &Name, bool DeleteByFrameFlag);

  bool createLightCameraFor(const std::string &Name, const CAM_INFO *Info);

  void insertAmbientLight(const std::string &Name,
                          const DirectX::XMFLOAT4 &Light);
  void eraseAmbientLight(const std::string &Name);
  void setCurrentAmbientLight(const std::string &Name);
  void forceCurrentAmbientLight(const DirectX::XMFLOAT4 &Ambient);
  const DirectX::XMFLOAT4 &getCurrentAmbientLight();

  std::vector<class RSLight *> *getLightsArray();
  std::vector<class RSLight *> *getShadowLightsArray();
  std::vector<INT> *getShadowLightIndeicesArray();

  void createLightBloom(const std::string &Name,
                        const RS_SUBMESH_DATA &MeshData);
  void uploadLightBloomDrawCall();

  void lockContainer() { DataLock.lock(); }
  void unlockContainer() { DataLock.unlock(); }

private:
  const DirectX::XMFLOAT4 &getAmbientLight(const std::string &Name);
};
