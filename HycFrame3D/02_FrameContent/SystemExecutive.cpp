#include "SystemExecutive.h"
#include "SceneNode.h"
#include "System.h"
#include "InputSystem.h"

SystemExecutive::SystemExecutive() :
    mSceneManagerPtr(nullptr), mSystemsVec({}) {}

SystemExecutive::~SystemExecutive() {}

bool SystemExecutive::StartUp(SceneManager* _sceneManager)
{
    if (!_sceneManager)
    {
        P_LOG(LOG_ERROR, "invalid scene manager pointer\n");
        return false;
    }

    mSceneManagerPtr = _sceneManager;

    System* sys = nullptr;

    sys = new InputSystem(this); if (!sys) { return false; }
    mSystemsVec.push_back(sys);

    return InitAllSystem();
}

void SystemExecutive::CleanAndStop()
{

}

void SystemExecutive::RunAllSystems(Timer& _timer)
{
    CheckCurrentScene();

    for (auto& sys : mSystemsVec)
    {
        sys->Run(_timer);
    }
}

SceneManager* SystemExecutive::GetSceneManager() const
{
#ifdef _DEBUG
    assert(mSceneManagerPtr);
#endif // _DEBUG
    return mSceneManagerPtr;
}

bool SystemExecutive::InitAllSystem()
{
    for (auto& sys : mSystemsVec)
    {
        if (!sys->Init()) { return false; }
    }
    return true;
}

void SystemExecutive::CheckCurrentScene()
{

}
