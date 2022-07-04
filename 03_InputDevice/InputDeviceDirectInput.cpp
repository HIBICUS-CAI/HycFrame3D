#include "ID_Common.h"

#include "InputDeviceDirectInput.h"

InputDeviceDirectInput::InputDeviceDirectInput(INPUT_DEVICE_TYPE DeviceType)
    : InputDeviceBase(DeviceType), DeviceStatus(nullptr), DeviceStatusSize(0),
      ButtonsStatusBeforeThisPoll({false}) {
  switch (DeviceType) {
  case INPUT_DEVICE_TYPE::KEYBOARD:
    DeviceStatus = new DI_KEYBOARD_STATUS_MINE();
    DeviceStatusSize = sizeof(DI_KEYBOARD_STATUS_MINE);
    break;
  case INPUT_DEVICE_TYPE::MOUSE:
    DeviceStatus = new DIMOUSESTATE2();
    DeviceStatusSize = sizeof(DIMOUSESTATE2);
    break;
  case INPUT_DEVICE_TYPE::GAMEPAD:
    DeviceStatus = new DIJOYSTATE2();
    DeviceStatusSize = sizeof(DIJOYSTATE2);
    for (int I = 0; I < 4; I++) {
      DIJOYSTATE2 *Status = (DIJOYSTATE2 *)DeviceStatus;
      Status->rgdwPOV[I] = 1;
    }
    break;
  default:
    break;
  }
}

InputDeviceDirectInput::~InputDeviceDirectInput() {
  if (DeviceStatus) {
    auto Type = getInputDeviceType();
    switch (Type) {
    case INPUT_DEVICE_TYPE::KEYBOARD:
      delete static_cast<DI_KEYBOARD_STATUS_MINE *>(DeviceStatus);
      break;
    case INPUT_DEVICE_TYPE::MOUSE:
      delete static_cast<DIMOUSESTATE2 *>(DeviceStatus);
      break;
    case INPUT_DEVICE_TYPE::GAMEPAD:
      delete static_cast<DIJOYSTATE2 *>(DeviceStatus);
      break;
    default:
      break;
    }
  }

  if (DIDeviceHandle) {
    DIDeviceHandle->Unacquire();
    DIDeviceHandle->Release();
  }
}

INPUT_TYPE
InputDeviceDirectInput::getInputType() { return INPUT_TYPE::DIRECTINPUT; }

const LPVOID
InputDeviceDirectInput::getDeviceStatus() {
  return DeviceStatus;
}

HRESULT
InputDeviceDirectInput::pollDeviceStatus() {
  HRESULT Hr = S_OK;
  Hr = DIDeviceHandle->Poll();

  if (FAILED(Hr)) {
    Hr = DIDeviceHandle->Acquire();
    while (Hr == DIERR_INPUTLOST) {
      Hr = DIDeviceHandle->Acquire();
    }

    return S_OK;
  }

  // store this status to backup before being updated
  switch (getInputDeviceType()) {
  case INPUT_DEVICE_TYPE::KEYBOARD:
    for (UINT I = KB_ESCAPE, E = KB_MEDIASELECT + KB_ESCAPE; I < E; I++) {
      DI_KEYBOARD_STATUS_MINE *KStatus =
          (DI_KEYBOARD_STATUS_MINE *)DeviceStatus;
      if (KStatus->Status[I] & 0x80) {
        ButtonsStatusBeforeThisPoll[I] = true;
      } else {
        ButtonsStatusBeforeThisPoll[I] = false;
      }
    }
    break;
  case INPUT_DEVICE_TYPE::MOUSE:
    for (UINT I = 0x00; I < 0x08; I++) {
      DIMOUSESTATE2 *MStatus = (DIMOUSESTATE2 *)DeviceStatus;
      if (MStatus->rgbButtons[I] & 0x80) {
        ButtonsStatusBeforeThisPoll[static_cast<ULONGLONG>(I) +
                                    MOUSE_BTN_OFFSET] = true;
      } else {
        ButtonsStatusBeforeThisPoll[static_cast<ULONGLONG>(I) +
                                    MOUSE_BTN_OFFSET] = false;
      }
    }
    break;
  case INPUT_DEVICE_TYPE::GAMEPAD:
    for (UINT I = 0x00; I < 0x14; I++) {
      DIJOYSTATE2 *GStatus = (DIJOYSTATE2 *)DeviceStatus;
      if (I < 0x0C) {
        if (GStatus->rgbButtons[I] & 0x80) {
          ButtonsStatusBeforeThisPoll[static_cast<ULONGLONG>(I) +
                                      GAMEPAD_BTN_OFFSET] = true;
        } else {
          ButtonsStatusBeforeThisPoll[static_cast<ULONGLONG>(I) +
                                      GAMEPAD_BTN_OFFSET] = false;
        }
      } else {
        for (UINT I = 0x00; I < 0x08; I++) {
          ButtonsStatusBeforeThisPoll[static_cast<ULONGLONG>(I) + GP_UPDIRBTN] =
              false;
        }
        if (GStatus->rgdwPOV[0] == 1) {
          break;
        }
        UINT DirOffset = GStatus->rgdwPOV[0] / 4500;
        if (DirOffset > 7) {
          break;
        } else {
          ButtonsStatusBeforeThisPoll[static_cast<ULONGLONG>(DirOffset) +
                                      GP_UPDIRBTN] = true;
        }
      }
    }
    break;
  default:
    break;
  }

  Hr = DIDeviceHandle->GetDeviceState(DeviceStatusSize, DeviceStatus);
  if (FAILED(Hr)) {
    return Hr;
  }

  return Hr;
}

bool
InputDeviceDirectInput::hasKeyPushedInLastFrame(UINT KeyCode) {
  return ButtonsStatusBeforeThisPoll[KeyCode];
}

bool
InputDeviceDirectInput::isKeyBeingPushed(UINT KeyCode) {
  switch (getInputDeviceType()) {
  case INPUT_DEVICE_TYPE::KEYBOARD:
    if (KeyCode < 0x01 || KeyCode > 0xED) {
      return false;
    } else {
      DI_KEYBOARD_STATUS_MINE *KStatus =
          (DI_KEYBOARD_STATUS_MINE *)DeviceStatus;
      if (KStatus->Status[KeyCode] & 0x80) {
        return true;
      } else {
        return false;
      }
    }
  case INPUT_DEVICE_TYPE::MOUSE:
    if (KeyCode < MOUSE_BTN_OFFSET || KeyCode > MOUSE_BTN_OFFSET + 0x07) {
      return false;
    } else {
      DIMOUSESTATE2 *MStatus = (DIMOUSESTATE2 *)DeviceStatus;
      if (MStatus->rgbButtons[KeyCode - MOUSE_BTN_OFFSET] & 0x80) {
        return true;
      } else {
        return false;
      }
    }
  case INPUT_DEVICE_TYPE::GAMEPAD:
    if (KeyCode < GAMEPAD_BTN_OFFSET || KeyCode > GAMEPAD_BTN_OFFSET + 0x13) {
      return false;
    } else {
      DIJOYSTATE2 *GStatus = (DIJOYSTATE2 *)DeviceStatus;
      if (KeyCode < GP_UPDIRBTN) {
        if (GStatus->rgbButtons[KeyCode - GAMEPAD_BTN_OFFSET] & 0x80) {
          return true;
        } else {
          return false;
        }
      } else {
        bool Result = false;

        switch (KeyCode) {
        case GP_UPDIRBTN:
          if (GStatus->rgdwPOV[0] == 0) {
            Result = true;
          }
          break;
        case GP_UPRIGHTDIRBTN:
          if (GStatus->rgdwPOV[0] == 4500) {
            Result = true;
          }
          break;
        case GP_RIGHTDIRBTN:
          if (GStatus->rgdwPOV[0] == 9000) {
            Result = true;
          }
          break;
        case GP_DOWNRIGHTDIRBTN:
          if (GStatus->rgdwPOV[0] == 13500) {
            Result = true;
          }
          break;
        case GP_DOWNDIRBTN:
          if (GStatus->rgdwPOV[0] == 18000) {
            Result = true;
          }
          break;
        case GP_DOWNLEFTDIRBTN:
          if (GStatus->rgdwPOV[0] == 22500) {
            Result = true;
          }
          break;
        case GP_LEFTDIRBTN:
          if (GStatus->rgdwPOV[0] == 27000) {
            Result = true;
          }
          break;
        case GP_UPLEFTDIRBTN:
          if (GStatus->rgdwPOV[0] == 31500) {
            Result = true;
          }
          break;
        default:
          break;
        }

        return Result;
      }
    }
    break;
  default:
    break;
  }
  //--------------------
  return false;
}

LONG
InputDeviceDirectInput::getXPositionOffset() {
  LONG OffsetX = 0;

  switch (getInputDeviceType()) {
  case INPUT_DEVICE_TYPE::MOUSE: {
    DIMOUSESTATE2 *MStatus = (DIMOUSESTATE2 *)DeviceStatus;
    OffsetX = MStatus->lX;
    break;
  }
  case INPUT_DEVICE_TYPE::GAMEPAD: {
    DIJOYSTATE2 *GStatus = (DIJOYSTATE2 *)DeviceStatus;
    OffsetX = GStatus->lX;
    if (OffsetX <= 100 && OffsetX >= -100) {
      OffsetX = 0;
    }
    break;
  }
  default:
    break;
  }

  return OffsetX;
}

LONG
InputDeviceDirectInput::getYPositionOffset() {
  LONG OffsetY = 0;

  switch (getInputDeviceType()) {
  case INPUT_DEVICE_TYPE::MOUSE: {
    DIMOUSESTATE2 *MStatus = (DIMOUSESTATE2 *)DeviceStatus;
    OffsetY = MStatus->lY;
    break;
  }
  case INPUT_DEVICE_TYPE::GAMEPAD: {
    DIJOYSTATE2 *GStatus = (DIJOYSTATE2 *)DeviceStatus;
    OffsetY = GStatus->lY;
    if (OffsetY <= 100 && OffsetY >= -100) {
      OffsetY = 0;
    }
    break;
  }
  default:
    break;
  }

  return OffsetY;
}

LONG
InputDeviceDirectInput::getZPositionOffset() {
  LONG OffsetZ = 0;

  switch (getInputDeviceType()) {
  case INPUT_DEVICE_TYPE::MOUSE: {
    DIMOUSESTATE2 *MStatus = (DIMOUSESTATE2 *)DeviceStatus;
    OffsetZ = MStatus->lZ;
    break;
  }
  case INPUT_DEVICE_TYPE::GAMEPAD: {
    DIJOYSTATE2 *GStatus = (DIJOYSTATE2 *)DeviceStatus;
    OffsetZ = GStatus->lZ;
    if (OffsetZ <= 100 && OffsetZ >= -100) {
      OffsetZ = 0;
    }
    break;
  }
  default:
    break;
  }

  return OffsetZ;
}

LONG
InputDeviceDirectInput::getXRotationOffset() {
  LONG OffsetX = 0;

  if (getInputDeviceType() != INPUT_DEVICE_TYPE::GAMEPAD) {
    return OffsetX;
  }

  DIJOYSTATE2 *GStatus = (DIJOYSTATE2 *)DeviceStatus;
  OffsetX = (GStatus->lRx + 1000) / 2;

  return OffsetX;
}

LONG
InputDeviceDirectInput::getYRotationOffset() {
  LONG OffsetY = 0;

  if (getInputDeviceType() != INPUT_DEVICE_TYPE::GAMEPAD) {
    return OffsetY;
  }

  DIJOYSTATE2 *GStatus = (DIJOYSTATE2 *)DeviceStatus;
  OffsetY = (GStatus->lRy + 1000) / 2;

  return OffsetY;
}

LONG
InputDeviceDirectInput::getZRotationOffset() {
  LONG OffsetZ = 0;

  if (getInputDeviceType() != INPUT_DEVICE_TYPE::GAMEPAD) {
    return OffsetZ;
  }

  DIJOYSTATE2 *GStatus = (DIJOYSTATE2 *)DeviceStatus;
  OffsetZ = GStatus->lRz;
  if (OffsetZ <= 100 && OffsetZ >= -100) {
    OffsetZ = 0;
  }

  return OffsetZ;
}
