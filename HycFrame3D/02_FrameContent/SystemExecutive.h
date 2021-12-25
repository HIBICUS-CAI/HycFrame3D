#pragma once

#include "Hyc3DCommon.h"
#include <vector>

class SystemExecutive
{
public:
    SystemExecutive();
    ~SystemExecutive();

    bool StartUp(class SceneManager* _sceneManager);
    void CleanAndStop();
    void RunAllSystems(Timer& _timer);

    class SceneManager* GetSceneManager() const;

private:
    bool InitAllSystem();
    void CheckCurrentScene();

private:
    class SceneManager* mSceneManagerPtr;

    std::vector<class System*> mSystemsVec;
};
