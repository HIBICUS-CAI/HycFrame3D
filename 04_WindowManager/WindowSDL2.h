#pragma once

#include "WM_ExportMacro.h"

struct SDL_Window;

class WINDOWMANAGER_EXPORT WindowSDL2 {
private:
  SDL_Window *WndHandle;
  bool FullScreenFlag;

public:
  WindowSDL2() : WndHandle(nullptr), FullScreenFlag(false) {}

  ~WindowSDL2();

  bool createWindow(const char *WndName,
                    unsigned int Width,
                    unsigned int Height,
                    bool AsFullScreen);

  bool switchWindowSize();

  SDL_Window *getWndHandle() const { return WndHandle; }

  bool isFullScreen() const { return FullScreenFlag; }

  void getClientSize(int &OutWidth, int &OutHeight) const;

  void getCursorPosition(int &OutX, int &OutY) const;
};
