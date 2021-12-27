#include "SystemExecutive.h"
#include "SceneNode.h"
#include "SceneManager.h"
#include "ObjectContainer.h"
#include "System.h"
#include "InputSystem.h"
#include "RenderSystem.h"
#include "InstanceSystem.h"
#include "InteractSystem.h"
#include "CollisionSystem.h"

SystemExecutive::SystemExecutive() :
    mSceneManagerPtr(nullptr), mSystemsVec({}), mCurrentSceneNode(nullptr) {}

SystemExecutive::~SystemExecutive() {}

bool SystemExecutive::StartUp(SceneManager* _sceneManager)
{
    if (!_sceneManager)
    {
        P_LOG(LOG_ERROR, "invalid scene manager pointer\n");
        return false;
    }

    mSceneManagerPtr = _sceneManager;
    mCurrentSceneNode = _sceneManager->GetCurrentSceneNode();

    System* sys = nullptr;

    sys = new InputSystem(this); if (!sys) { return false; }
    mSystemsVec.push_back(sys);
    sys = new CollisionSystem(this); if (!sys) { return false; }
    mSystemsVec.push_back(sys);
    sys = new InteractSystem(this); if (!sys) { return false; }
    mSystemsVec.push_back(sys);
    sys = new InstanceSystem(this); if (!sys) { return false; }
    mSystemsVec.push_back(sys);
    sys = new RenderSystem(this); if (!sys) { return false; }
    mSystemsVec.push_back(sys);

    return InitAllSystem();
}

void SystemExecutive::CleanAndStop()
{
    for (auto& sys : mSystemsVec)
    {
        sys->Destory();
        delete sys;
    }
    mSystemsVec.clear();
}

void SystemExecutive::RunAllSystems(Timer& _timer)
{
    CheckCurrentScene();

    mCurrentSceneNode->GetObjectContainer()->DeleteAllDeadObjects();
    mCurrentSceneNode->GetObjectContainer()->InitAllNewObjects();

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
    SceneNode* nextNode = mSceneManagerPtr->GetCurrentSceneNode();
    SceneNode* thisNode = mCurrentSceneNode;

    if (nextNode != thisNode)
    {
        mCurrentSceneNode = nextNode;
        bool next_scene_init = InitAllSystem();
#ifdef _DEBUG
        assert(next_scene_init);
#endif // _DEBUG
    }
}
