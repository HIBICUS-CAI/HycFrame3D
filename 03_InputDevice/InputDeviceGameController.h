#pragma once

#include "InputDeviceBase.h"

#include <map>

struct _SDL_GameController;

// SDL game controller api normalizes pads into the xinput layout,
// so this device keeps the semantic of the original xinput device.
class InputDeviceGameController : public InputDeviceBase {
private:
  struct PAD_STATUS {
    uint32_t Buttons = 0;
    int16_t LeftX = 0;
    int16_t LeftY = 0;
    int16_t RightX = 0;
    int16_t RightY = 0;
    int16_t LeftTrigger = 0;
    int16_t RightTrigger = 0;
  };

  _SDL_GameController *Controller;
  PAD_STATUS DeviceStatus;
  PAD_STATUS DeviceStatusBeforeThisPoll;

  std::map<unsigned int, int> KeyCodeToButton;

public:
  InputDeviceGameController(int JoystickIndex);
  ~InputDeviceGameController();

  bool isValid() const { return Controller != nullptr; }

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
  bool isPushed(const PAD_STATUS &Status, unsigned int KeyCode);
  static bool hasButton(const PAD_STATUS &Status, int Button);
};
