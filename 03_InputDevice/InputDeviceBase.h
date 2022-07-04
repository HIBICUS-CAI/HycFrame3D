#pragma once

#include "ID_BasicMacro.h"

#include <Xinput.h>
#include <array>
#include <dinput.h>

enum class INPUT_TYPE { DIRECTINPUT, XINPUT, UNKNOWN };

enum class INPUT_DEVICE_TYPE { KEYBOARD, MOUSE, GAMEPAD };

struct DI_KEYBOARD_STATUS_MINE {
  std::array<UCHAR, MAX_KEYBOARDS_KEY_NUM> Status;
};

class InputDeviceBase {
public:
  LPDIRECTINPUTDEVICE8 DIDeviceHandle;

private:
  DWORD XIDeviceHandle;

  INPUT_DEVICE_TYPE DeviceType;

public:
  InputDeviceBase(INPUT_DEVICE_TYPE DeviceType,
                  DWORD XIDeviceHandle = MAX_INPUTDEVICE_NUM);

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
  pollDeviceStatus() = 0;

  virtual const LPVOID
  getDeviceStatus() = 0;

  virtual bool
  isKeyBeingPushed(UINT KeyCode) = 0;

  virtual bool
  hasKeyPushedInLastFrame(UINT KeyCode) = 0;

  virtual LONG
  getXPositionOffset() = 0;

  virtual LONG
  getYPositionOffset() = 0;

  virtual LONG
  getZPositionOffset() = 0;

  virtual LONG
  getXRotationOffset() = 0;

  virtual LONG
  getYRotationOffset() = 0;

  virtual LONG
  getZRotationOffset() = 0;
};
