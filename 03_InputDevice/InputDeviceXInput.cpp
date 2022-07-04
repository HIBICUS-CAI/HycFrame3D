#include "ID_Common.h"

#include "InputDeviceXInput.h"

#define INPUT_DEADZONE (0.24f * FLOAT(0x7FFF))

InputDeviceXInput::InputDeviceXInput(DWORD XIDeviceHandle)
    : InputDeviceBase(INPUT_DEVICE_TYPE::GAMEPAD, XIDeviceHandle),
      DeviceStatus(nullptr), DeviceStatusBeforeThisPoll(nullptr) {
  if (XIDeviceHandle < 4 && XIDeviceHandle >= 0) {
    DeviceStatus = new XINPUT_STATE();
    DeviceStatusBeforeThisPoll = new XINPUT_STATE();
  }

  XIKeyCodeToNorm.insert({GP_LEFTBTN, XINPUT_GAMEPAD_X});
  XIKeyCodeToNorm.insert({GP_BOTTOMBTN, XINPUT_GAMEPAD_A});
  XIKeyCodeToNorm.insert({GP_RIGHTBTN, XINPUT_GAMEPAD_B});
  XIKeyCodeToNorm.insert({GP_TOPBTN, XINPUT_GAMEPAD_Y});
  XIKeyCodeToNorm.insert({GP_LEFTFORESHDBTN, XINPUT_GAMEPAD_LEFT_SHOULDER});
  XIKeyCodeToNorm.insert({GP_RIGHTFORESHDBTN, XINPUT_GAMEPAD_RIGHT_SHOULDER});
  XIKeyCodeToNorm.insert({GP_LEFTMENUBTN, XINPUT_GAMEPAD_BACK});
  XIKeyCodeToNorm.insert({GP_RIGHTMENUBTN, XINPUT_GAMEPAD_START});
  XIKeyCodeToNorm.insert({GP_LEFTSTICKBTN, XINPUT_GAMEPAD_LEFT_THUMB});
  XIKeyCodeToNorm.insert({GP_RIGHTSTICKBTN, XINPUT_GAMEPAD_RIGHT_THUMB});
  XIKeyCodeToNorm.insert({GP_UPDIRBTN, XINPUT_GAMEPAD_DPAD_UP});
  XIKeyCodeToNorm.insert({GP_RIGHTDIRBTN, XINPUT_GAMEPAD_DPAD_RIGHT});
  XIKeyCodeToNorm.insert({GP_DOWNDIRBTN, XINPUT_GAMEPAD_DPAD_DOWN});
  XIKeyCodeToNorm.insert({GP_LEFTDIRBTN, XINPUT_GAMEPAD_DPAD_LEFT});
}

InputDeviceXInput::~InputDeviceXInput() {
  if (DeviceStatus) {
    delete DeviceStatus;
  }
  if (DeviceStatusBeforeThisPoll) {
    delete DeviceStatusBeforeThisPoll;
  }
}

INPUT_TYPE
InputDeviceXInput::getInputType() { return INPUT_TYPE::XINPUT; }

HRESULT
InputDeviceXInput::pollDeviceStatus() {
  if (DeviceStatus && DeviceStatusBeforeThisPoll) {
    *DeviceStatusBeforeThisPoll = *DeviceStatus;
  } else {
    return E_FAIL;
  }

  DWORD Result = XInputGetState(getXIDeviceHandle(), DeviceStatus);

  if (Result == ERROR_SUCCESS) {
    return S_OK;
  } else {
    return E_FAIL;
  }
}

const LPVOID
InputDeviceXInput::getDeviceStatus() {
  return static_cast<LPVOID>(DeviceStatus);
}

bool
InputDeviceXInput::isKeyBeingPushed(UINT KeyCode) {
  if (!DeviceStatus) {
    return false;
  }

  switch (KeyCode) {
  case GP_LEFTBACKSHDBTN:
    return (getZPositionOffset() > 0) ? true : false;

  case GP_RIGHTBACKSHDBTN:
    return (getZRotationOffset() > 0) ? true : false;

  case GP_UPRIGHTDIRBTN:
    return ((DeviceStatus->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) &&
            (DeviceStatus->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT))
               ? true
               : false;

  case GP_DOWNRIGHTDIRBTN:
    return ((DeviceStatus->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) &&
            (DeviceStatus->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT))
               ? true
               : false;

  case GP_DOWNLEFTDIRBTN:
    return ((DeviceStatus->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) &&
            (DeviceStatus->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT))
               ? true
               : false;

  case GP_UPLEFTDIRBTN:
    return ((DeviceStatus->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) &&
            (DeviceStatus->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT))
               ? true
               : false;

  default:
    break;
  }

  WORD XIKeyCode = 0;
  bool HasFound = false;
  for (auto &code : XIKeyCodeToNorm) {
    if (code.first == KeyCode) {
      XIKeyCode = code.second;
      HasFound = true;
      break;
    }
  }

  if (!HasFound) {
    return false;
  } else {
    return (DeviceStatus->Gamepad.wButtons & XIKeyCode) ? true : false;
  }
}

bool
InputDeviceXInput::hasKeyPushedInLastFrame(UINT KeyCode) {
  if (!DeviceStatusBeforeThisPoll) {
    return false;
  }

  switch (KeyCode) {
  case GP_LEFTBACKSHDBTN:
    return (DeviceStatusBeforeThisPoll->Gamepad.bLeftTrigger > 0) ? true
                                                                  : false;

  case GP_RIGHTBACKSHDBTN:
    return (DeviceStatusBeforeThisPoll->Gamepad.bRightTrigger > 0) ? true
                                                                   : false;

  case GP_UPRIGHTDIRBTN:
    return ((DeviceStatusBeforeThisPoll->Gamepad.wButtons &
             XINPUT_GAMEPAD_DPAD_UP) &&
            (DeviceStatusBeforeThisPoll->Gamepad.wButtons &
             XINPUT_GAMEPAD_DPAD_RIGHT))
               ? true
               : false;

  case GP_DOWNRIGHTDIRBTN:
    return ((DeviceStatusBeforeThisPoll->Gamepad.wButtons &
             XINPUT_GAMEPAD_DPAD_DOWN) &&
            (DeviceStatusBeforeThisPoll->Gamepad.wButtons &
             XINPUT_GAMEPAD_DPAD_RIGHT))
               ? true
               : false;

  case GP_DOWNLEFTDIRBTN:
    return ((DeviceStatusBeforeThisPoll->Gamepad.wButtons &
             XINPUT_GAMEPAD_DPAD_DOWN) &&
            (DeviceStatusBeforeThisPoll->Gamepad.wButtons &
             XINPUT_GAMEPAD_DPAD_LEFT))
               ? true
               : false;

  case GP_UPLEFTDIRBTN:
    return ((DeviceStatusBeforeThisPoll->Gamepad.wButtons &
             XINPUT_GAMEPAD_DPAD_UP) &&
            (DeviceStatusBeforeThisPoll->Gamepad.wButtons &
             XINPUT_GAMEPAD_DPAD_LEFT))
               ? true
               : false;

  default:
    break;
  }

  WORD XIKeyCode = 0;
  bool HasFound = false;
  for (auto &code : XIKeyCodeToNorm) {
    if (code.first == KeyCode) {
      XIKeyCode = code.second;
      HasFound = true;
      break;
    }
  }

  if (!HasFound) {
    return false;
  } else {
    return (DeviceStatusBeforeThisPoll->Gamepad.wButtons & XIKeyCode) ? true
                                                                      : false;
  }
}

LONG
InputDeviceXInput::getXPositionOffset() {
  if (!DeviceStatus) {
    return 0;
  }

  SHORT XOffset = DeviceStatus->Gamepad.sThumbLX;
  if (XOffset < INPUT_DEADZONE && XOffset > -INPUT_DEADZONE) {
    XOffset = 0;
  }

  return static_cast<LONG>(static_cast<FLOAT>(XOffset) /
                           static_cast<FLOAT>(0x7FFF) * 1000.f);
}

LONG
InputDeviceXInput::getYPositionOffset() {
  if (!DeviceStatus) {
    return 0;
  }

  SHORT YOffset = DeviceStatus->Gamepad.sThumbLY;
  if (YOffset < INPUT_DEADZONE && YOffset > -INPUT_DEADZONE) {
    YOffset = 0;
  }

  return -static_cast<LONG>(static_cast<FLOAT>(YOffset) /
                            static_cast<FLOAT>(0x7FFF) * 1000.f);
}

LONG
InputDeviceXInput::getZPositionOffset() {
  if (!DeviceStatus) {
    return 0;
  }

  return static_cast<LONG>(
      static_cast<FLOAT>(DeviceStatus->Gamepad.bLeftTrigger) /
      static_cast<FLOAT>(255) * 1000.f);
}

LONG
InputDeviceXInput::getXRotationOffset() {
  if (!DeviceStatus) {
    return 0;
  }

  SHORT XOffset = DeviceStatus->Gamepad.sThumbRX;
  if (XOffset < INPUT_DEADZONE && XOffset > -INPUT_DEADZONE) {
    XOffset = 0;
  }

  return static_cast<LONG>(static_cast<FLOAT>(XOffset) /
                           static_cast<FLOAT>(0x7FFF) * 1000.f);
}

LONG
InputDeviceXInput::getYRotationOffset() {
  if (!DeviceStatus) {
    return 0;
  }

  SHORT YOffset = DeviceStatus->Gamepad.sThumbRY;
  if (YOffset < INPUT_DEADZONE && YOffset > -INPUT_DEADZONE) {
    YOffset = 0;
  }

  return -static_cast<LONG>(static_cast<FLOAT>(YOffset) /
                            static_cast<FLOAT>(0x7FFF) * 1000.f);
}

LONG
InputDeviceXInput::getZRotationOffset() {
  if (!DeviceStatus) {
    return 0;
  }

  return static_cast<LONG>(
      static_cast<FLOAT>(DeviceStatus->Gamepad.bRightTrigger) /
      static_cast<FLOAT>(255) * 1000.f);
}
