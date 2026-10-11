#pragma once

#include "WM_ExportMacro.h"

#include "WindowSDL2.h"

namespace window {

bool WINDOWMANAGER_EXPORT startUp();

bool WINDOWMANAGER_EXPORT createWindow(const char *WndName,
                                       unsigned int Width,
                                       unsigned int Height,
                                       bool AsFullScreen = false);

WindowSDL2 WINDOWMANAGER_EXPORT *getWindowPtr();

// dispatch all pending window messages, return false after quit requested
bool WINDOWMANAGER_EXPORT pollMessages();

void WINDOWMANAGER_EXPORT cleanAndStop();

} // namespace window
