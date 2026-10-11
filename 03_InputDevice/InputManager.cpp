#include "ID_Common.h"

#include "InputManager.h"

#include "InputDeviceGameController.h"
#include "InputDeviceSDL2.h"

#include <SDL.h>

InputManager::InputManager(WindowSDL2 *Wnd)
    : Wnd(Wnd), KeyBoardPtr(nullptr), MousePtr(nullptr),
      GamePadPtrs({nullptr}) {}

bool InputManager::createInputMain() {
  return SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0;
}

void InputManager::closeInputMain() {
  for (auto &GamePad : GamePadPtrs) {
    delete GamePad;
    GamePad = nullptr;
  }
  delete MousePtr;
  MousePtr = nullptr;
  delete KeyBoardPtr;
  KeyBoardPtr = nullptr;

  SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
}

void InputManager::enumAllInputDevices() {
  KeyBoardPtr = new InputDeviceSDL2(INPUT_DEVICE_TYPE::KEYBOARD);
  MousePtr = new InputDeviceSDL2(INPUT_DEVICE_TYPE::MOUSE);

  // gamepads which have a mapping in SDL's controller database
  int Index = 0;
  for (int I = 0, E = SDL_NumJoysticks(); I < E; I++) {
    if (Index >= MAX_INPUTDEVICE_NUM) {
      break;
    }
    if (!SDL_IsGameController(I)) {
      continue;
    }
    auto GamePad = new InputDeviceGameController(I);
    if (!GamePad->isValid()) {
      delete GamePad;
      continue;
    }
    GamePadPtrs[Index++] = GamePad;
  }
}

InputDeviceBase *InputManager::getKeyBoard() { return KeyBoardPtr; }

InputDeviceBase *InputManager::getMouse() { return MousePtr; }

InputDeviceBase *InputManager::getGamePadByIndex(int Index) {
  if (Index >= MAX_INPUTDEVICE_NUM) {
    return nullptr;
  }

  return GamePadPtrs[Index];
}

bool InputManager::pollAllInputDevices() {
  bool Result = true;

  if (KeyBoardPtr && !KeyBoardPtr->pollDeviceStatus()) {
    Result = false;
  }
  if (MousePtr && !MousePtr->pollDeviceStatus()) {
    Result = false;
  }
  for (auto &GamePad : GamePadPtrs) {
    if (GamePad && !GamePad->pollDeviceStatus()) {
      Result = false;
    }
  }

  return Result;
}

bool InputManager::isThisKeyBeingPushedInSingle(unsigned int KeyCode) {
  bool Keyboard = false;
  bool Mouse = false;
  bool Gamepad = false;
  if (KeyBoardPtr) {
    Keyboard = KeyBoardPtr->isKeyBeingPushed(KeyCode);
  }
  if (MousePtr) {
    Mouse = MousePtr->isKeyBeingPushed(KeyCode);
  }
  if (GamePadPtrs[0]) {
    Gamepad = GamePadPtrs[0]->isKeyBeingPushed(KeyCode);
  }

  return (Keyboard || Mouse || Gamepad);
}

bool InputManager::isThisKeyHasBeenPushedInSingle(unsigned int KeyCode) {
  bool Confirm1 = isThisKeyBeingPushedInSingle(KeyCode);

  if (!Confirm1) {
    return Confirm1;
  }

  bool KConfirm2 = false;
  bool MConfirm2 = false;
  bool GConfirm2 = false;

  if (KeyBoardPtr) {
    KConfirm2 = KeyBoardPtr->hasKeyPushedInLastFrame(KeyCode);
  }
  if (MousePtr) {
    MConfirm2 = MousePtr->hasKeyPushedInLastFrame(KeyCode);
  }
  if (GamePadPtrs[0]) {
    GConfirm2 = GamePadPtrs[0]->hasKeyPushedInLastFrame(KeyCode);
  }

  return (!(KConfirm2 || MConfirm2 || GConfirm2) && Confirm1);
}

MOUSE_OFFSET InputManager::getMouseOffset() {
  MOUSE_OFFSET Mo = {};
  Mo.x = MousePtr->getXPositionOffset();
  Mo.y = MousePtr->getYPositionOffset();

  return Mo;
}

bool InputManager::isMouseScrollingUp() {
  return MousePtr->getZPositionOffset() > 0;
}

bool InputManager::isMouseScrollingDown() {
  return MousePtr->getZPositionOffset() < 0;
}

STICK_OFFSET InputManager::getGamePadLeftStickOffset(int GamepadIndex) {
  STICK_OFFSET So = {};
  if (GamePadPtrs[GamepadIndex]) {
    So.x = GamePadPtrs[GamepadIndex]->getXPositionOffset();
    So.y = GamePadPtrs[GamepadIndex]->getYPositionOffset();
  }

  return So;
}

STICK_OFFSET InputManager::getGamePadRightStickOffset(int GamepadIndex) {
  STICK_OFFSET So = {};
  if (GamePadPtrs[GamepadIndex]) {
    So.x = GamePadPtrs[GamepadIndex]->getXRotationOffset();
    So.y = GamePadPtrs[GamepadIndex]->getYRotationOffset();
  }

  return So;
}

BACKSHD_OFFSET InputManager::getGamePadLeftBackShdBtnOffset(int GamepadIndex) {
  if (!GamePadPtrs[GamepadIndex]) {
    return 0;
  }

  return GamePadPtrs[GamepadIndex]->getZPositionOffset();
}

BACKSHD_OFFSET InputManager::getGamePadRightBackShdBtnOffset(int GamepadIndex) {
  if (!GamePadPtrs[GamepadIndex]) {
    return 0;
  }

  return GamePadPtrs[GamepadIndex]->getZRotationOffset();
}
