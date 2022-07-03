﻿//---------------------------------------------------------------
// File: RSLightsContainer.cpp
// Proj: RenderSystem_DX11
// Info: 对所有的光源进行引用及简单处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSLightsContainer.h"
#include "RSCamerasContainer.h"
#include "RSRoot_DX11.h"
#include "RSLight.h"
#include <algorithm>

#define LOCK EnterCriticalSection(&mDataLock)
#define UNLOCK LeaveCriticalSection(&mDataLock)

RSLightsContainer::RSLightsContainer() :
    mRootPtr(nullptr), mLightMap({}), mAmbientLights({}),
    mCurrentAmbient({ 0.f,0.f,0.f,0.f }), mLights({}), mShadowLights({}),
    mShadowLightIndeices({}), mDataLock({})
{

}

RSLightsContainer::~RSLightsContainer()
{

}

bool RSLightsContainer::StartUp(RSRoot_DX11* _root)
{
    if (!_root) { return false; }

    mRootPtr = _root;
    InitializeCriticalSection(&mDataLock);

    return true;
}

void RSLightsContainer::CleanAndStop()
{
    for (auto& light : mLightMap)
    {
        delete light.second;
    }
    mLightMap.clear();
    mLights.clear();
    mShadowLights.clear();
    mShadowLightIndeices.clear();
    mAmbientLights.clear();
    DeleteCriticalSection(&mDataLock);
}

bool LightLessCompare(RSLight* a, RSLight* b)
{
    return (UINT)a->GetRSLightType() < (UINT)b->GetRSLightType();
}

RSLight* RSLightsContainer::CreateRSLight(
    std::string& _name, LIGHT_INFO* _info)
{
    if (!_info) { return nullptr; }

    LOCK;
    if (mLightMap.find(_name) == mLightMap.end())
    {
        UNLOCK;
        RSLight* light = new RSLight(_info);
        LOCK;
        mLightMap.insert({ _name,light });
        mLights.emplace_back(light);
        std::sort(mLights.begin(), mLights.end(),
            LightLessCompare);
        if (_info->mWithShadow)
        {
            mShadowLights.emplace_back(light);
            mShadowLightIndeices.emplace_back(
                (UINT)(mShadowLights.size() - 1));
        }
    }
    auto light = mLightMap[_name];
    UNLOCK;

    return light;
}

RSLight* RSLightsContainer::GetRSLight(std::string& _name)
{
    LOCK;
    auto found = mLightMap.find(_name);
    if (found != mLightMap.end())
    {
        auto light = found->second;
        UNLOCK;
        return light;
    }
    else
    {
        UNLOCK;
        return nullptr;
    }
}

RS_LIGHT_INFO* RSLightsContainer::GetRSLightInfo(
    std::string& _name)
{
    LOCK;
    auto found = mLightMap.find(_name);
    if (found != mLightMap.end())
    {
        auto light = found->second;
        UNLOCK;
        return light->GetRSLightInfo();
    }
    else
    {
        UNLOCK;
        return nullptr;
    }
}

void RSLightsContainer::DeleteRSLight(std::string& _name,
    bool _bloomDeleteByFrame)
{
    LOCK;
    auto found = mLightMap.find(_name);
    if (found != mLightMap.end())
    {
        for (auto i = mLights.begin(); i != mLights.end(); i++)
        {
            if ((*i) == found->second)
            {
                mLights.erase(i);
                break;
            }
        }

        for (auto i = mShadowLightIndeices.begin();
            i != mShadowLightIndeices.end(); i++)
        {
            if (mShadowLights[(*i)] == found->second)
            {
                for (auto& index : mShadowLightIndeices)
                {
                    if (index > (*i)) { --index; }
                }
                mShadowLightIndeices.erase(i);
                std::string camName = _name + "-light-cam";
                mRootPtr->CamerasContainer()->DeleteRSCamera(camName);
                break;
            }
        }
        for (auto i = mShadowLights.begin();
            i != mShadowLights.end(); i++)
        {
            if ((*i) == found->second)
            {
                mShadowLights.erase(i);
                break;
            }
        }

        /*int index = 0;
        for (auto i = mShadowLights.begin();
            i != mShadowLights.end(); i++)
        {
            if ((*i) == found->second)
            {
                mShadowLights.erase(i);
                break;
            }
            ++index;
        }
        for (auto i = mShadowLightIndeices.begin();
            i != mShadowLightIndeices.end(); i++)
        {
            if ((*i) == index)
            {
                mShadowLightIndeices.erase(i);
                std::string camName = _name + "-light-cam";
                mRootPtr->CamerasContainer()->DeleteRSCamera(camName);
                break;
            }
        }*/
        found->second->ReleaseLightBloom(_bloomDeleteByFrame);
        delete found->second;
        mLightMap.erase(found);
    }
    UNLOCK;
}

bool RSLightsContainer::CreateLightCameraFor(
    std::string& _name, CAM_INFO* _info)
{
    LOCK;
    auto found = mLightMap.find(_name);
    if (found != mLightMap.end())
    {
        auto light = found->second;
        UNLOCK;
        auto cam = light->CreateLightCamera(
            _name, _info, mRootPtr->CamerasContainer());
        if (cam)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        UNLOCK;
        return false;
    }
}

std::vector<RSLight*>* RSLightsContainer::GetLights()
{
    return &mLights;
}

std::vector<RSLight*>* RSLightsContainer::GetShadowLights()
{
    return &mShadowLights;
}

std::vector<INT>* RSLightsContainer::GetShadowLightIndeices()
{
    return &mShadowLightIndeices;
}

void RSLightsContainer::InsertAmbientLight(std::string&& _name,
    DirectX::XMFLOAT4&& _light)
{
    LOCK;
    auto found = mAmbientLights.find(_name);
    if (found == mAmbientLights.end())
    {
        mAmbientLights.insert({ _name,_light });
    }
    UNLOCK;
}

void RSLightsContainer::EraseAmbientLight(std::string&& _name)
{
    LOCK;
    auto found = mAmbientLights.find(_name);
    if (found != mAmbientLights.end())
    {
        mAmbientLights.erase(_name);
    }
    UNLOCK;
}

DirectX::XMFLOAT4& RSLightsContainer::GetAmbientLight(
    std::string& _name)
{
    LOCK;
    auto found = mAmbientLights.find(_name);
    static DirectX::XMFLOAT4 ambient = {};
    if (found != mAmbientLights.end())
    {
        DirectX::XMFLOAT4& refAmb = mAmbientLights[_name];
        UNLOCK;
        return refAmb;
    }
    UNLOCK;
    return ambient;
}

void RSLightsContainer::SetCurrentAmbientLight(std::string&& _name)
{
    mCurrentAmbient = GetAmbientLight(_name);
}

void RSLightsContainer::ForceCurrentAmbientLight(DirectX::XMFLOAT4&& _ambient)
{
    mCurrentAmbient = _ambient;
}

void RSLightsContainer::ForceCurrentAmbientLight(DirectX::XMFLOAT4& _ambient)
{
    mCurrentAmbient = _ambient;
}

DirectX::XMFLOAT4& RSLightsContainer::GetCurrentAmbientLight()
{
    return mCurrentAmbient;
}

void RSLightsContainer::UploadLightBloomDrawCall()
{
    LOCK;
    for (auto& light : mLights)
    {
        light->UploadLightDrawCall();
    }
    UNLOCK;
}

void RSLightsContainer::CreateLightBloom(std::string&& _name,
    RS_SUBMESH_DATA&& _meshData)
{
    GetRSLight(_name)->SetLightBloom(_meshData);
}

void RSLightsContainer::CreateLightBloom(std::string& _name,
    RS_SUBMESH_DATA&& _meshData)
{
    GetRSLight(_name)->SetLightBloom(_meshData);
}
