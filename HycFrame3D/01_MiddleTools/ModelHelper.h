#pragma once

#include <RSCommon.h>

#include <map>
#include <string>

enum class MODEL_FILE_TYPE { BIN, JSON };

struct MODEL_TEXTURE_INFO {
  std::string Type = "";
  std::string Path = "";
};

struct SUBMESH_BONE_DATA {
  std::string BoneName = "";
  dx::XMFLOAT4X4 LocalToBone = {};
  dx::XMFLOAT4X4 BoneTransform = {};
};

using SUBMESH_BONES = std::vector<SUBMESH_BONE_DATA>;

struct MESH_NODE {
  std::string NodeName = "";
  MESH_NODE *Parent = nullptr;
  std::vector<MESH_NODE *> Children = {};
  dx::XMFLOAT4X4 ThisToParent = {};
};

using POSITION_KEY = std::pair<float, dx::XMFLOAT3>;
using ROTATION_KEY = std::pair<float, dx::XMFLOAT4>;
using SCALING_KEY = std::pair<float, dx::XMFLOAT3>;

struct ANIMATION_CHANNEL {
  std::string NodeName = "";
  std::vector<POSITION_KEY> PositionKeys = {};
  std::vector<ROTATION_KEY> RotationKeys = {};
  std::vector<SCALING_KEY> ScalingKeys = {};
};

struct ANIMATION_INFO {
  std::string AnimationName = "";
  float Duration = 0.f;
  float TicksPerSecond = 0.f;
  std::vector<ANIMATION_CHANNEL> NodeActions = {};
};

using MESH_ANIMATIONS = std::vector<ANIMATION_INFO>;

struct MESH_ANIMATION_DATA {
  MESH_NODE *RootNode = nullptr;
  MESH_ANIMATIONS AllAnimations = {};

  void
  releaseNode(MESH_NODE *NodePtr) {
    if (NodePtr->Children.size()) {
      for (auto Child : NodePtr->Children) {
        releaseNode(Child);
      }
    }
    delete NodePtr;
  }

  ~MESH_ANIMATION_DATA() {
    if (RootNode) {
      releaseNode(RootNode);
    }
  }
};

void
loadModelFile(const std::string& FilePath,
              MODEL_FILE_TYPE Type,
              int SubMeshIndex,
              RS_SUBMESH_DATA *OutResult,
              SUBMESH_BONES *OutBoneData = nullptr,
              MESH_ANIMATION_DATA **AnimData = nullptr);

void
addTextureToSubMesh(RS_SUBMESH_DATA *OutResult,
                    const std::string& FilePath,
                    MESH_TEXTURE_TYPE Type);
