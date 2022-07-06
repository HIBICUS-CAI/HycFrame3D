#include "AssetsPool.h"

#include "ALightComponent.h"
#include "UButtonComponent.h"

#include <RSMeshHelper.h>
#include <RSRoot_DX11.h>

AssetsPool::AssetsPool(SceneNode &SceneNode)
    : SceneNodeOwner(SceneNode), SubMeshPool({}), SubMeshToMesh({}),
      MeshPool({}), MeshAnimationsPool({}), SoundPool({}) {
  SubMeshPool.reserve(256);
  MeshPool.reserve(256);
  SubMeshToMesh.reserve(256);
}

AssetsPool::~AssetsPool() {}

SUBMESH_DATA *
AssetsPool::getSubMeshIfExisted(const std::string &MeshName) {
  auto Found = SubMeshPool.find(MeshName);
  if (Found != SubMeshPool.end()) {
    return &(Found->second);
  } else {
    P_LOG(LOG_WARNING, "this mesh doesnt exist : %s\n", MeshName.c_str());
    return nullptr;
  }
}

SUBMESH_NAME_VEC *
AssetsPool::getMeshIfExisted(const std::string &MeshName) {
  auto Found = MeshPool.find(MeshName);
  if (Found != MeshPool.end()) {
    return &(Found->second);
  } else {
    P_LOG(LOG_WARNING, "this mesh doesnt exist : %s\n", MeshName.c_str());
    return nullptr;
  }
}

MESH_ANIMATION_DATA *
AssetsPool::getAnimationIfExistedSub(const std::string &AniName) {
  auto Found = SubMeshToMesh.find(AniName);
  if (Found != SubMeshToMesh.end()) {
    auto AniFound = MeshAnimationsPool.find(Found->second);
    if (AniFound != MeshAnimationsPool.end()) {
      return AniFound->second;
    } else {
      P_LOG(LOG_WARNING, "cannot found this animation : %s\n", AniName.c_str());
      return nullptr;
    }
  } else {
    P_LOG(LOG_WARNING, "cannot found this animation's main mesh : %s\n",
          AniName.c_str());
    return nullptr;
  }
}

MESH_ANIMATION_DATA *
AssetsPool::getAnimationIfExisted(const std::string &AniName) {
  auto AniFound = MeshAnimationsPool.find(AniName);
  if (AniFound != MeshAnimationsPool.end()) {
    return AniFound->second;
  } else {
    P_LOG(LOG_WARNING, "cannot found this animation : %s\n", AniName.c_str());
    return nullptr;
  }
}

SOUND_HANDLE
AssetsPool::getSoundIfExisted(const std::string &SoundName) {
  auto Found = SoundPool.find(SoundName);
  if (Found != SoundPool.end()) {
    return Found->second;
  } else {
    P_LOG(LOG_WARNING, "this sound doesnt exist : %s\n", SoundName.c_str());
    return nullptr;
  }
}

void
AssetsPool::insertNewSubMesh(const std::string &MeshName,
                             const RS_SUBMESH_DATA &MeshData,
                             MESH_TYPE MeshType,
                             const SUBMESH_BONES *BonesData,
                             const MESH_ANIMATION_DATA *AnimationData) {
  SUBMESH_DATA SD = {};
  SubMeshPool.insert({MeshName, SD});
  SubMeshPool[MeshName].MeshData = MeshData;
  SubMeshPool[MeshName].MeshType = MeshType;
  if (BonesData) {
    SubMeshPool[MeshName].OriginBoneData = *BonesData;
  }
  SubMeshPool[MeshName].InstanceVector.reserve(MAX_INSTANCE_SIZE);

  if (MeshData.AnimationFlag && AnimationData) {
    MeshAnimationsPool.insert(
        {MeshName, const_cast<MESH_ANIMATION_DATA *>(AnimationData)});
  }
}

void
AssetsPool::insertNewIndexedMesh(const std::string &MeshName,
                                 const RS_SUBMESH_DATA &MeshData,
                                 MESH_TYPE MeshType,
                                 int SubIndex,
                                 const SUBMESH_BONES *BonesData,
                                 const MESH_ANIMATION_DATA *AnimationData) {
  std::string SubMeshName = MeshName + std::to_string(SubIndex);
  MeshPool[MeshName].emplace_back(SubMeshName);
  SubMeshToMesh[SubMeshName] = MeshName;

  SUBMESH_DATA SD = {};
  SubMeshPool.insert({SubMeshName, SD});
  SubMeshPool[SubMeshName].MeshData = MeshData;
  SubMeshPool[SubMeshName].MeshType = MeshType;
  if (BonesData) {
    SubMeshPool[SubMeshName].OriginBoneData = *BonesData;
  }
  SubMeshPool[SubMeshName].InstanceVector.reserve(MAX_INSTANCE_SIZE);

  if (MeshData.AnimationFlag && AnimationData &&
      MeshAnimationsPool.find(MeshName) == MeshAnimationsPool.end()) {
    MeshAnimationsPool.insert(
        {MeshName, const_cast<MESH_ANIMATION_DATA *>(AnimationData)});
  } else {
    if (AnimationData) {
      delete AnimationData;
    }
  }
}

void
AssetsPool::insertNewSound(const std::string &SoundName) {
  SOUND_HANDLE Sound = getSoundHandle(SoundName);
#ifdef _DEBUG
  assert(Sound);
#endif // _DEBUG
  SoundPool.insert({SoundName, Sound});
}

void
AssetsPool::deleteAllAssets() {
  for (auto &SubMeshData : SubMeshPool) {
    if (SubMeshData.first.find("-sprite") != std::string::npos) {
      continue;
    } else if (SubMeshData.first == SELECTED_BTN_SPRITE_NAME) {
      continue;
    }

    getRSDX11RootInstance()->getMeshHelper()->releaseSubMesh(
        SubMeshData.second.MeshData);
  }
  SubMeshPool.clear();
  MeshPool.clear();
  SubMeshToMesh.clear();

  for (auto &MeshAniData : MeshAnimationsPool) {
    delete MeshAniData.second;
  }
  MeshAnimationsPool.clear();

  SoundPool.clear();
}
