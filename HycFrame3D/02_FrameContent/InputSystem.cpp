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
    input::pollDevices();

    bool up = input::isKeyPushedInSingle(KB_UP);
    bool down = input::isKeyPushedInSingle(KB_DOWN);
    bool left = input::isKeyPushedInSingle(KB_LEFT);
    bool right = input::isKeyPushedInSingle(KB_RIGHT);
    bool gp_up = input::isKeyPushedInSingle(GP_UPDIRBTN);
    bool gp_down = input::isKeyPushedInSingle(GP_DOWNDIRBTN);
    bool gp_left = input::isKeyPushedInSingle(GP_LEFTDIRBTN);
    bool gp_right = input::isKeyPushedInSingle(GP_RIGHTDIRBTN);
    bool click = input::isKeyPushedInSingle(M_LEFTBTN);
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
    if (input::isKeyDownInSingle(KB_RALT) &&
        input::isKeyDownInSingle(KB_BACKSPACE))
    {
        PostQuitMessage(0);
    }
    // TEMP----------------------------------
}

void InputSystem::Destory()
{

}
