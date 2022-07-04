#pragma once

#include "ID_BasicMacro.h"

#include <Xinput.h>
#include <dinput.h>

enum class INPUT_TYPE { DIRECTINPUT, XINPUT, UNKNOWN };

enum class INPUT_DEVICE_TYPE { KEYBOARD, MOUSE, GAMEPAD };

struct DI_KEYBOARD_STATUS_MINE {
  UCHAR Status[MAX_KEYBOARDS_KEY_NUM];
};

class InputDeviceBase {
public:
  LPDIRECTINPUTDEVICE8 DIDeviceHandle;

private:
  DWORD XIDeviceHandle;

  INPUT_DEVICE_TYPE DeviceType;

public:
  InputDeviceBase(INPUT_DEVICE_TYPE DeviceType,
                  DWORD XiDeviceHandle = MAX_INPUTDEVICE_NUM);

  virtual ~InputDeviceBase();

  virtual INPUT_TYPE
  getInputType() = 0;

  INPUT_DEVICE_TYPE
  getInputDeviceType();

  LPDIRECTINPUTDEVICE8
  getDIDeviceHandle();

  DWORD
  getXIDeviceHandle();

  virtual HRESULT
  PollDeviceStatus() = 0;

  virtual const LPVOID
  getDeviceStatus() = 0;

  virtual const bool
  isKeyBeingPushed(UINT keyCode) = 0;

  virtual const bool
  hasKeyPushedInLastFrame(UINT keyCode) = 0;

  virtual const LONG
  getXPositionOffset() = 0;

  virtual const LONG
  getYPositionOffset() = 0;

  virtual const LONG
  getZPositionOffset() = 0;

  virtual const LONG
  getXRotationOffset() = 0;

  virtual const LONG
  getYRotationOffset() = 0;

  virtual const LONG
  getZRotationOffset() = 0;
};
