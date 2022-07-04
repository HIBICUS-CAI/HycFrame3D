#pragma once

#include "ID_BasicMacro.h"

#include "InputDeviceBase.h"
#include "WindowWIN32.h"

#include <Windows.h>
#include <Xinput.h>
#include <array>
#include <dinput.h>

using MOUSE_OFFSET = POINT;
using STICK_OFFSET = POINT;
using BACKSHD_OFFSET = LONG;

class InputManager {
private:
  HINSTANCE Instance;
  HWND WndHandle;

  static LPDIRECTINPUT8 DirectInputPtr;

  InputDeviceBase *KeyBoardPtr;
  InputDeviceBase *MousePtr;
  std::array<InputDeviceBase *, MAX_INPUTDEVICE_NUM> GamePadPtrs;

public:
  InputManager(WindowWIN32 *Wnd);

  HRESULT
  createDirectInputMain();

  void
  closeDirectInputMain();

  void
  enumAllInputDevices();

  HRESULT
  pollAllInputDevices();

  bool
  isThisKeyBeingPushedInSingle(UINT KeyCode);

  bool
  isThisKeyHasBeenPushedInSingle(UINT KeyCode);

  STICK_OFFSET
  getGamePadLeftStickOffset(int GamepadIndex = 0);

  STICK_OFFSET
  getGamePadRightStickOffset(int GamepadIndex = 0);

  BACKSHD_OFFSET
  getGamePadLeftBackShdBtnOffset(int GamepadIndex = 0);

  BACKSHD_OFFSET
  getGamePadRightBackShdBtnOffset(int GamepadIndex = 0);

  MOUSE_OFFSET
  getMouseOffset();

  bool
  isMouseScrollingUp();

  bool
  isMouseScrollingDown();

  InputDeviceBase *
  getKeyBoard();

  InputDeviceBase *
  getMouse();

  InputDeviceBase *
  getGamePadByIndex(int Index = 0);

private:
  static BOOL CALLBACK
  enumGamePadCallBack(const DIDEVICEINSTANCE *DIDeviceInst, VOID *ContextPtr);

  static BOOL CALLBACK
  diEnumGamePadObjCallBack(const DIDEVICEOBJECTINSTANCE *DIDeviceObjInst,
                           VOID *ContextPtr);
};
