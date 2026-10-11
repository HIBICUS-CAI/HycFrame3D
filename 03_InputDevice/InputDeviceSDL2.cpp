#include "ID_Common.h"

#include "InputDeviceSDL2.h"

#include <SDL.h>

#include <utility>

// DirectInput keyboard codes are PC/AT set 1 scan codes, game logic is
// written with them (KB_* macros), so translate SDL's USB HID scan codes.
static const std::pair<SDL_Scancode, unsigned int> G_ScancodeToKeyCode[] = {
    {SDL_SCANCODE_ESCAPE, KB_ESCAPE},
    {SDL_SCANCODE_1, KB_1},
    {SDL_SCANCODE_2, KB_2},
    {SDL_SCANCODE_3, KB_3},
    {SDL_SCANCODE_4, KB_4},
    {SDL_SCANCODE_5, KB_5},
    {SDL_SCANCODE_6, KB_6},
    {SDL_SCANCODE_7, KB_7},
    {SDL_SCANCODE_8, KB_8},
    {SDL_SCANCODE_9, KB_9},
    {SDL_SCANCODE_0, KB_0},
    {SDL_SCANCODE_MINUS, KB_MINUS},
    {SDL_SCANCODE_EQUALS, KB_EQUALS},
    {SDL_SCANCODE_BACKSPACE, KB_BACK},
    {SDL_SCANCODE_TAB, KB_TAB},
    {SDL_SCANCODE_Q, KB_Q},
    {SDL_SCANCODE_W, KB_W},
    {SDL_SCANCODE_E, KB_E},
    {SDL_SCANCODE_R, KB_R},
    {SDL_SCANCODE_T, KB_T},
    {SDL_SCANCODE_Y, KB_Y},
    {SDL_SCANCODE_U, KB_U},
    {SDL_SCANCODE_I, KB_I},
    {SDL_SCANCODE_O, KB_O},
    {SDL_SCANCODE_P, KB_P},
    {SDL_SCANCODE_LEFTBRACKET, KB_LBRACKET},
    {SDL_SCANCODE_RIGHTBRACKET, KB_RBRACKET},
    {SDL_SCANCODE_RETURN, KB_RETURN},
    {SDL_SCANCODE_LCTRL, KB_LCONTROL},
    {SDL_SCANCODE_A, KB_A},
    {SDL_SCANCODE_S, KB_S},
    {SDL_SCANCODE_D, KB_D},
    {SDL_SCANCODE_F, KB_F},
    {SDL_SCANCODE_G, KB_G},
    {SDL_SCANCODE_H, KB_H},
    {SDL_SCANCODE_J, KB_J},
    {SDL_SCANCODE_K, KB_K},
    {SDL_SCANCODE_L, KB_L},
    {SDL_SCANCODE_SEMICOLON, KB_SEMICOLON},
    {SDL_SCANCODE_APOSTROPHE, KB_APOSTROPHE},
    {SDL_SCANCODE_GRAVE, KB_GRAVE},
    {SDL_SCANCODE_LSHIFT, KB_LSHIFT},
    {SDL_SCANCODE_BACKSLASH, KB_BACKSLASH},
    {SDL_SCANCODE_Z, KB_Z},
    {SDL_SCANCODE_X, KB_X},
    {SDL_SCANCODE_C, KB_C},
    {SDL_SCANCODE_V, KB_V},
    {SDL_SCANCODE_B, KB_B},
    {SDL_SCANCODE_N, KB_N},
    {SDL_SCANCODE_M, KB_M},
    {SDL_SCANCODE_COMMA, KB_COMMA},
    {SDL_SCANCODE_PERIOD, KB_PERIOD},
    {SDL_SCANCODE_SLASH, KB_SLASH},
    {SDL_SCANCODE_RSHIFT, KB_RSHIFT},
    {SDL_SCANCODE_KP_MULTIPLY, KB_MULTIPLY},
    {SDL_SCANCODE_LALT, KB_LMENU},
    {SDL_SCANCODE_SPACE, KB_SPACE},
    {SDL_SCANCODE_CAPSLOCK, KB_CAPITAL},
    {SDL_SCANCODE_F1, KB_F1},
    {SDL_SCANCODE_F2, KB_F2},
    {SDL_SCANCODE_F3, KB_F3},
    {SDL_SCANCODE_F4, KB_F4},
    {SDL_SCANCODE_F5, KB_F5},
    {SDL_SCANCODE_F6, KB_F6},
    {SDL_SCANCODE_F7, KB_F7},
    {SDL_SCANCODE_F8, KB_F8},
    {SDL_SCANCODE_F9, KB_F9},
    {SDL_SCANCODE_F10, KB_F10},
    {SDL_SCANCODE_NUMLOCKCLEAR, KB_NUMLOCK},
    {SDL_SCANCODE_SCROLLLOCK, KB_SCROLL},
    {SDL_SCANCODE_KP_7, KB_NUMPAD7},
    {SDL_SCANCODE_KP_8, KB_NUMPAD8},
    {SDL_SCANCODE_KP_9, KB_NUMPAD9},
    {SDL_SCANCODE_KP_MINUS, KB_SUBTRACT},
    {SDL_SCANCODE_KP_4, KB_NUMPAD4},
    {SDL_SCANCODE_KP_5, KB_NUMPAD5},
    {SDL_SCANCODE_KP_6, KB_NUMPAD6},
    {SDL_SCANCODE_KP_PLUS, KB_ADD},
    {SDL_SCANCODE_KP_1, KB_NUMPAD1},
    {SDL_SCANCODE_KP_2, KB_NUMPAD2},
    {SDL_SCANCODE_KP_3, KB_NUMPAD3},
    {SDL_SCANCODE_KP_0, KB_NUMPAD0},
    {SDL_SCANCODE_KP_PERIOD, KB_DECIMAL},
    {SDL_SCANCODE_NONUSBACKSLASH, KB_OEM_102},
    {SDL_SCANCODE_F11, KB_F11},
    {SDL_SCANCODE_F12, KB_F12},
    {SDL_SCANCODE_F13, KB_F13},
    {SDL_SCANCODE_F14, KB_F14},
    {SDL_SCANCODE_F15, KB_F15},
    {SDL_SCANCODE_INTERNATIONAL2, KB_KANA},
    {SDL_SCANCODE_INTERNATIONAL1, KB_ABNT_C1},
    {SDL_SCANCODE_INTERNATIONAL4, KB_CONVERT},
    {SDL_SCANCODE_INTERNATIONAL5, KB_NOCONVERT},
    {SDL_SCANCODE_INTERNATIONAL3, KB_YEN},
    {SDL_SCANCODE_KP_EQUALS, KB_NUMPADEQUALS},
    {SDL_SCANCODE_AUDIOPREV, KB_PREVTRACK},
    {SDL_SCANCODE_LANG5, KB_KANJI},
    {SDL_SCANCODE_STOP, KB_STOP},
    {SDL_SCANCODE_AUDIONEXT, KB_NEXTTRACK},
    {SDL_SCANCODE_KP_ENTER, KB_NUMPADENTER},
    {SDL_SCANCODE_RCTRL, KB_RCONTROL},
    {SDL_SCANCODE_AUDIOMUTE, KB_MUTE},
    {SDL_SCANCODE_CALCULATOR, KB_CALCULATOR},
    {SDL_SCANCODE_AUDIOPLAY, KB_PLAYPAUSE},
    {SDL_SCANCODE_AUDIOSTOP, KB_MEDIASTOP},
    {SDL_SCANCODE_VOLUMEDOWN, KB_VOLUMEDOWN},
    {SDL_SCANCODE_VOLUMEUP, KB_VOLUMEUP},
    {SDL_SCANCODE_AC_HOME, KB_WEBHOME},
    {SDL_SCANCODE_KP_COMMA, KB_NUMPADCOMMA},
    {SDL_SCANCODE_KP_DIVIDE, KB_DIVIDE},
    {SDL_SCANCODE_SYSREQ, KB_SYSRQ},
    {SDL_SCANCODE_RALT, KB_RMENU},
    {SDL_SCANCODE_PAUSE, KB_PAUSE},
    {SDL_SCANCODE_HOME, KB_HOME},
    {SDL_SCANCODE_UP, KB_UP},
    {SDL_SCANCODE_PAGEUP, KB_PRIOR},
    {SDL_SCANCODE_LEFT, KB_LEFT},
    {SDL_SCANCODE_RIGHT, KB_RIGHT},
    {SDL_SCANCODE_END, KB_END},
    {SDL_SCANCODE_DOWN, KB_DOWN},
    {SDL_SCANCODE_PAGEDOWN, KB_NEXT},
    {SDL_SCANCODE_INSERT, KB_INSERT},
    {SDL_SCANCODE_DELETE, KB_DELETE},
    {SDL_SCANCODE_LGUI, KB_LWIN},
    {SDL_SCANCODE_RGUI, KB_RWIN},
    {SDL_SCANCODE_APPLICATION, KB_APPS},
    {SDL_SCANCODE_POWER, KB_POWER},
    {SDL_SCANCODE_SLEEP, KB_SLEEP},
    {SDL_SCANCODE_AC_SEARCH, KB_WEBSEARCH},
    {SDL_SCANCODE_AC_BOOKMARKS, KB_WEBFAVORITES},
    {SDL_SCANCODE_AC_REFRESH, KB_WEBREFRESH},
    {SDL_SCANCODE_AC_STOP, KB_WEBSTOP},
    {SDL_SCANCODE_AC_FORWARD, KB_WEBFORWARD},
    {SDL_SCANCODE_AC_BACK, KB_WEBBACK},
    {SDL_SCANCODE_COMPUTER, KB_MYCOMPUTER},
    {SDL_SCANCODE_MAIL, KB_MAIL},
    {SDL_SCANCODE_MEDIASELECT, KB_MEDIASELECT},
};

int32_t InputDeviceSDL2::AccumulatedMouseX = 0;
int32_t InputDeviceSDL2::AccumulatedMouseY = 0;
int32_t InputDeviceSDL2::AccumulatedMouseZ = 0;

InputDeviceSDL2::InputDeviceSDL2(INPUT_DEVICE_TYPE DeviceType)
    : InputDeviceBase(DeviceType), ButtonsStatus({false}),
      ButtonsStatusBeforeThisPoll({false}), MouseOffsetX(0), MouseOffsetY(0),
      MouseOffsetZ(0) {
  if (DeviceType == INPUT_DEVICE_TYPE::MOUSE) {
    AccumulatedMouseX = 0;
    AccumulatedMouseY = 0;
    AccumulatedMouseZ = 0;
    SDL_AddEventWatch(mouseEventWatcher, nullptr);
  }
}

InputDeviceSDL2::~InputDeviceSDL2() {
  if (getInputDeviceType() == INPUT_DEVICE_TYPE::MOUSE) {
    SDL_DelEventWatch(mouseEventWatcher, nullptr);
  }
}

int InputDeviceSDL2::mouseEventWatcher(void *UserData, SDL_Event *Event) {
  (void)UserData;
  switch (Event->type) {
  case SDL_MOUSEMOTION:
    AccumulatedMouseX += Event->motion.xrel;
    AccumulatedMouseY += Event->motion.yrel;
    break;
  case SDL_MOUSEWHEEL: {
    // one notch is WHEEL_DELTA (120) in DirectInput
    int32_t Notch = Event->wheel.y;
    if (Event->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) {
      Notch = -Notch;
    }
    AccumulatedMouseZ += Notch * 120;
    break;
  }
  default:
    break;
  }

  return 0;
}

INPUT_TYPE InputDeviceSDL2::getInputType() { return INPUT_TYPE::SDL2; }

bool InputDeviceSDL2::pollDeviceStatus() {
  // store this status to backup before being updated
  ButtonsStatusBeforeThisPoll = ButtonsStatus;
  ButtonsStatus.fill(false);

  switch (getInputDeviceType()) {
  case INPUT_DEVICE_TYPE::KEYBOARD:
    pollKeyboard();
    break;
  case INPUT_DEVICE_TYPE::MOUSE:
    pollMouse();
    break;
  default:
    break;
  }

  return true;
}

void InputDeviceSDL2::pollKeyboard() {
  // keyboard is only readable in foreground like DISCL_FOREGROUND
  if (!SDL_GetKeyboardFocus()) {
    return;
  }

  int KeyNum = 0;
  const Uint8 *KeyState = SDL_GetKeyboardState(&KeyNum);
  for (const auto &Pair : G_ScancodeToKeyCode) {
    if (Pair.first < KeyNum && KeyState[Pair.first]) {
      ButtonsStatus[Pair.second] = true;
    }
  }
}

void InputDeviceSDL2::pollMouse() {
  MouseOffsetX = AccumulatedMouseX;
  MouseOffsetY = AccumulatedMouseY;
  MouseOffsetZ = AccumulatedMouseZ;
  AccumulatedMouseX = 0;
  AccumulatedMouseY = 0;
  AccumulatedMouseZ = 0;

  if (!SDL_GetMouseFocus()) {
    MouseOffsetX = 0;
    MouseOffsetY = 0;
    MouseOffsetZ = 0;
    return;
  }

  Uint32 Buttons = SDL_GetMouseState(nullptr, nullptr);
  const Uint32 ButtonMasks[] = {SDL_BUTTON_LMASK, SDL_BUTTON_RMASK,
                                SDL_BUTTON_MMASK, SDL_BUTTON_X1MASK,
                                SDL_BUTTON_X2MASK};
  for (unsigned int I = 0; I < 5; I++) {
    if (Buttons & ButtonMasks[I]) {
      ButtonsStatus[I + MOUSE_BTN_OFFSET] = true;
    }
  }
}

bool InputDeviceSDL2::hasKeyPushedInLastFrame(unsigned int KeyCode) {
  if (KeyCode >= ButtonsStatusBeforeThisPoll.size()) {
    return false;
  }
  return ButtonsStatusBeforeThisPoll[KeyCode];
}

bool InputDeviceSDL2::isKeyBeingPushed(unsigned int KeyCode) {
  switch (getInputDeviceType()) {
  case INPUT_DEVICE_TYPE::KEYBOARD:
    if (KeyCode < 0x01 || KeyCode > 0xED) {
      return false;
    }
    return ButtonsStatus[KeyCode];
  case INPUT_DEVICE_TYPE::MOUSE:
    if (KeyCode < MOUSE_BTN_OFFSET || KeyCode > MOUSE_BTN_OFFSET + 0x07) {
      return false;
    }
    return ButtonsStatus[KeyCode];
  default:
    return false;
  }
}

int32_t InputDeviceSDL2::getXPositionOffset() {
  if (getInputDeviceType() != INPUT_DEVICE_TYPE::MOUSE) {
    return 0;
  }
  return MouseOffsetX;
}

int32_t InputDeviceSDL2::getYPositionOffset() {
  if (getInputDeviceType() != INPUT_DEVICE_TYPE::MOUSE) {
    return 0;
  }
  return MouseOffsetY;
}

int32_t InputDeviceSDL2::getZPositionOffset() {
  if (getInputDeviceType() != INPUT_DEVICE_TYPE::MOUSE) {
    return 0;
  }
  return MouseOffsetZ;
}

int32_t InputDeviceSDL2::getXRotationOffset() { return 0; }

int32_t InputDeviceSDL2::getYRotationOffset() { return 0; }

int32_t InputDeviceSDL2::getZRotationOffset() { return 0; }
