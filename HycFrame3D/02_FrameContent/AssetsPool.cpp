#include "AssetsPool.h"
#include "RSMeshHelper.h"
#include "RSRoot_DX11.h"
#include "ALightComponent.h"
#include "UButtonComponent.h"

AssetsPool::AssetsPool(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode),
    mSubMeshPool({}),
    mSubMeshToMesh({}),
    mMeshPool({}),
    mMeshAnimationsPool({}),
    mSoundPool({})
{
    mSubMeshPool.reserve(256);
    mMeshPool.reserve(256);
    mSubMeshToMesh.reserve(256);
}

AssetsPool::~AssetsPool()
{

}

SUBMESH_DATA* AssetsPool::GetSubMeshIfExisted(std::string&& _meshName)
{
    auto found = mSubMeshPool.find(_meshName);
    if (found != mSubMeshPool.end())
    {
        return &(found->second);
    }
    else
    {
        P_LOG(LOG_WARNING, "this mesh doesnt exist : %s\n", _meshName.c_str());
        return nullptr;
    }
}

SUBMESH_DATA* AssetsPool::GetSubMeshIfExisted(std::string& _meshName)
{
    auto found = mSubMeshPool.find(_meshName);
    if (found != mSubMeshPool.end())
    {
        return &(found->second);
    }
    else
    {
        P_LOG(LOG_WARNING, "this mesh doesnt exist : %s\n", _meshName.c_str());
        return nullptr;
    }
}

SUBMESH_NAME_VEC* AssetsPool::GetMeshIfExisted(std::string&& _meshName)
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

SUBMESH_NAME_VEC* AssetsPool::GetMeshIfExisted(std::string& _meshName)
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

MESH_ANIMATION_DATA* AssetsPool::GetAnimationIfExistedSub(
    std::string&& _aniName)
{
    auto found = mSubMeshToMesh.find(_aniName);
    if (found != mSubMeshToMesh.end())
    {
        auto aniFound = mMeshAnimationsPool.find(found->second);
        if (aniFound != mMeshAnimationsPool.end())
        {
            return aniFound->second;
        }
        else
        {
            P_LOG(LOG_WARNING, "cannot found this animation : %s\n",
                _aniName.c_str());
            return nullptr;
        }
    }
    else
    {
        P_LOG(LOG_WARNING, "cannot found this animation's main mesh : %s\n",
            _aniName.c_str());
        return nullptr;
    }
}

MESH_ANIMATION_DATA* AssetsPool::GetAnimationIfExistedSub(
    std::string& _aniName)
{
    auto found = mSubMeshToMesh.find(_aniName);
    if (found != mSubMeshToMesh.end())
    {
        auto aniFound = mMeshAnimationsPool.find(found->second);
        if (aniFound != mMeshAnimationsPool.end())
        {
            return aniFound->second;
        }
        else
        {
            P_LOG(LOG_WARNING, "cannot found this animation : %s\n",
                _aniName.c_str());
            return nullptr;
        }
    }
    else
    {
        P_LOG(LOG_WARNING, "cannot found this animation's main mesh : %s\n",
            _aniName.c_str());
        return nullptr;
    }
}

MESH_ANIMATION_DATA* AssetsPool::GetAnimationIfExisted(
    std::string&& _aniName)
{
    auto aniFound = mMeshAnimationsPool.find(_aniName);
    if (aniFound != mMeshAnimationsPool.end())
    {
        return aniFound->second;
    }
    else
    {
        P_LOG(LOG_WARNING, "cannot found this animation : %s\n",
            _aniName.c_str());
        return nullptr;
    }
}

MESH_ANIMATION_DATA* AssetsPool::GetAnimationIfExisted(
    std::string& _aniName)
{
    auto aniFound = mMeshAnimationsPool.find(_aniName);
    if (aniFound != mMeshAnimationsPool.end())
    {
        return aniFound->second;
    }
    else
    {
        P_LOG(LOG_WARNING, "cannot found this animation : %s\n",
            _aniName.c_str());
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

void AssetsPool::InsertNewSubMesh(std::string&& _meshName,
    RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType,
    SUBMESH_BONES* _bonesData, MESH_ANIMATION_DATA* _animationData)
{
    SUBMESH_DATA md = {};
    mSubMeshPool.insert({ _meshName,md });
    mSubMeshPool[_meshName].mMeshData = _meshData;
    mSubMeshPool[_meshName].mMeshType = _meshType;
    if (_bonesData) { mSubMeshPool[_meshName].mOriginBoneData = *_bonesData; }
    mSubMeshPool[_meshName].mInstanceVector.reserve(MAX_INSTANCE_SIZE);

    if (_meshData.AnimationFlag && _animationData)
    {
        mMeshAnimationsPool.insert({ _meshName,_animationData });
    }
}

void AssetsPool::InsertNewSubMesh(std::string& _meshName,
    RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType,
    SUBMESH_BONES* _bonesData, MESH_ANIMATION_DATA* _animationData)
{
    SUBMESH_DATA md = {};
    mSubMeshPool.insert({ _meshName,md });
    mSubMeshPool[_meshName].mMeshData = _meshData;
    mSubMeshPool[_meshName].mMeshType = _meshType;
    if (_bonesData) { mSubMeshPool[_meshName].mOriginBoneData = *_bonesData; }
    mSubMeshPool[_meshName].mInstanceVector.reserve(MAX_INSTANCE_SIZE);

    if (_meshData.AnimationFlag && _animationData)
    {
        mMeshAnimationsPool.insert({ _meshName,_animationData });
    }
}

void AssetsPool::InsertNewIndexedMesh(std::string&& _meshName,
    RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType,
    int _subIndex, SUBMESH_BONES* _bonesData,
    MESH_ANIMATION_DATA* _animationData)
{
    std::string subMeshName = _meshName + std::to_string(_subIndex);
    mMeshPool[_meshName].emplace_back(subMeshName);
    mSubMeshToMesh[subMeshName] = _meshName;

    SUBMESH_DATA md = {};
    mSubMeshPool.insert({ subMeshName,md });
    mSubMeshPool[subMeshName].mMeshData = _meshData;
    mSubMeshPool[subMeshName].mMeshType = _meshType;
    if (_bonesData) { mSubMeshPool[subMeshName].mOriginBoneData = *_bonesData; }
    mSubMeshPool[subMeshName].mInstanceVector.reserve(MAX_INSTANCE_SIZE);

    if (_meshData.AnimationFlag && _animationData &&
        mMeshAnimationsPool.find(_meshName) == mMeshAnimationsPool.end())
    {
        mMeshAnimationsPool.insert({ _meshName,_animationData });
    }
    else
    {
        if (_animationData) { delete _animationData; }
    }
}

void AssetsPool::InsertNewIndexedMesh(std::string& _meshName,
    RS_SUBMESH_DATA& _meshData, MESH_TYPE _meshType,
    int _subIndex, SUBMESH_BONES* _bonesData,
    MESH_ANIMATION_DATA* _animationData)
{
    std::string subMeshName = _meshName + std::to_string(_subIndex);
    mMeshPool[_meshName].emplace_back(subMeshName);
    mSubMeshToMesh[subMeshName] = _meshName;

    SUBMESH_DATA md = {};
    mSubMeshPool.insert({ subMeshName,md });
    mSubMeshPool[subMeshName].mMeshData = _meshData;
    mSubMeshPool[subMeshName].mMeshType = _meshType;
    if (_bonesData) { mSubMeshPool[subMeshName].mOriginBoneData = *_bonesData; }
    mSubMeshPool[subMeshName].mInstanceVector.reserve(MAX_INSTANCE_SIZE);

    if (_meshData.AnimationFlag && _animationData &&
        mMeshAnimationsPool.find(_meshName) == mMeshAnimationsPool.end())
    {
        mMeshAnimationsPool.insert({ _meshName,_animationData });
    }
    else
    {
        if (_animationData) { delete _animationData; }
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
    for (auto& mesh_data : mSubMeshPool)
    {
        if (mesh_data.first.find("-sprite") != std::string::npos)
        {
            continue;
        }
        else if (mesh_data.first == SELECTED_BTN_SPRITE_NAME)
        {
            continue;
        }

        getRSDX11RootInstance()->getMeshHelper()->
            releaseSubMesh(mesh_data.second.mMeshData);
    }
    mSubMeshPool.clear();
    mMeshPool.clear();
    mSubMeshToMesh.clear();

    for (auto& mesh_ani_data : mMeshAnimationsPool)
    {
        delete mesh_ani_data.second;
    }
    mMeshAnimationsPool.clear();

    mSoundPool.clear();
}
