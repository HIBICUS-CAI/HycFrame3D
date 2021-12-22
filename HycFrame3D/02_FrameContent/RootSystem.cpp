#include "Hyc3DCommon.h"
#include "WM_Interface.h"
#include "RootSystem.h"
#include "SystemExecutive.h"
#include "ObjectFactory.h"
#include "SceneManager.h"

RootSystem::RootSystem() :
    mSceneManagerPtr(nullptr), mObjectFactoryPtr(nullptr),
    mSystemExecutivePtr(nullptr), mTimer({})
{

}

RootSystem::~RootSystem() {}

bool RootSystem::StartUp(HINSTANCE _hInstance, int _iCmdShow)
{
    if (!WindowInterface::StartUp())
    {
        P_LOG(LOG_ERROR, "failed to start up windows system\n");
        return false;
    }
    if (!WindowInterface::CreateInitWindow("a game wnd dont know how to name",
#ifdef _DEBUG
        _hInstance, _iCmdShow, 1280, 720, false))
#else
        _hInstance, _iCmdShow, 1280, 720, true))
#endif // _DEBUG
    {
        P_LOG(LOG_ERROR, "failed to create window\n");
        return false;
    }
    if (!InputInterface::StartUp())
    {
        P_LOG(LOG_ERROR, "failed to start up input system\n");
        return false;
    }

    mSceneManagerPtr = new SceneManager();
    mObjectFactoryPtr = new ObjectFactory();
    mSystemExecutivePtr = new SystemExecutive();
    assert(mSceneManagerPtr && mObjectFactoryPtr && mSystemExecutivePtr);

    if (!mSystemExecutivePtr->StartUp(mSceneManagerPtr))
    {
        P_LOG(LOG_ERROR, "cannot init system executive correctly\n");
        return false;
    }
    if (!mObjectFactoryPtr->StartUp(mSceneManagerPtr))
    {
        P_LOG(LOG_ERROR, "cannot init system executive correctly\n");
        return false;
    }
    if (!mSceneManagerPtr->StartUp(mObjectFactoryPtr))
    {
        P_LOG(LOG_ERROR, "cannot init system executive correctly\n");
        return false;
    }

    return true;
}

void RootSystem::CleanAndStop()
{
    mSceneManagerPtr->CleanAndStop();
    mObjectFactoryPtr->CleanAndStop();
    mSystemExecutivePtr->CleanAndStop();
    delete mSceneManagerPtr; mSceneManagerPtr = nullptr;
    delete mObjectFactoryPtr; mObjectFactoryPtr = nullptr;
    delete mSystemExecutivePtr; mSystemExecutivePtr = nullptr;

    InputInterface::CleanAndStop();
    WindowInterface::CleanAndStop();
}

void RootSystem::RunGameLoop()
{
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            mTimer.TimeIn();

            InputInterface::PollDevices();

            // TEMP----------------------------------
            if (InputInterface::IsKeyDownInSingle(KB_1) &&
                InputInterface::IsKeyDownInSingle(KB_2))
            {
                PostQuitMessage(0);
            }
            // TEMP----------------------------------

            mTimer.TimeOut();
        }
    }
}
