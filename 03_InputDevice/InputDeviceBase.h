#pragma once

#include "ID_BasicMacro.h"

#include <array>
#include <cstdint>

enum class INPUT_TYPE { SDL2, GAMECONTROLLER, UNKNOWN };

enum class INPUT_DEVICE_TYPE { KEYBOARD, MOUSE, GAMEPAD };

class InputDeviceBase {
private:
  INPUT_DEVICE_TYPE DeviceType;

public:
  InputDeviceBase(INPUT_DEVICE_TYPE DeviceType);
  virtual ~InputDeviceBase();

  virtual INPUT_TYPE getInputType() = 0;
  INPUT_DEVICE_TYPE getInputDeviceType();
  virtual bool pollDeviceStatus() = 0;

  virtual bool isKeyBeingPushed(unsigned int KeyCode) = 0;
  virtual bool hasKeyPushedInLastFrame(unsigned int KeyCode) = 0;
  virtual int32_t getXPositionOffset() = 0;
  virtual int32_t getYPositionOffset() = 0;
  virtual int32_t getZPositionOffset() = 0;
  virtual int32_t getXRotationOffset() = 0;
  virtual int32_t getYRotationOffset() = 0;
  virtual int32_t getZRotationOffset() = 0;
};
