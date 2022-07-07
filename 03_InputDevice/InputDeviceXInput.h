#pragma once

#include "InputDeviceBase.h"

#include <map>

class InputDeviceXInput : public InputDeviceBase {
private:
  XINPUT_STATE *DeviceStatus;
  XINPUT_STATE *DeviceStatusBeforeThisPoll;

  std::map<UINT, WORD> XIKeyCodeToNorm;

public:
  InputDeviceXInput(DWORD XIDeviceHandle);
  ~InputDeviceXInput();

  virtual INPUT_TYPE getInputType();
  virtual HRESULT pollDeviceStatus();
  virtual const LPVOID getDeviceStatus();

  virtual bool isKeyBeingPushed(UINT KeyCode);
  virtual bool hasKeyPushedInLastFrame(UINT KeyCode);
  virtual LONG getXPositionOffset();
  virtual LONG getYPositionOffset();
  virtual LONG getZPositionOffset();
  virtual LONG getXRotationOffset();
  virtual LONG getYRotationOffset();
  virtual LONG getZRotationOffset();
};
