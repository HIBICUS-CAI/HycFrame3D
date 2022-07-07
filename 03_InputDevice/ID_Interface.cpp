#include "ID_Common.h"

#include "ID_Interface.h"

#include "WM_Interface.h"

namespace input {

InputManager *G_InputManager = nullptr;

bool startUp() {
  G_InputManager = new InputManager(window::getWindowPtr());
  if (!G_InputManager) {
    return false;
  }

  HRESULT Hr = G_InputManager->createDirectInputMain();
  if (FAILED(Hr)) {
    delete G_InputManager;
    G_InputManager = nullptr;
    return false;
  }

  G_InputManager->enumAllInputDevices();

  return true;
}

void cleanAndStop() {
  if (G_InputManager) {
    G_InputManager->closeDirectInputMain();
    delete G_InputManager;
    G_InputManager = nullptr;
  }
}

InputManager *getInputManagerPtr() { return G_InputManager; }

bool pollDevices() {
  HRESULT Hr = G_InputManager->pollAllInputDevices();
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool isKeyDownInSingle(UINT KeyCode) {
  return G_InputManager->isThisKeyBeingPushedInSingle(KeyCode);
}

bool isKeyPushedInSingle(UINT KeyCode) {
  return G_InputManager->isThisKeyHasBeenPushedInSingle(KeyCode);
}

STICK_OFFSET leftStickOffset(int GamepadIndex) {
  return G_InputManager->getGamePadLeftStickOffset(GamepadIndex);
}

STICK_OFFSET rightStickOffset(int GamepadIndex) {
  return G_InputManager->getGamePadRightStickOffset(GamepadIndex);
}

BACKSHD_OFFSET leftBackShdBtnOffset(int GamepadIndex) {
  return G_InputManager->getGamePadLeftBackShdBtnOffset(GamepadIndex);
}

BACKSHD_OFFSET rightBackShdBtnOffset(int GamepadIndex) {
  return G_InputManager->getGamePadRightBackShdBtnOffset(GamepadIndex);
}

MOUSE_OFFSET getMouseOffset() { return G_InputManager->getMouseOffset(); }

bool isMouseScrollingUp() { return G_InputManager->isMouseScrollingUp(); }

bool isMouseScrollingDown() { return G_InputManager->isMouseScrollingDown(); }

} // namespace input
