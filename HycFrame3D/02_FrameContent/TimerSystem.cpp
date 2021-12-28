#include "TimerSystem.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "ATimerComponent.h"

TimerSystem::TimerSystem(SystemExecutive* _sysExecutive) :
    System("timer-system", _sysExecutive),
    mATimerVecPtr(nullptr)
{

}

TimerSystem::~TimerSystem()
{

}

bool TimerSystem::Init()
{
#ifdef _DEBUG
    assert(GetSystemExecutive());
#endif // _DEBUG

    mATimerVecPtr = (std::vector<ATimerComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::A_TIMER);

    if (!(mATimerVecPtr)) { return false; }

    return true;
}

void TimerSystem::Run(Timer& _timer)
{
    for (auto& atmc : *mATimerVecPtr)
    {
        atmc.Update(_timer);
    }
}

void TimerSystem::Destory()
{

}
