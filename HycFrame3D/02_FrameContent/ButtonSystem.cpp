#include "ButtonSystem.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "UButtonComponent.h"

ButtonSystem::ButtonSystem(SystemExecutive* _sysExecutive) :
    System("button-system", _sysExecutive),
    mUBtnVecPtr(nullptr)
{

}

ButtonSystem::~ButtonSystem()
{

}

bool ButtonSystem::Init()
{
#ifdef _DEBUG
    assert(GetSystemExecutive());
#endif // _DEBUG

    mUBtnVecPtr = (std::vector<UButtonComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::U_BUTTON);

    if (!(mUBtnVecPtr)) { return false; }

    return true;
}

void ButtonSystem::Run(Timer& _timer)
{
    for (auto& ubc : *mUBtnVecPtr)
    {
        ubc.Update(_timer);
    }
}

void ButtonSystem::Destory()
{

}
