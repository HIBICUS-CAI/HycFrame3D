#include "InputSystem.h"
#include "SystemExecutive.h"
#include "ID_Interface.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "AInputComponent.h"
#include "UInputComponent.h"

InputSystem::InputSystem(SystemExecutive* _sysExecutive) :
    System("input-system", _sysExecutive),
    mAInputVecPtr(nullptr), mUInputVecPtr(nullptr)
{

}

InputSystem::~InputSystem()
{

}

bool InputSystem::Init()
{
#ifdef _DEBUG
    assert(GetSystemExecutive());
#endif // _DEBUG

    mAInputVecPtr = (std::vector<AInputComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::A_INPUT);
    mUInputVecPtr = (std::vector<UInputComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->GetCompVecPtr(COMP_TYPE::U_INPUT);

    if (!(mAInputVecPtr && mUInputVecPtr)) { return false; }

    return true;
}

void InputSystem::Run(Timer& _timer)
{
    InputInterface::PollDevices();

    for (auto& aic : *mAInputVecPtr)
    {
        if (aic.GetCompStatus() == STATUS::ACTIVE) { aic.Update(_timer); }
    }

    for (auto& uic : *mUInputVecPtr)
    {
        if (uic.GetCompStatus() == STATUS::ACTIVE) { uic.Update(_timer); }
    }

    // TEMP----------------------------------
    if (InputInterface::IsKeyDownInSingle(KB_1) &&
        InputInterface::IsKeyDownInSingle(KB_2))
    {
        PostQuitMessage(0);
    }
    // TEMP----------------------------------
}

void InputSystem::Destory()
{

}
