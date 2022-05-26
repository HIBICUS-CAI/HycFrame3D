//---------------------------------------------------------------
// File: RSCamerasContainer.cpp
// Proj: RenderSystem_DX11
// Info: 对所有的摄像机进行引用及简单处理
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSCamerasContainer.h"
#include "RSRoot_DX11.h"
#include "RSCamera.h"

#define LOCK EnterCriticalSection(&mDataLock)
#define UNLOCK LeaveCriticalSection(&mDataLock)

RSCamerasContainer::RSCamerasContainer() :
    mRootPtr(nullptr), mCameraMap({}), mDataLock({})
{

}

RSCamerasContainer::~RSCamerasContainer()
{

}

bool RSCamerasContainer::StartUp(RSRoot_DX11* _root)
{
    if (!_root) { return false; }

    mRootPtr = _root;
    InitializeCriticalSection(&mDataLock);

    return true;
}

void RSCamerasContainer::CleanAndStop()
{
    for (auto& cam : mCameraMap)
    {
        delete cam.second;
    }
    mCameraMap.clear();
    DeleteCriticalSection(&mDataLock);
}

RSCamera* RSCamerasContainer::CreateRSCamera(
    std::string& _name, CAM_INFO* _info)
{
    if (!_info) { return nullptr; }

    LOCK;
    if (mCameraMap.find(_name) == mCameraMap.end())
    {
        RSCamera* cam = new RSCamera(_info);
        mCameraMap.insert({ _name,cam });
    }
    auto cam = mCameraMap[_name];
    UNLOCK;

    return cam;
}

RSCamera* RSCamerasContainer::GetRSCamera(std::string& _name)
{
    LOCK;
    auto found = mCameraMap.find(_name);
    if (found != mCameraMap.end())
    {
        auto cam = found->second;
        UNLOCK;
        return cam;
    }
    else
    {
        UNLOCK;
        return nullptr;
    }
}

RS_CAM_INFO* RSCamerasContainer::GetRSCameraInfo(
    std::string& _name)
{
    LOCK;
    auto found = mCameraMap.find(_name);
    if (found != mCameraMap.end())
    {
        auto cam = found->second;
        UNLOCK;
        return cam->GetRSCameraInfo();
    }
    else
    {
        UNLOCK;
        return nullptr;
    }
}

void RSCamerasContainer::DeleteRSCamera(std::string& _name)
{
    LOCK;
    auto found = mCameraMap.find(_name);
    if (found != mCameraMap.end())
    {
        delete found->second;
        mCameraMap.erase(found);
    }
    UNLOCK;
}
