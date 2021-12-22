#pragma once

#include "Hyc3DCommon.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "RSCommon.h"
#include "SoundHelper.h"

class AssetsPool
{
public:
    AssetsPool(class SceneNode& _sceneNode);
    ~AssetsPool();

    RS_SUBMESH_DATA* GetMeshIfExisted(std::string&& _meshName);
    RS_SUBMESH_DATA* GetMeshIfExisted(std::string& _meshName);
    SOUND_HANDLE* GetSoundIfExisted(std::string&& _soundName);
    SOUND_HANDLE* GetSoundIfExisted(std::string& _soundName);

    void InsertNewMesh(std::string&& _meshName, RS_SUBMESH_DATA& _meshData);
    void InsertNewMesh(std::string& _meshName, RS_SUBMESH_DATA& _meshData);
    void InsertNewSound(std::string&& _soundName, SOUND_HANDLE& _soundData);
    void InsertNewSound(std::string& _soundName, SOUND_HANDLE& _soundData);

    void DeleteAllAssets();

private:
    const class SceneNode& mSceneNodeOwner;

    std::unordered_map<std::string, RS_SUBMESH_DATA> mMeshPool;
    std::unordered_map<std::string, SOUND_HANDLE> mSoundPool;
};
