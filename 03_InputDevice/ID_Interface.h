#pragma once

#include "ID_ExportMacro.h"

#include "ID_BasicMacro.h"

#include "InputManager.h"

namespace input {

bool INPUTDEVICE_EXPORT startUp();
void INPUTDEVICE_EXPORT cleanAndStop();

InputManager INPUTDEVICE_EXPORT *getInputManagerPtr();
bool INPUTDEVICE_EXPORT pollDevices();

bool INPUTDEVICE_EXPORT isKeyDownInSingle(UINT KeyCode);
bool INPUTDEVICE_EXPORT isKeyPushedInSingle(UINT KeyCode);
STICK_OFFSET INPUTDEVICE_EXPORT leftStickOffset(int GamepadIndex = 0);
STICK_OFFSET INPUTDEVICE_EXPORT rightStickOffset(int GamepadIndex = 0);
BACKSHD_OFFSET INPUTDEVICE_EXPORT leftBackShdBtnOffset(int GamepadIndex = 0);
BACKSHD_OFFSET INPUTDEVICE_EXPORT rightBackShdBtnOffset(int GamepadIndex = 0);
MOUSE_OFFSET INPUTDEVICE_EXPORT getMouseOffset();
bool INPUTDEVICE_EXPORT isMouseScrollingUp();
bool INPUTDEVICE_EXPORT isMouseScrollingDown();

} // namespace input
