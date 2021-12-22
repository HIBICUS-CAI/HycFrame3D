#include "SystemExecutive.h"

SystemExecutive::SystemExecutive() :
    mSceneManagerPtr(nullptr), mCurrentSceneNodePtr(nullptr),
    mSystemsVec({}) {}

SystemExecutive::~SystemExecutive() {}

bool SystemExecutive::StartUp(SceneManager* _sceneManager)
{
    if (!_sceneManager)
    {
        P_LOG(LOG_ERROR, "invalid scene manager pointer\n");
        return false;
    }

    mSceneManagerPtr = _sceneManager;
    return true;
}

void SystemExecutive::CleanAndStop()
{

}

void SystemExecutive::RunAllSystems(Timer& _timer)
{

}

bool SystemExecutive::InitAllSystem()
{
    // TEMP--------------------
    return true;
    // TEMP--------------------
}

void SystemExecutive::CheckCurrentScene()
{

}
