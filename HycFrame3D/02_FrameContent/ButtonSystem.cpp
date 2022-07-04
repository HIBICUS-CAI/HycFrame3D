#include "ButtonSystem.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "UButtonComponent.h"
#include "WM_Interface.h"
#include "ID_Interface.h"

static HWND g_WndHandle = {};
static float g_WndWidth = 0.f;
static float g_WndHeight = 0.f;
static const float g_ScreenWidth = 1280.f;
static const float g_ScreenHeight = 720.f;

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
    RECT wndRect = {};
    g_WndHandle = window::getWindowPtr()->getWndHandle();
    GetClientRect(g_WndHandle, &wndRect);
    g_WndWidth = (float)(wndRect.right - wndRect.left);
    g_WndHeight = (float)(wndRect.bottom - wndRect.top);

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
    POINT p = {};
    GetCursorPos(&p);
    ScreenToClient(g_WndHandle, &p);
    float cursorX = ((float)(p.x) / g_WndWidth - 0.5f) * g_ScreenWidth;
    float cursorY = -((float)(p.y) / g_WndHeight - 0.5f) * g_ScreenHeight;
#ifdef _DEBUG
    cursorX = (cursorX < -g_ScreenWidth / 2.f) ?
        -g_ScreenWidth / 2.f : cursorX;
    cursorX = (cursorX > g_ScreenWidth / 2.f) ?
        g_ScreenWidth / 2.f : cursorX;
    cursorY = (cursorY < -g_ScreenHeight / 2.f) ?
        -g_ScreenHeight / 2.f : cursorY;
    cursorY = (cursorY > g_ScreenHeight / 2.f) ?
        g_ScreenHeight / 2.f : cursorY;
#endif // _DEBUG
    UButtonComponent::SetScreenSpaceCursorPos(cursorX, cursorY);

    auto mouseOffset = InputInterface::GetMouseOffset();
    if (mouseOffset.x || mouseOffset.y)
    {
        UButtonComponent::SetShouldUseMouse(true);
    }

    for (auto& ubc : *mUBtnVecPtr)
    {
        if (ubc.GetCompStatus() == STATUS::ACTIVE) { ubc.Update(_timer); }
    }
}

void ButtonSystem::Destory()
{

}
