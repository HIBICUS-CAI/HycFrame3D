#include "AssetsPool.h"
#include "RSMeshHelper.h"
#include "RSRoot_DX11.h"
#include "ALightComponent.h"
#include "UButtonComponent.h"

AssetsPool::AssetsPool(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode), mMeshPool({}), mSoundPool({}),
    mMeshAnimationsPool({})
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

SOUND_HANDLE AssetsPool::GetSoundIfExisted(std::string&& _soundName)
{
    auto found = mSoundPool.find(_soundName);
    if (found != mSoundPool.end())
    {
        return found->second;
    }
    else
    {
        P_LOG(LOG_WARNING, "this sound doesnt exist : %s\n",
            _soundName.c_str());
        return nullptr;
    }
}

SOUND_HANDLE AssetsPool::GetSoundIfExisted(std::string& _soundName)
{
    auto found = mSoundPool.find(_soundName);
    if (found != mSoundPool.end())
    {
        return found->second;
    }
    else
    {
        P_LOG(LOG_WARNING, "this sound doesnt exist : %s\n",
            _soundName.c_str());
        return nullptr;
    }
}

void AssetsPool::InsertNewMesh(std::string&& _meshName,
    RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType,
    SUBMESH_BONES* _bonesData, MESH_ANIMATION_DATA* _animationData)
{
    MESH_DATA md = {};
    mMeshPool.insert({ _meshName,md });
    mMeshPool[_meshName].mMeshData = _meshData;
    mMeshPool[_meshName].mMeshType = _meshType;
    if (_bonesData) { mMeshPool[_meshName].mBoneData = *_bonesData; }
    mMeshPool[_meshName].mInstanceVector.reserve(MAX_INSTANCE_SIZE);

    if (_meshData.mWithAnimation && _animationData)
    {
        // TODO shoule be able to process mutiply submesh data
        mMeshAnimationsPool.insert({ _meshName,_animationData });
    }
}

void AssetsPool::InsertNewMesh(std::string& _meshName,
    RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType,
    SUBMESH_BONES* _bonesData, MESH_ANIMATION_DATA* _animationData)
{
    MESH_DATA md = {};
    mMeshPool.insert({ _meshName,md });
    mMeshPool[_meshName].mMeshData = _meshData;
    mMeshPool[_meshName].mMeshType = _meshType;
    if (_bonesData) { mMeshPool[_meshName].mBoneData = *_bonesData; }
    mMeshPool[_meshName].mInstanceVector.reserve(MAX_INSTANCE_SIZE);

    if (_meshData.mWithAnimation && _animationData)
    {
        // TODO shoule be able to process mutiply submesh data
        mMeshAnimationsPool.insert({ _meshName,_animationData });
    }
}

void AssetsPool::InsertNewSound(std::string&& _soundName)
{
    SOUND_HANDLE sound = GetSoundHandle(_soundName);
#ifdef _DEBUG
    assert(sound);
#endif // _DEBUG
    mSoundPool.insert({ _soundName,sound });
}

void AssetsPool::InsertNewSound(std::string& _soundName)
{
    SOUND_HANDLE sound = GetSoundHandle(_soundName);
#ifdef _DEBUG
    assert(sound);
#endif // _DEBUG
    mSoundPool.insert({ _soundName,sound });
}

void AssetsPool::DeleteAllAssets()
{
    for (auto& mesh_data : mMeshPool)
    {
        if (mesh_data.first.find("-sprite") != std::string::npos)
        {
            continue;
        }
        else if (mesh_data.first == SELECTED_BTN_SPRITE_NAME)
        {
            continue;
        }

        GetRSRoot_DX11_Singleton()->MeshHelper()->
            ReleaseSubMesh(mesh_data.second.mMeshData);
    }
    mMeshPool.clear();

    for (auto& mesh_ani_data : mMeshAnimationsPool)
    {
        delete mesh_ani_data.second;
    }
    mMeshAnimationsPool.clear();

    mSoundPool.clear();
}
