#include "InputSystem.h"

#include "AInputComponent.h"
#include "ComponentContainer.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "SystemExecutive.h"
#include "UButtonComponent.h"
#include "UInputComponent.h"

#include <ID_Interface.h>

InputSystem::InputSystem(SystemExecutive *SysExecutive)
    : System("input-system", SysExecutive), AInputArrayPtr(nullptr),
      UInputArrayPtr(nullptr) {}

InputSystem::~InputSystem() {}

bool InputSystem::init() {
#ifdef _DEBUG
  assert(getSystemExecutive());
#endif // _DEBUG

  AInputArrayPtr = static_cast<std::vector<AInputComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::A_INPUT));
  UInputArrayPtr = static_cast<std::vector<UInputComponent> *>(
      getSystemExecutive()
          ->getSceneManager()
          ->getCurrentSceneNode()
          ->getComponentContainer()
          ->getCompVecPtr(COMP_TYPE::U_INPUT));

  if (!(AInputArrayPtr && UInputArrayPtr)) {
    return false;
  }

  return true;
}

void InputSystem::run(const Timer &Timer) {
  input::pollDevices();

  bool Up = input::isKeyPushedInSingle(KB_UP);
  bool Down = input::isKeyPushedInSingle(KB_DOWN);
  bool Left = input::isKeyPushedInSingle(KB_LEFT);
  bool Right = input::isKeyPushedInSingle(KB_RIGHT);
  bool GpUp = input::isKeyPushedInSingle(GP_UPDIRBTN);
  bool GpDown = input::isKeyPushedInSingle(GP_DOWNDIRBTN);
  bool GpLeft = input::isKeyPushedInSingle(GP_LEFTDIRBTN);
  bool GpRight = input::isKeyPushedInSingle(GP_RIGHTDIRBTN);
  bool Click = input::isKeyPushedInSingle(M_LEFTBTN);
  if (Up || Down || Left || Right || GpUp || GpDown || GpLeft || GpRight) {
    UButtonComponent::setShouldUseMouse(false);
  } else if (Click) {
    UButtonComponent::setShouldUseMouse(true);
  }

  for (auto &Aic : *AInputArrayPtr) {
    if (Aic.getCompStatus() == STATUS::ACTIVE) {
      Aic.update(Timer);
    }
  }

  for (auto &Uic : *UInputArrayPtr) {
    if (Uic.getCompStatus() == STATUS::ACTIVE) {
      Uic.update(Timer);
    }
  }

  // TEMP----------------------------------
  if (input::isKeyDownInSingle(KB_RALT) &&
      input::isKeyDownInSingle(KB_BACKSPACE)) {
    PostQuitMessage(0);
  }
  // TEMP----------------------------------
}

void InputSystem::destory() {}
