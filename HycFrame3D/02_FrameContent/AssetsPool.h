#pragma once

#include "Hyc3DCommon.h"
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <unordered_map>
#include "RSCommon.h"
#include "SoundHelper.h"

enum class MESH_TYPE
{
    OPACITY,
    LIGHT,
    UI_SPRITE,

    SIZE
};

struct SUBMESH_BONE_DATA
{
    std::string mBoneName = "";
    DirectX::XMFLOAT4X4 mLocalToBone = {};
    DirectX::XMFLOAT4X4 mBoneTransform = {};
};

using SUBMESH_BONES = std::vector<SUBMESH_BONE_DATA>;

struct MESH_DATA
{
    RS_SUBMESH_DATA mMeshData = {};
    MESH_TYPE mMeshType = MESH_TYPE::SIZE;
    std::unordered_multimap<std::string, RS_INSTANCE_DATA> mInstanceMap = {};
    std::vector<RS_INSTANCE_DATA> mInstanceVector = {};
    // TODO shoule be able to process mutiply instance data
    SUBMESH_BONES mBoneData = {};
};

struct MESH_NODE
{
    std::string mNodeName = "";
    MESH_NODE* mParent = nullptr;
    std::vector<MESH_NODE*> mChildren = {};
    DirectX::XMFLOAT4X4 mThisToParent = {};
};

using POSITION_KEY = std::pair<float, DirectX::XMFLOAT3>;
using ROTATION_KEY = std::pair<float, DirectX::XMFLOAT3>;
using SCALING_KEY = std::pair<float, DirectX::XMFLOAT3>;

struct ANIMATION_CHANNEL
{
    std::string mNodeName = "";
    std::map<float, DirectX::XMFLOAT3> mPositionKeys = {};
    std::map<float, DirectX::XMFLOAT4> mRotationKeys = {};
    std::map<float, DirectX::XMFLOAT3> mScalingKeys = {};
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
};

class AssetsPool
{
    friend class RenderSystem;

public:
    AssetsPool(class SceneNode& _sceneNode);
    ~AssetsPool();

    MESH_DATA* GetMeshIfExisted(std::string&& _meshName);
    MESH_DATA* GetMeshIfExisted(std::string& _meshName);
    SOUND_HANDLE GetSoundIfExisted(std::string&& _soundName);
    SOUND_HANDLE GetSoundIfExisted(std::string& _soundName);

    void InsertNewMesh(std::string&& _meshName,
        RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType,
        SUBMESH_BONES* _bonesData = nullptr,
        MESH_ANIMATION_DATA* _animationData = nullptr);
    void InsertNewMesh(std::string& _meshName,
        RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType,
        SUBMESH_BONES* _bonesData = nullptr,
        MESH_ANIMATION_DATA* _animationData = nullptr);
    void InsertNewSound(std::string&& _soundName);
    void InsertNewSound(std::string& _soundName);

    void DeleteAllAssets();

private:
    const class SceneNode& mSceneNodeOwner;

    std::unordered_map<std::string, MESH_DATA> mMeshPool;
    std::unordered_map<std::string, MESH_ANIMATION_DATA*> mMeshAnimationsPool;
    std::unordered_map<std::string, SOUND_HANDLE> mSoundPool;
};
