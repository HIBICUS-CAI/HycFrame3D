#include "ButtonSystem.h"

#include "ComponentContainer.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SystemExecutive.h"
#include "UButtonComponent.h"

#include <ID_Interface.h>
#include <WM_Interface.h>

static float G_WndWidth = 0.f;
static float G_WndHeight = 0.f;
static const float G_ScreenWidth = 1280.f;
static const float G_ScreenHeight = 720.f;

ButtonSystem::ButtonSystem(SystemExecutive *SysExecutive)
    : System("button-system", SysExecutive), UBtnArrayPtr(nullptr) {}

ButtonSystem::~ButtonSystem() {}

bool ButtonSystem::init() {
  int ClientWidth = 0;
  int ClientHeight = 0;
  window::getWindowPtr()->getClientSize(ClientWidth, ClientHeight);
  G_WndWidth = static_cast<float>(ClientWidth);
  G_WndHeight = static_cast<float>(ClientHeight);

#ifdef _DEBUG
  assert(getSystemExecutive());
#endif // _DEBUG

  UBtnArrayPtr = static_cast<std::vector<UButtonComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::U_BUTTON));

  if (!(UBtnArrayPtr)) {
    return false;
  }

  return true;
}

void ButtonSystem::run(const Timer &Timer) {
  int PX = 0;
  int PY = 0;
  window::getWindowPtr()->getCursorPosition(PX, PY);
  float CursorX = (static_cast<float>(PX) / G_WndWidth - 0.5f) * G_ScreenWidth;
  float CursorY =
      -(static_cast<float>(PY) / G_WndHeight - 0.5f) * G_ScreenHeight;
#ifdef _DEBUG
  CursorX = (CursorX < -G_ScreenWidth / 2.f) ? -G_ScreenWidth / 2.f : CursorX;
  CursorX = (CursorX > G_ScreenWidth / 2.f) ? G_ScreenWidth / 2.f : CursorX;
  CursorY = (CursorY < -G_ScreenHeight / 2.f) ? -G_ScreenHeight / 2.f : CursorY;
  CursorY = (CursorY > G_ScreenHeight / 2.f) ? G_ScreenHeight / 2.f : CursorY;
#endif // _DEBUG
  UButtonComponent::setScreenSpaceCursorPos(CursorX, CursorY);

  auto MouseOffset = input::getMouseOffset();
  if (MouseOffset.x || MouseOffset.y) {
    UButtonComponent::setShouldUseMouse(true);
  }

  for (auto &Ubc : *UBtnArrayPtr) {
    if (Ubc.getCompStatus() == STATUS::ACTIVE) {
      Ubc.update(Timer);
    }
  }
}

void ButtonSystem::destory() {}
