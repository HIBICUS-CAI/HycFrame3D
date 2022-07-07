#pragma once

#include "WM_ExportMacro.h"

#include "WindowWIN32.h"

#include <Windows.h>

namespace window {

bool WINDOWMANAGER_EXPORT startUp();

bool WINDOWMANAGER_EXPORT createWindow(const char *WndName,
                                       HINSTANCE HInstance,
                                       int CmdShow,
                                       UINT Width,
                                       UINT Height,
                                       bool AsFullScreen = false);

WindowWIN32 WINDOWMANAGER_EXPORT *getWindowPtr();

void WINDOWMANAGER_EXPORT cleanAndStop();

} // namespace window
