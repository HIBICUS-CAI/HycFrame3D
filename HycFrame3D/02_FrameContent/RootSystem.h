#pragma once

#include "Hyc3DCommon.h"
#include <Windows.h>

class RootSystem
{
public:
    RootSystem();
    ~RootSystem();

    bool StartUp(HINSTANCE _hInstance, int _iCmdShow);
    void CleanAndStop();
    void RunGameLoop();

private:
    class SceneManager* mSceneManagerPtr;
    class ObjectFactory* mObjectFactoryPtr;
    class SystemExecutive* mSystemExecutivePtr;

    Timer mTimer;
};
