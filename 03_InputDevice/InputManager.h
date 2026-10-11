#pragma once

#include "ID_BasicMacro.h"

#include "InputDeviceBase.h"
#include "WindowSDL2.h"

#include <array>
#include <cstdint>

struct INPUT_OFFSET_POINT {
  int32_t x = 0;
  int32_t y = 0;
};

using MOUSE_OFFSET = INPUT_OFFSET_POINT;
using STICK_OFFSET = INPUT_OFFSET_POINT;
using BACKSHD_OFFSET = int32_t;

class InputManager {
private:
  WindowSDL2 *Wnd;

  InputDeviceBase *KeyBoardPtr;
  InputDeviceBase *MousePtr;
  std::array<InputDeviceBase *, MAX_INPUTDEVICE_NUM> GamePadPtrs;

public:
  InputManager(WindowSDL2 *Wnd);

  bool createInputMain();
  void closeInputMain();

  void enumAllInputDevices();
  bool pollAllInputDevices();

  bool isThisKeyBeingPushedInSingle(unsigned int KeyCode);
  bool isThisKeyHasBeenPushedInSingle(unsigned int KeyCode);

  STICK_OFFSET getGamePadLeftStickOffset(int GamepadIndex = 0);
  STICK_OFFSET getGamePadRightStickOffset(int GamepadIndex = 0);
  BACKSHD_OFFSET getGamePadLeftBackShdBtnOffset(int GamepadIndex = 0);
  BACKSHD_OFFSET getGamePadRightBackShdBtnOffset(int GamepadIndex = 0);
  MOUSE_OFFSET getMouseOffset();
  bool isMouseScrollingUp();
  bool isMouseScrollingDown();

  InputDeviceBase *getKeyBoard();
  InputDeviceBase *getMouse();
  InputDeviceBase *getGamePadByIndex(int Index = 0);
};
