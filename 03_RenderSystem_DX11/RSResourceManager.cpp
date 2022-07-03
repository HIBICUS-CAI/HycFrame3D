﻿//---------------------------------------------------------------
// File: RSResourceManager.cpp
// Proj: RenderSystem_DX11
// Info: 保存并管理所有被创建的资源
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSResourceManager.h"
#include "RSRoot_DX11.h"

#define R_LOCK EnterCriticalSection(&mResDataLock)
#define R_UNLOCK LeaveCriticalSection(&mResDataLock)
#define M_LOCK EnterCriticalSection(&mMesDataLock)
#define M_UNLOCK LeaveCriticalSection(&mMesDataLock)

RSResourceManager::RSResourceManager() :
    mRootPtr(nullptr), mResourceMap({}), mMeshSrvMap({}),
    mResDataLock({}), mMesDataLock({})
{

}

RSResourceManager::~RSResourceManager()
{

}

bool RSResourceManager::StartUp(RSRoot_DX11* _root)
{
    if (!_root) { return false; }

    mRootPtr = _root;

    InitializeCriticalSection(&mResDataLock);
    InitializeCriticalSection(&mMesDataLock);

    return true;
}

void RSResourceManager::CleanAndStop()
{
    for (auto& meshSrv : mMeshSrvMap)
    {
        SAFE_RELEASE(meshSrv.second);
    }
    for (auto& resource : mResourceMap)
    {
        SAFE_RELEASE(resource.second.mUav);
        SAFE_RELEASE(resource.second.mSrv);
        SAFE_RELEASE(resource.second.mDsv);
        SAFE_RELEASE(resource.second.mRtv);
        auto type = resource.second.mType;
        switch (type)
        {
        case RS_RESOURCE_TYPE::BUFFER:
            SAFE_RELEASE(resource.second.mResource.mBuffer);
            break;
        case RS_RESOURCE_TYPE::TEXTURE1D:
            SAFE_RELEASE(resource.second.mResource.mTexture1D);
            break;
        case RS_RESOURCE_TYPE::TEXTURE2D:
            SAFE_RELEASE(resource.second.mResource.mTexture2D);
            break;
        case RS_RESOURCE_TYPE::TEXTURE3D:
            SAFE_RELEASE(resource.second.mResource.mTexture3D);
            break;
        default:
        {
            bool unvalid_resource_type = false;
            assert(unvalid_resource_type);
            (void)unvalid_resource_type;
            break;
        }
        }
    }
    mMeshSrvMap.clear();
    mResourceMap.clear();

    DeleteCriticalSection(&mResDataLock);
    DeleteCriticalSection(&mMesDataLock);
}

void RSResourceManager::AddResource(
    std::string& _name, RS_RESOURCE_INFO& _resource)
{
    R_LOCK;
    if (mResourceMap.find(_name) == mResourceMap.end())
    {
        mResourceMap.insert({ _name,_resource });
    }
    R_UNLOCK;
}

void RSResourceManager::AddMeshSrv(
    std::string& _name, ID3D11ShaderResourceView* _srv)
{
    M_LOCK;
    if (mMeshSrvMap.find(_name) == mMeshSrvMap.end())
    {
        mMeshSrvMap.insert({ _name,_srv });
    }
    M_UNLOCK;
}

RS_RESOURCE_INFO* RSResourceManager::GetResourceInfo(
    std::string& _name)
{
    R_LOCK;
    auto found = mResourceMap.find(_name);
    if (found != mResourceMap.end())
    {
        auto res = &(found->second);
        R_UNLOCK;
        return res;
    }
    else
    {
        R_UNLOCK;
        return nullptr;
    }
}

ID3D11ShaderResourceView* RSResourceManager::GetMeshSrv(
    std::string& _name)
{
    M_LOCK;
    auto found = mMeshSrvMap.find(_name);
    if (found != mMeshSrvMap.end())
    {
        auto srv = found->second;
        M_UNLOCK;
        return srv;
    }
    else
    {
        M_UNLOCK;
        return nullptr;
    }
}

void RSResourceManager::DeleteResource(std::string& _name)
{
    R_LOCK;
    auto found = mResourceMap.find(_name);
    if (found != mResourceMap.end())
    {
        SAFE_RELEASE(found->second.mUav);
        SAFE_RELEASE(found->second.mSrv);
        SAFE_RELEASE(found->second.mDsv);
        SAFE_RELEASE(found->second.mRtv);
        auto type = found->second.mType;
        switch (type)
        {
        case RS_RESOURCE_TYPE::BUFFER:
            SAFE_RELEASE(found->second.mResource.mBuffer);
            break;
        case RS_RESOURCE_TYPE::TEXTURE1D:
            SAFE_RELEASE(found->second.mResource.mTexture1D);
            break;
        case RS_RESOURCE_TYPE::TEXTURE2D:
            SAFE_RELEASE(found->second.mResource.mTexture2D);
            break;
        case RS_RESOURCE_TYPE::TEXTURE3D:
            SAFE_RELEASE(found->second.mResource.mTexture3D);
            break;
        default:
        {
            bool unvalid_resource_type = false;
            assert(unvalid_resource_type);
            (void)unvalid_resource_type;
            break;
        }
        }
        mResourceMap.erase(found);
    }
    R_UNLOCK;
}

void RSResourceManager::DeleteMeshSrv(std::string& _name)
{
    M_LOCK;
    auto found = mMeshSrvMap.find(_name);
    if (found != mMeshSrvMap.end())
    {
        SAFE_RELEASE(found->second);
        mMeshSrvMap.erase(found);
    }
    M_UNLOCK;
}

void RSResourceManager::ClearResources()
{
    R_LOCK;
    for (auto& resource : mResourceMap)
    {
        SAFE_RELEASE(resource.second.mUav);
        SAFE_RELEASE(resource.second.mSrv);
        SAFE_RELEASE(resource.second.mDsv);
        SAFE_RELEASE(resource.second.mRtv);
        auto type = resource.second.mType;
        switch (type)
        {
        case RS_RESOURCE_TYPE::BUFFER:
            SAFE_RELEASE(resource.second.mResource.mBuffer);
            break;
        case RS_RESOURCE_TYPE::TEXTURE1D:
            SAFE_RELEASE(resource.second.mResource.mTexture1D);
            break;
        case RS_RESOURCE_TYPE::TEXTURE2D:
            SAFE_RELEASE(resource.second.mResource.mTexture2D);
            break;
        case RS_RESOURCE_TYPE::TEXTURE3D:
            SAFE_RELEASE(resource.second.mResource.mTexture3D);
            break;
        default:
        {
            bool unvalid_resource_type = false;
            assert(unvalid_resource_type);
            (void)unvalid_resource_type;
            break;
        }
        }
    }
    mResourceMap.clear();
    R_UNLOCK;
}

void RSResourceManager::ClearMeshSrvs()
{
    M_LOCK;
    for (auto& meshSrv : mMeshSrvMap)
    {
        SAFE_RELEASE(meshSrv.second);
    }
    mMeshSrvMap.clear();
    M_UNLOCK;
}
