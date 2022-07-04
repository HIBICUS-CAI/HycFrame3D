#pragma once

#include "InputDeviceBase.h"

class InputDeviceDirectInput : public InputDeviceBase {
private:
  LPVOID DeviceStatus;
  WORD DeviceStatusSize;
  std::array<bool, GP_UPLEFTDIRBTN + 0x01> ButtonsStatusBeforeThisPoll;

public:
  InputDeviceDirectInput(INPUT_DEVICE_TYPE DeviceType);

  ~InputDeviceDirectInput();

  virtual INPUT_TYPE
  getInputType();

  virtual HRESULT
  pollDeviceStatus();

  virtual const LPVOID
  getDeviceStatus();

  virtual bool
  isKeyBeingPushed(UINT KeyCode);

  virtual bool
  hasKeyPushedInLastFrame(UINT KeyCode);

  virtual LONG
  getXPositionOffset();

  virtual LONG
  getYPositionOffset();

  virtual LONG
  getZPositionOffset();

  virtual LONG
  getXRotationOffset();

  virtual LONG
  getYRotationOffset();

  virtual LONG
  getZRotationOffset();
};
