#pragma once

#include "InputDeviceBase.h"

union SDL_Event;

class InputDeviceSDL2 : public InputDeviceBase {
private:
  // indexed by the same code space as KB_* / M_* macros
  std::array<bool, GP_UPLEFTDIRBTN + 0x01> ButtonsStatus;
  std::array<bool, GP_UPLEFTDIRBTN + 0x01> ButtonsStatusBeforeThisPoll;
  int32_t MouseOffsetX;
  int32_t MouseOffsetY;
  int32_t MouseOffsetZ;

  static int32_t AccumulatedMouseX;
  static int32_t AccumulatedMouseY;
  static int32_t AccumulatedMouseZ;

public:
  InputDeviceSDL2(INPUT_DEVICE_TYPE DeviceType);

  ~InputDeviceSDL2();

  virtual INPUT_TYPE getInputType();
  virtual bool pollDeviceStatus();

  virtual bool isKeyBeingPushed(unsigned int KeyCode);
  virtual bool hasKeyPushedInLastFrame(unsigned int KeyCode);
  virtual int32_t getXPositionOffset();
  virtual int32_t getYPositionOffset();
  virtual int32_t getZPositionOffset();
  virtual int32_t getXRotationOffset();
  virtual int32_t getYRotationOffset();
  virtual int32_t getZRotationOffset();

private:
  static int mouseEventWatcher(void *UserData, SDL_Event *Event);
  void pollKeyboard();
  void pollMouse();
};
