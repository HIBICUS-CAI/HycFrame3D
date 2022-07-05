//---------------------------------------------------------------
// File: RSResourceManager.h
// Proj: RenderSystem_DX11
// Info: 保存并管理所有被创建的资源
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include <unordered_map>

class RSResourceManager {
private:
  class RSRoot_DX11 *RenderSystemRoot;
  std::unordered_map<std::string, RS_RESOURCE_INFO> ResourceMap;
  std::unordered_map<std::string, ID3D11ShaderResourceView *> MeshSrvMap;

  CRITICAL_SECTION ResDataLock;
  CRITICAL_SECTION MesDataLock;

public:
  RSResourceManager();
  ~RSResourceManager();

  bool
  startUp(class RSRoot_DX11 *RootPtr);

  void
  cleanAndStop();

  void
  addResource(const std::string &Name, const RS_RESOURCE_INFO &Resource);

  void
  addMeshSrv(const std::string &Name, ID3D11ShaderResourceView *Srv);

  RS_RESOURCE_INFO *
  getResource(const std::string &Name);

  ID3D11ShaderResourceView *
  getMeshSrv(const std::string &Name);

  void
  deleteResource(const std::string &Name);

  void
  deleteMeshSrv(const std::string &Name);

  void
  clearResources();

  void
  clearMeshSrvs();
};
