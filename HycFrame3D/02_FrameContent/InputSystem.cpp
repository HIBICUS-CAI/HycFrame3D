#include "InputSystem.h"
#include "SystemExecutive.h"
#include "ID_Interface.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "AInputComponent.h"
#include "UInputComponent.h"
#include "UButtonComponent.h"

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

    bool up = InputInterface::IsKeyPushedInSingle(KB_UP);
    bool down = InputInterface::IsKeyPushedInSingle(KB_DOWN);
    bool left = InputInterface::IsKeyPushedInSingle(KB_LEFT);
    bool right = InputInterface::IsKeyPushedInSingle(KB_RIGHT);
    bool gp_up = InputInterface::IsKeyPushedInSingle(GP_UPDIRBTN);
    bool gp_down = InputInterface::IsKeyPushedInSingle(GP_DOWNDIRBTN);
    bool gp_left = InputInterface::IsKeyPushedInSingle(GP_LEFTDIRBTN);
    bool gp_right = InputInterface::IsKeyPushedInSingle(GP_RIGHTDIRBTN);
    bool click = InputInterface::IsKeyPushedInSingle(M_LEFTBTN);
    if (up || down || left || right ||
        gp_up || gp_down || gp_left || gp_right)
    {
        UButtonComponent::SetShouldUseMouse(false);
    }
    else if (click)
    {
        UButtonComponent::SetShouldUseMouse(true);
    }

    for (auto& aic : *mAInputVecPtr)
    {
        if (aic.GetCompStatus() == STATUS::ACTIVE) { aic.Update(_timer); }
    }

    for (auto& uic : *mUInputVecPtr)
    {
        if (uic.GetCompStatus() == STATUS::ACTIVE) { uic.Update(_timer); }
    }

    // TEMP----------------------------------
    if (InputInterface::IsKeyDownInSingle(KB_LALT) &&
        InputInterface::IsKeyDownInSingle(KB_BACKSPACE))
    {
        PostQuitMessage(0);
    }
    // TEMP----------------------------------
}

void InputSystem::Destory()
{

}
