#include "ID_Common.h"

#include "InputManager.h"

#include "InputDeviceDirectInput.h"
#include "InputDeviceXInput.h"

LPDIRECTINPUT8 InputManager::DirectInputPtr = nullptr;

InputManager::InputManager(WindowWIN32 *Wnd)
    : Instance(Wnd->getWndInstance()), WndHandle(Wnd->getWndHandle()),
      KeyBoardPtr(nullptr), MousePtr(nullptr), GamePadPtrs({nullptr}) {}

HRESULT
InputManager::createDirectInputMain() {
  return DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION,
                            IID_IDirectInput8, (VOID **)&DirectInputPtr,
                            nullptr);
}

void
InputManager::closeDirectInputMain() {
  for (int I = 0; I < MAX_INPUTDEVICE_NUM; I++) {
    if (GamePadPtrs[I] &&
        (GamePadPtrs[I]->getInputType() == INPUT_TYPE::DIRECTINPUT)) {
      GamePadPtrs[I]->getDIDeviceHandle()->Release();
    }
  }
  if (MousePtr) {
    MousePtr->DIDeviceHandle->Release();
  }
  if (KeyBoardPtr) {
    KeyBoardPtr->DIDeviceHandle->Release();
  }

  if (DirectInputPtr) {
    DirectInputPtr->Release();
  }
}

void
InputManager::enumAllInputDevices() {
  if (!DirectInputPtr) {
    return;
  }

  HRESULT Hr = S_OK;

  // keyboard
  KeyBoardPtr = new InputDeviceDirectInput(INPUT_DEVICE_TYPE::KEYBOARD);
  Hr = DirectInputPtr->CreateDevice(GUID_SysKeyboard,
                                    &(KeyBoardPtr->DIDeviceHandle), nullptr);
  if (FAILED(Hr)) {
    delete KeyBoardPtr;
    KeyBoardPtr = nullptr;
  } else {
    Hr = KeyBoardPtr->DIDeviceHandle->SetCooperativeLevel(
        WndHandle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(Hr)) {
      delete KeyBoardPtr;
      KeyBoardPtr = nullptr;
    } else {
      Hr = KeyBoardPtr->DIDeviceHandle->SetDataFormat(&c_dfDIKeyboard);
      if (FAILED(Hr)) {
        delete KeyBoardPtr;
        KeyBoardPtr = nullptr;
      }
    }
  }

  // mouse
  MousePtr = new InputDeviceDirectInput(INPUT_DEVICE_TYPE::MOUSE);
  Hr = DirectInputPtr->CreateDevice(GUID_SysMouse, &(MousePtr->DIDeviceHandle),
                                    nullptr);
  if (FAILED(Hr)) {
    delete MousePtr;
    MousePtr = nullptr;
  } else {
    Hr = MousePtr->DIDeviceHandle->SetCooperativeLevel(
        WndHandle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(Hr)) {
      delete MousePtr;
      MousePtr = nullptr;
    } else {
      Hr = MousePtr->DIDeviceHandle->SetDataFormat(&c_dfDIMouse2);
      if (FAILED(Hr)) {
        delete MousePtr;
        MousePtr = nullptr;
      }
    }
  }

  // gamepads
  // 1 - xinput
  // 2 - directinput
  Hr = DirectInputPtr->EnumDevices(DI8DEVCLASS_GAMECTRL, enumGamePadCallBack,
                                   &GamePadPtrs, DIEDFL_ATTACHEDONLY);
  if (FAILED(Hr)) {
    return;
  }
  for (int I = 0; I < MAX_INPUTDEVICE_NUM; I++) {
    if (GamePadPtrs[I] &&
        GamePadPtrs[I]->getInputType() == INPUT_TYPE::DIRECTINPUT) {
      if (FAILED(GamePadPtrs[I]->DIDeviceHandle->SetDataFormat(
              &c_dfDIJoystick2)) ||
          FAILED(GamePadPtrs[I]->DIDeviceHandle->SetCooperativeLevel(
              WndHandle, DISCL_EXCLUSIVE | DISCL_FOREGROUND)) ||
          FAILED(GamePadPtrs[I]->DIDeviceHandle->EnumObjects(
              diEnumGamePadObjCallBack, &GamePadPtrs[I], DIDFT_ALL))) {
        delete GamePadPtrs[I];
        for (int J = I; J < MAX_INPUTDEVICE_NUM - 1; J++) {
          GamePadPtrs[J] = GamePadPtrs[J + 1ULL];
        }
        GamePadPtrs[MAX_INPUTDEVICE_NUM - 1] = nullptr;
        --I;
      }
    }
  }
}

BOOL CALLBACK
InputManager::enumGamePadCallBack(const DIDEVICEINSTANCE *DIDeviceInst,
                                  VOID *ContextPtr) {
  auto GamePadPtrs = reinterpret_cast<InputDeviceBase **>(ContextPtr);

  int Index = -1;
  bool InXI = false;
  for (int I = 0; I < MAX_INPUTDEVICE_NUM; I++) {
    if (!GamePadPtrs[I]) {
      static int XIIndex = 0;
      XINPUT_STATE Xs;
      DWORD XResult = XInputGetState(I, &Xs);
      if (XResult == ERROR_SUCCESS) {
        GamePadPtrs[I] = new InputDeviceXInput(XIIndex);
        ++XIIndex;
        Index = I;
        InXI = true;
        break;
      }

      GamePadPtrs[I] = new InputDeviceDirectInput(INPUT_DEVICE_TYPE::GAMEPAD);
      Index = I;
      break;
    }
  }

  if (Index == -1) {
    return DIENUM_STOP;
  }

  if (!InXI) {
    DirectInputPtr->CreateDevice(DIDeviceInst->guidInstance,
                                 &(GamePadPtrs[Index]->DIDeviceHandle),
                                 nullptr);
  }

  return DIENUM_CONTINUE;
}

BOOL CALLBACK
InputManager::diEnumGamePadObjCallBack(
    const DIDEVICEOBJECTINSTANCE *DIDeviceObjInst,
    VOID *ContextPtr) {
  auto GamePad = reinterpret_cast<InputDeviceBase **>(ContextPtr);

  if (DIDeviceObjInst->dwType & DIDFT_AXIS) {
    DIPROPRANGE DIprg = {};
    DIprg.diph.dwSize = sizeof(DIPROPRANGE);
    DIprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    DIprg.diph.dwHow = DIPH_BYID;
    DIprg.diph.dwObj = DIDeviceObjInst->dwType;
    DIprg.lMin = -1000;
    DIprg.lMax = +1000;

    // Set the range for the axis
    if (FAILED((*GamePad)->DIDeviceHandle->SetProperty(DIPROP_RANGE,
                                                       &DIprg.diph))) {
      return DIENUM_STOP;
    }
  }

  return DIENUM_CONTINUE;
}

InputDeviceBase *
InputManager::getKeyBoard() {
  return KeyBoardPtr;
}

InputDeviceBase *
InputManager::getMouse() {
  return MousePtr;
}

InputDeviceBase *
InputManager::getGamePadByIndex(int Index) {
  if (Index >= MAX_INPUTDEVICE_NUM) {
    return nullptr;
  }

  return GamePadPtrs[Index];
}

HRESULT
InputManager::pollAllInputDevices() {
  HRESULT Hr = S_OK;
  HRESULT FHr = S_OK;

  if (KeyBoardPtr) {
    Hr = KeyBoardPtr->pollDeviceStatus();
    if (FAILED(Hr)) {
      FHr = Hr;
    }
  }
  if (MousePtr) {
    Hr = MousePtr->pollDeviceStatus();
    if (FAILED(Hr)) {
      FHr = Hr;
    }
  }
  for (int I = 0; I < MAX_INPUTDEVICE_NUM; I++) {
    if (GamePadPtrs[I]) {
      Hr = GamePadPtrs[I]->pollDeviceStatus();
    }
    if (FAILED(Hr)) {
      FHr = Hr;
    }
  }

  return FHr;
}

bool
InputManager::isThisKeyBeingPushedInSingle(UINT KeyCode) {
  bool Keyboard = false;
  bool Mouse = false;
  bool Gamepad = false;
  if (KeyBoardPtr) {
    Keyboard = KeyBoardPtr->isKeyBeingPushed(KeyCode);
  }
  if (MousePtr) {
    Mouse = MousePtr->isKeyBeingPushed(KeyCode);
  }
  if (GamePadPtrs[0]) {
    Gamepad = GamePadPtrs[0]->isKeyBeingPushed(KeyCode);
  }

  return (Keyboard || Mouse || Gamepad);
}

bool
InputManager::isThisKeyHasBeenPushedInSingle(UINT KeyCode) {
  bool Confirm1 = isThisKeyBeingPushedInSingle(KeyCode);

  if (!Confirm1) {
    return Confirm1;
  }

  bool KConfirm2 = false;
  bool MConfirm2 = false;
  bool GConfirm2 = false;

  if (KeyBoardPtr) {
    KConfirm2 = KeyBoardPtr->hasKeyPushedInLastFrame(KeyCode);
  }
  if (MousePtr) {
    MConfirm2 = MousePtr->hasKeyPushedInLastFrame(KeyCode);
  }
  if (GamePadPtrs[0]) {
    GConfirm2 = GamePadPtrs[0]->hasKeyPushedInLastFrame(KeyCode);
  }

  return (!(KConfirm2 || MConfirm2 || GConfirm2) && Confirm1);
}

MOUSE_OFFSET
InputManager::getMouseOffset() {
  MOUSE_OFFSET Mo = {};
  Mo.x = MousePtr->getXPositionOffset();
  Mo.y = MousePtr->getYPositionOffset();

  return Mo;
}

bool
InputManager::isMouseScrollingUp() {
  return MousePtr->getZPositionOffset() > 0;
}

bool
InputManager::isMouseScrollingDown() {
  return MousePtr->getZPositionOffset() < 0;
}

STICK_OFFSET
InputManager::getGamePadLeftStickOffset(int GamepadIndex) {
  STICK_OFFSET So = {};
  if (GamePadPtrs[GamepadIndex]) {
    if (GamePadPtrs[GamepadIndex]->getInputType() == INPUT_TYPE::DIRECTINPUT) {
      So.x = GamePadPtrs[GamepadIndex]->getXPositionOffset();
      So.y = GamePadPtrs[GamepadIndex]->getYPositionOffset();
    } else if (GamePadPtrs[GamepadIndex]->getInputType() ==
               INPUT_TYPE::XINPUT) {
      So.x = GamePadPtrs[GamepadIndex]->getXPositionOffset();
      So.y = GamePadPtrs[GamepadIndex]->getYPositionOffset();
    } else {
      So.x = 0;
      So.y = 0;
    }
  } else {
    So.x = 0;
    So.y = 0;
  }

  return So;
}

STICK_OFFSET
InputManager::getGamePadRightStickOffset(int GamepadIndex) {
  STICK_OFFSET So = {};
  if (GamePadPtrs[GamepadIndex]) {
    if (GamePadPtrs[GamepadIndex]->getInputType() == INPUT_TYPE::DIRECTINPUT) {
      So.x = GamePadPtrs[GamepadIndex]->getZPositionOffset();
      So.y = GamePadPtrs[GamepadIndex]->getZRotationOffset();
    } else if (GamePadPtrs[GamepadIndex]->getInputType() ==
               INPUT_TYPE::XINPUT) {
      So.x = GamePadPtrs[GamepadIndex]->getXRotationOffset();
      So.y = GamePadPtrs[GamepadIndex]->getYRotationOffset();
    } else {
      So.x = 0;
      So.y = 0;
    }
  } else {
    So.x = 0;
    So.y = 0;
  }

  return So;
}

BACKSHD_OFFSET
InputManager::getGamePadLeftBackShdBtnOffset(int GamepadIndex) {
  if (!GamePadPtrs[GamepadIndex]) {
    return 0;
  }

  if (GamePadPtrs[GamepadIndex]->getInputType() == INPUT_TYPE::DIRECTINPUT) {
    return GamePadPtrs[GamepadIndex]->getXRotationOffset();
  } else if (GamePadPtrs[GamepadIndex]->getInputType() == INPUT_TYPE::XINPUT) {
    return GamePadPtrs[GamepadIndex]->getZPositionOffset();
  } else {
    return 0;
  }
}

BACKSHD_OFFSET
InputManager::getGamePadRightBackShdBtnOffset(int GamepadIndex) {
  if (!GamePadPtrs[GamepadIndex]) {
    return 0;
  }

  if (GamePadPtrs[GamepadIndex]->getInputType() == INPUT_TYPE::DIRECTINPUT) {
    return GamePadPtrs[GamepadIndex]->getYRotationOffset();
  } else if (GamePadPtrs[GamepadIndex]->getInputType() == INPUT_TYPE::XINPUT) {
    return GamePadPtrs[GamepadIndex]->getZRotationOffset();
  } else {
    return 0;
  }
}
