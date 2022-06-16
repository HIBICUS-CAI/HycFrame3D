#pragma once

#include <string>
#include <map>
#include "RSCommon.h"

enum class MODEL_FILE_TYPE
{
    BIN,
    JSON
};

struct MODEL_TEXTURE_INFO
{
    std::string mType = "";
    std::string mPath = "";
};

struct SUBMESH_BONE_DATA
{
    std::string mBoneName = "";
    DirectX::XMFLOAT4X4 mLocalToBone = {};
    DirectX::XMFLOAT4X4 mBoneTransform = {};
};

using SUBMESH_BONES = std::vector<SUBMESH_BONE_DATA>;

struct MESH_NODE
{
    std::string mNodeName = "";
    MESH_NODE* mParent = nullptr;
    std::vector<MESH_NODE*> mChildren = {};
    DirectX::XMFLOAT4X4 mThisToParent = {};
};

using POSITION_KEY = std::pair<float, DirectX::XMFLOAT3>;
using ROTATION_KEY = std::pair<float, DirectX::XMFLOAT4>;
using SCALING_KEY = std::pair<float, DirectX::XMFLOAT3>;

struct ANIMATION_CHANNEL
{
    std::string mNodeName = "";
    std::vector<POSITION_KEY> mPositionKeys = {};
    std::vector<ROTATION_KEY> mRotationKeys = {};
    std::vector<SCALING_KEY> mScalingKeys = {};
};

struct ANIMATION_INFO
{
    std::string mAnimationName = "";
    float mDuration = 0.f;
    float mTicksPerSecond = 0.f;
    std::vector<ANIMATION_CHANNEL> mNodeActions = {};
};

using MESH_ANIMATIONS = std::vector<ANIMATION_INFO>;

struct MESH_ANIMATION_DATA
{
    MESH_NODE* mRootNode = nullptr;
    MESH_ANIMATIONS mAllAnimations = {};

    void RelNode(MESH_NODE* _node)
    {
        if (_node->mChildren.size())
        {
            for (auto child : _node->mChildren) { RelNode(child); }
        }
        delete _node;
    }

    ~MESH_ANIMATION_DATA()
    {
        if (mRootNode)
        {
            RelNode(mRootNode);
        }
    }
};

void LoadModelFile(const std::string _filePath, MODEL_FILE_TYPE _type,
    int _subMeshIndex, RS_SUBMESH_DATA* _result,
    SUBMESH_BONES* _boneData = nullptr,
    MESH_ANIMATION_DATA** _animData = nullptr);

void AddDiffuseTexTo(RS_SUBMESH_DATA* _result, std::string _filePath);

void AddBumpedTexTo(RS_SUBMESH_DATA* _result, std::string _filePath);

void AddMetallicTexTo(RS_SUBMESH_DATA* _result, std::string _filePath);

void AddRoughnessTexTo(RS_SUBMESH_DATA* _result, std::string _filePath);
