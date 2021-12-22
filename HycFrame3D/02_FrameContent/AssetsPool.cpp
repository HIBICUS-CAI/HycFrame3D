#include "AssetsPool.h"

AssetsPool::AssetsPool(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode), mMeshPool({}), mSoundPool({})
{

}

AssetsPool::~AssetsPool()
{

}

RS_SUBMESH_DATA* AssetsPool::GetMeshIfExisted(std::string&& _meshName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

RS_SUBMESH_DATA* AssetsPool::GetMeshIfExisted(std::string& _meshName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

SOUND_HANDLE* AssetsPool::GetSoundIfExisted(std::string&& _soundName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

SOUND_HANDLE* AssetsPool::GetSoundIfExisted(std::string& _soundName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

void AssetsPool::InsertNewMesh(std::string&& _meshName, RS_SUBMESH_DATA& _meshData)
{

}

void AssetsPool::InsertNewMesh(std::string& _meshName, RS_SUBMESH_DATA& _meshData)
{

}

void AssetsPool::InsertNewSound(std::string&& _soundName, SOUND_HANDLE& _soundData)
{

}

void AssetsPool::InsertNewSound(std::string& _soundName, SOUND_HANDLE& _soundData)
{

}

void AssetsPool::DeleteAllAssets()
{

}
