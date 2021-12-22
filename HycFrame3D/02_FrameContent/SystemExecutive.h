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

private:
    bool InitAllSystem();
    void CheckCurrentScene();

private:
    class SceneManager* mSceneManagerPtr;
    class SceneNode* mCurrentSceneNodePtr;

    std::vector<class System*> mSystemsVec;
};
