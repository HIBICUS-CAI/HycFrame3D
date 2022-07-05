//---------------------------------------------------------------
// File: RSResourceManager.cpp
// Proj: RenderSystem_DX11
// Info: 保存并管理所有被创建的资源
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSResourceManager.h"

#include "RSRoot_DX11.h"

#define R_LOCK EnterCriticalSection(&ResDataLock)
#define R_UNLOCK LeaveCriticalSection(&ResDataLock)
#define M_LOCK EnterCriticalSection(&MesDataLock)
#define M_UNLOCK LeaveCriticalSection(&MesDataLock)

RSResourceManager::RSResourceManager()
    : RenderSystemRoot(nullptr), ResourceMap({}), MeshSrvMap({}),
      ResDataLock({}), MesDataLock({}) {}

RSResourceManager::~RSResourceManager() {}

bool
RSResourceManager::startUp(RSRoot_DX11 *RootPtr) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;

  InitializeCriticalSection(&ResDataLock);
  InitializeCriticalSection(&MesDataLock);

  return true;
}

void
RSResourceManager::cleanAndStop() {
  for (auto &RSMeshSrv : MeshSrvMap) {
    SAFE_RELEASE(RSMeshSrv.second);
  }
  for (auto &RSResource : ResourceMap) {
    SAFE_RELEASE(RSResource.second.Uav);
    SAFE_RELEASE(RSResource.second.Srv);
    SAFE_RELEASE(RSResource.second.Dsv);
    SAFE_RELEASE(RSResource.second.Rtv);
    auto Type = RSResource.second.Type;
    switch (Type) {
    case RS_RESOURCE_TYPE::BUFFER:
      SAFE_RELEASE(RSResource.second.Resource.Buffer);
      break;
    case RS_RESOURCE_TYPE::TEXTURE1D:
      SAFE_RELEASE(RSResource.second.Resource.Texture1D);
      break;
    case RS_RESOURCE_TYPE::TEXTURE2D:
      SAFE_RELEASE(RSResource.second.Resource.Texture2D);
      break;
    case RS_RESOURCE_TYPE::TEXTURE3D:
      SAFE_RELEASE(RSResource.second.Resource.Texture3D);
      break;
    default: {
      assert(false && "unvalid resource type");
      break;
    }
    }
  }
  MeshSrvMap.clear();
  ResourceMap.clear();

  DeleteCriticalSection(&ResDataLock);
  DeleteCriticalSection(&MesDataLock);
}

void
RSResourceManager::addResource(const std::string &Name,
                               const RS_RESOURCE_INFO &Resource) {
  R_LOCK;
  if (ResourceMap.find(Name) == ResourceMap.end()) {
    ResourceMap.insert({Name, Resource});
  }
  R_UNLOCK;
}

void
RSResourceManager::addMeshSrv(const std::string &Name,
                              ID3D11ShaderResourceView *MeshSrv) {
  M_LOCK;
  if (MeshSrvMap.find(Name) == MeshSrvMap.end()) {
    MeshSrvMap.insert({Name, MeshSrv});
  }
  M_UNLOCK;
}

RS_RESOURCE_INFO *
RSResourceManager::getResource(const std::string &Name) {
  R_LOCK;
  auto Found = ResourceMap.find(Name);
  if (Found != ResourceMap.end()) {
    auto ResourcePtr = &(Found->second);
    R_UNLOCK;
    return ResourcePtr;
  } else {
    R_UNLOCK;
    return nullptr;
  }
}

ID3D11ShaderResourceView *
RSResourceManager::getMeshSrv(const std::string &Name) {
  M_LOCK;
  auto Found = MeshSrvMap.find(Name);
  if (Found != MeshSrvMap.end()) {
    auto MeshSrv = Found->second;
    M_UNLOCK;
    return MeshSrv;
  } else {
    M_UNLOCK;
    return nullptr;
  }
}

void
RSResourceManager::deleteResource(const std::string &Name) {
  R_LOCK;
  auto Found = ResourceMap.find(Name);
  if (Found != ResourceMap.end()) {
    SAFE_RELEASE(Found->second.Uav);
    SAFE_RELEASE(Found->second.Srv);
    SAFE_RELEASE(Found->second.Dsv);
    SAFE_RELEASE(Found->second.Rtv);
    auto Type = Found->second.Type;
    switch (Type) {
    case RS_RESOURCE_TYPE::BUFFER:
      SAFE_RELEASE(Found->second.Resource.Buffer);
      break;
    case RS_RESOURCE_TYPE::TEXTURE1D:
      SAFE_RELEASE(Found->second.Resource.Texture1D);
      break;
    case RS_RESOURCE_TYPE::TEXTURE2D:
      SAFE_RELEASE(Found->second.Resource.Texture2D);
      break;
    case RS_RESOURCE_TYPE::TEXTURE3D:
      SAFE_RELEASE(Found->second.Resource.Texture3D);
      break;
    default: {
      assert(false && "unvalid resource type");
      break;
    }
    }
    ResourceMap.erase(Found);
  }
  R_UNLOCK;
}

void
RSResourceManager::deleteMeshSrv(const std::string &Name) {
  M_LOCK;
  auto Found = MeshSrvMap.find(Name);
  if (Found != MeshSrvMap.end()) {
    SAFE_RELEASE(Found->second);
    MeshSrvMap.erase(Found);
  }
  M_UNLOCK;
}

void
RSResourceManager::clearResources() {
  R_LOCK;
  for (auto &RSResource : ResourceMap) {
    SAFE_RELEASE(RSResource.second.Uav);
    SAFE_RELEASE(RSResource.second.Srv);
    SAFE_RELEASE(RSResource.second.Dsv);
    SAFE_RELEASE(RSResource.second.Rtv);
    auto Type = RSResource.second.Type;
    switch (Type) {
    case RS_RESOURCE_TYPE::BUFFER:
      SAFE_RELEASE(RSResource.second.Resource.Buffer);
      break;
    case RS_RESOURCE_TYPE::TEXTURE1D:
      SAFE_RELEASE(RSResource.second.Resource.Texture1D);
      break;
    case RS_RESOURCE_TYPE::TEXTURE2D:
      SAFE_RELEASE(RSResource.second.Resource.Texture2D);
      break;
    case RS_RESOURCE_TYPE::TEXTURE3D:
      SAFE_RELEASE(RSResource.second.Resource.Texture3D);
      break;
    default: {
      assert(false && "unvalid resource type");
      break;
    }
    }
  }
  ResourceMap.clear();
  R_UNLOCK;
}

void
RSResourceManager::clearMeshSrvs() {
  M_LOCK;
  for (auto &RSMeshSrv : MeshSrvMap) {
    SAFE_RELEASE(RSMeshSrv.second);
  }
  MeshSrvMap.clear();
  M_UNLOCK;
}
