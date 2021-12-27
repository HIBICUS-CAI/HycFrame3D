#pragma once

#include "Hyc3DCommon.h"
#include <vector>
#include <string>
#include <queue>
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

struct MESH_DATA
{
    RS_SUBMESH_DATA mMeshData = {};
    MESH_TYPE mMeshType = MESH_TYPE::SIZE;
    std::unordered_multimap<std::string, RS_INSTANCE_DATA> mInstanceMap = {};
    std::vector<RS_INSTANCE_DATA> mInstanceVector = {};
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

    void InsertNewMesh(std::string&& _meshName, RS_SUBMESH_DATA& _meshData,
        MESH_TYPE _meshType);
    void InsertNewMesh(std::string& _meshName, RS_SUBMESH_DATA& _meshData,
        MESH_TYPE _meshType);
    void InsertNewSound(std::string&& _soundName);
    void InsertNewSound(std::string& _soundName);

    void DeleteAllAssets();

private:
    const class SceneNode& mSceneNodeOwner;

    std::unordered_map<std::string, MESH_DATA> mMeshPool;
    std::unordered_map<std::string, SOUND_HANDLE> mSoundPool;
};
