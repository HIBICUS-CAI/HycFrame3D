#pragma once

#include "Hyc3DCommon.h"

#include "ModelHelper.h"
#include "SoundHelper.h"

#include <RSCommon.h>

#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

enum class MESH_TYPE {
  OPACITY,
  TRANSPARENCY,
  LIGHT,
  UI_SPRITE,

  SIZE
};

struct SUBMESH_DATA {
  RS_SUBMESH_DATA MeshData = {};
  MESH_TYPE MeshType = MESH_TYPE::SIZE;
  std::unordered_multimap<std::string, RS_INSTANCE_DATA> InstanceMap = {};
  std::vector<RS_INSTANCE_DATA> InstanceVector = {};
  SUBMESH_BONES OriginBoneData = {};
  std::unordered_map<std::string, SUBMESH_BONES> BonesMap = {};
  std::vector<SUBMESH_BONES> BonesVector = {};
};

using SUBMESH_NAME_VEC = std::vector<std::string>;

class AssetsPool {
private:
  const class SceneNode &SceneNodeOwner;

  std::unordered_map<std::string, SUBMESH_DATA> SubMeshPool;
  std::unordered_map<std::string, std::string> SubMeshToMesh;
  std::unordered_map<std::string, SUBMESH_NAME_VEC> MeshPool;
  std::unordered_map<std::string, MESH_ANIMATION_DATA *> MeshAnimationsPool;
  std::unordered_map<std::string, SOUND_HANDLE> SoundPool;

public:
  AssetsPool(class SceneNode &SceneNode);
  ~AssetsPool();

  SUBMESH_DATA *
  getSubMeshIfExisted(const std::string &MeshName);

  SUBMESH_NAME_VEC *
  getMeshIfExisted(const std::string &MeshName);

  MESH_ANIMATION_DATA *
  getAnimationIfExistedSub(const std::string &AniName);

  MESH_ANIMATION_DATA *
  getAnimationIfExisted(const std::string &AniName);

  SOUND_HANDLE
  getSoundIfExisted(const std::string &SoundName);

  void
  insertNewSubMesh(const std::string &MeshName,
                   const RS_SUBMESH_DATA &MeshData,
                   MESH_TYPE MeshType,
                   const SUBMESH_BONES *BonesData = nullptr,
                   const MESH_ANIMATION_DATA *AnimationData = nullptr);

  void
  insertNewIndexedMesh(const std::string &MeshName,
                       const RS_SUBMESH_DATA &MeshData,
                       MESH_TYPE MeshType,
                       int SubIndex,
                       const SUBMESH_BONES *BonesData = nullptr,
                       const MESH_ANIMATION_DATA *AnimationData = nullptr);

  void
  insertNewSound(const std::string &SoundName);

  void
  deleteAllAssets();

  friend class RenderSystem;
};
