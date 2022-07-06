#include "InteractSystem.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "AInteractComponent.h"
#include "UInteractComponent.h"

InteractSystem::InteractSystem(SystemExecutive* _sysExecutive) :
    System("interact-system", _sysExecutive),
    mAInterVecPtr(nullptr), mUInterVecPtr(nullptr)
{

}

InteractSystem::~InteractSystem()
{

}

bool InteractSystem::Init()
{
#ifdef _DEBUG
    assert(GetSystemExecutive());
#endif // _DEBUG

    mAInterVecPtr = (std::vector<AInteractComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::A_INTERACT);
    mUInterVecPtr = (std::vector<UInteractComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::U_INTERACT);

    if (!(mAInterVecPtr && mUInterVecPtr)) { return false; }

    return true;
}

void InteractSystem::Run(Timer& _timer)
{
    for (auto& aitc : *mAInterVecPtr)
    {
        if (aitc.getCompStatus() == STATUS::ACTIVE) { aitc.update(_timer); }
    }

    for (auto& uitc : *mUInterVecPtr)
    {
        if (uitc.getCompStatus() == STATUS::ACTIVE) { uitc.update(_timer); }
    }
}

void InteractSystem::Destory()
{

}
