#include "ID_Common.h"

#include "InputDeviceGameController.h"

#include <SDL.h>

static constexpr float INPUT_DEADZONE = 0.24f * static_cast<float>(0x7FFF);

InputDeviceGameController::InputDeviceGameController(int JoystickIndex)
    : InputDeviceBase(INPUT_DEVICE_TYPE::GAMEPAD), Controller(nullptr),
      DeviceStatus({}), DeviceStatusBeforeThisPoll({}) {
  Controller = SDL_GameControllerOpen(JoystickIndex);

  KeyCodeToButton.insert({GP_LEFTBTN, SDL_CONTROLLER_BUTTON_X});
  KeyCodeToButton.insert({GP_BOTTOMBTN, SDL_CONTROLLER_BUTTON_A});
  KeyCodeToButton.insert({GP_RIGHTBTN, SDL_CONTROLLER_BUTTON_B});
  KeyCodeToButton.insert({GP_TOPBTN, SDL_CONTROLLER_BUTTON_Y});
  KeyCodeToButton.insert(
      {GP_LEFTFORESHDBTN, SDL_CONTROLLER_BUTTON_LEFTSHOULDER});
  KeyCodeToButton.insert(
      {GP_RIGHTFORESHDBTN, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER});
  KeyCodeToButton.insert({GP_LEFTMENUBTN, SDL_CONTROLLER_BUTTON_BACK});
  KeyCodeToButton.insert({GP_RIGHTMENUBTN, SDL_CONTROLLER_BUTTON_START});
  KeyCodeToButton.insert({GP_LEFTSTICKBTN, SDL_CONTROLLER_BUTTON_LEFTSTICK});
  KeyCodeToButton.insert({GP_RIGHTSTICKBTN, SDL_CONTROLLER_BUTTON_RIGHTSTICK});
  KeyCodeToButton.insert({GP_UPDIRBTN, SDL_CONTROLLER_BUTTON_DPAD_UP});
  KeyCodeToButton.insert({GP_RIGHTDIRBTN, SDL_CONTROLLER_BUTTON_DPAD_RIGHT});
  KeyCodeToButton.insert({GP_DOWNDIRBTN, SDL_CONTROLLER_BUTTON_DPAD_DOWN});
  KeyCodeToButton.insert({GP_LEFTDIRBTN, SDL_CONTROLLER_BUTTON_DPAD_LEFT});
}

InputDeviceGameController::~InputDeviceGameController() {
  if (Controller) {
    SDL_GameControllerClose(Controller);
    Controller = nullptr;
  }
}

INPUT_TYPE InputDeviceGameController::getInputType() {
  return INPUT_TYPE::GAMECONTROLLER;
}

bool InputDeviceGameController::pollDeviceStatus() {
  DeviceStatusBeforeThisPoll = DeviceStatus;

  if (!Controller || !SDL_GameControllerGetAttached(Controller)) {
    DeviceStatus = {};
    return false;
  }

  PAD_STATUS Status = {};
  for (int I = 0; I < SDL_CONTROLLER_BUTTON_MAX; I++) {
    if (SDL_GameControllerGetButton(Controller,
                                    static_cast<SDL_GameControllerButton>(I))) {
      Status.Buttons |= (1u << I);
    }
  }
  Status.LeftX = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTX);
  Status.LeftY = SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_LEFTY);
  Status.RightX =
      SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_RIGHTX);
  Status.RightY =
      SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_RIGHTY);
  Status.LeftTrigger =
      SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
  Status.RightTrigger =
      SDL_GameControllerGetAxis(Controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
  DeviceStatus = Status;

  return true;
}

bool InputDeviceGameController::hasButton(const PAD_STATUS &Status,
                                          int Button) {
  return (Status.Buttons & (1u << Button)) ? true : false;
}

bool InputDeviceGameController::isPushed(const PAD_STATUS &Status,
                                         unsigned int KeyCode) {
  switch (KeyCode) {
  case GP_LEFTBACKSHDBTN:
    return Status.LeftTrigger > 0;

  case GP_RIGHTBACKSHDBTN:
    return Status.RightTrigger > 0;

  case GP_UPRIGHTDIRBTN:
    return hasButton(Status, SDL_CONTROLLER_BUTTON_DPAD_UP) &&
           hasButton(Status, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

  case GP_DOWNRIGHTDIRBTN:
    return hasButton(Status, SDL_CONTROLLER_BUTTON_DPAD_DOWN) &&
           hasButton(Status, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

  case GP_DOWNLEFTDIRBTN:
    return hasButton(Status, SDL_CONTROLLER_BUTTON_DPAD_DOWN) &&
           hasButton(Status, SDL_CONTROLLER_BUTTON_DPAD_LEFT);

  case GP_UPLEFTDIRBTN:
    return hasButton(Status, SDL_CONTROLLER_BUTTON_DPAD_UP) &&
           hasButton(Status, SDL_CONTROLLER_BUTTON_DPAD_LEFT);

  default:
    break;
  }

  auto Found = KeyCodeToButton.find(KeyCode);
  if (Found == KeyCodeToButton.end()) {
    return false;
  }

  return hasButton(Status, Found->second);
}

bool InputDeviceGameController::isKeyBeingPushed(unsigned int KeyCode) {
  return isPushed(DeviceStatus, KeyCode);
}

bool InputDeviceGameController::hasKeyPushedInLastFrame(unsigned int KeyCode) {
  return isPushed(DeviceStatusBeforeThisPoll, KeyCode);
}

static int32_t normalizeStick(int16_t Value) {
  float FValue = static_cast<float>(Value);
  if (FValue < INPUT_DEADZONE && FValue > -INPUT_DEADZONE) {
    FValue = 0.f;
  }
  if (FValue < -static_cast<float>(0x7FFF)) {
    FValue = -static_cast<float>(0x7FFF);
  }
  return static_cast<int32_t>(FValue / static_cast<float>(0x7FFF) * 1000.f);
}

static int32_t normalizeTrigger(int16_t Value) {
  return static_cast<int32_t>(static_cast<float>(Value) /
                              static_cast<float>(0x7FFF) * 1000.f);
}

// SDL reports the y axes pointing down, same as the original output of
// -XINPUT_GAMEPAD::sThumbLY, so no negation here.
int32_t InputDeviceGameController::getXPositionOffset() {
  return normalizeStick(DeviceStatus.LeftX);
}

int32_t InputDeviceGameController::getYPositionOffset() {
  return normalizeStick(DeviceStatus.LeftY);
}

int32_t InputDeviceGameController::getZPositionOffset() {
  return normalizeTrigger(DeviceStatus.LeftTrigger);
}

int32_t InputDeviceGameController::getXRotationOffset() {
  return normalizeStick(DeviceStatus.RightX);
}

int32_t InputDeviceGameController::getYRotationOffset() {
  return normalizeStick(DeviceStatus.RightY);
}

int32_t InputDeviceGameController::getZRotationOffset() {
  return normalizeTrigger(DeviceStatus.RightTrigger);
}
