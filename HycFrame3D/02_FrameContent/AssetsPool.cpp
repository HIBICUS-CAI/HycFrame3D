#include "AssetsPool.h"
#include "RSMeshHelper.h"
#include "RSRoot_DX11.h"

AssetsPool::AssetsPool(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode), mMeshPool({}), mSoundPool({})
{
    mMeshPool.reserve(256);
}

AssetsPool::~AssetsPool()
{

}

MESH_DATA* AssetsPool::GetMeshIfExisted(std::string&& _meshName)
{
    auto found = mMeshPool.find(_meshName);
    if (found != mMeshPool.end())
    {
        return &(found->second);
    }
    else
    {
        P_LOG(LOG_WARNING, "this mesh doesnt exist : %s\n", _meshName.c_str());
        return nullptr;
    }
}

MESH_DATA* AssetsPool::GetMeshIfExisted(std::string& _meshName)
{
    auto found = mMeshPool.find(_meshName);
    if (found != mMeshPool.end())
    {
        return &(found->second);
    }
    else
    {
        P_LOG(LOG_WARNING, "this mesh doesnt exist : %s\n", _meshName.c_str());
        return nullptr;
    }
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

void AssetsPool::InsertNewMesh(std::string&& _meshName,
    RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType)
{
    MESH_DATA md = {};
    mMeshPool.insert({ _meshName,md });
    mMeshPool[_meshName].mMeshData = _meshData;
    mMeshPool[_meshName].mMeshType = _meshType;
    mMeshPool[_meshName].mInstanceVector.reserve(MAX_INSTANCE_SIZE);
}

void AssetsPool::InsertNewMesh(std::string& _meshName,
    RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType)
{
    MESH_DATA md = {};
    mMeshPool.insert({ _meshName,md });
    mMeshPool[_meshName].mMeshData = _meshData;
    mMeshPool[_meshName].mMeshType = _meshType;
    mMeshPool[_meshName].mInstanceVector.reserve(MAX_INSTANCE_SIZE);
}

void AssetsPool::InsertNewSound(std::string&& _soundName, SOUND_HANDLE& _soundData)
{

}

void AssetsPool::InsertNewSound(std::string& _soundName, SOUND_HANDLE& _soundData)
{

}

void AssetsPool::DeleteAllAssets()
{
    for (auto& mesh_data : mMeshPool)
    {
        GetRSRoot_DX11_Singleton()->MeshHelper()->
            ReleaseSubMesh(mesh_data.second.mMeshData);
    }
    mMeshPool.clear();
}
