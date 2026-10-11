#include "WM_Common.h"

#include "WindowSDL2.h"

#include <SDL.h>

WindowSDL2::~WindowSDL2() {
  if (WndHandle) {
    SDL_DestroyWindow(WndHandle);
    WndHandle = nullptr;
  }
}

bool WindowSDL2::createWindow(const char *WndName,
                              unsigned int Width,
                              unsigned int Height,
                              bool AsFullScreen) {
  // the client area is fixed like the original win32 window,
  // render system scales its output to the real surface size
  WndHandle = SDL_CreateWindow(
      WndName, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      static_cast<int>(Width), static_cast<int>(Height),
      SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

  if (!WndHandle) {
    return false;
  }

  if (AsFullScreen) {
    return switchWindowSize();
  }

  return true;
}

bool WindowSDL2::switchWindowSize() {
  Uint32 Flag = FullScreenFlag ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP;
  if (SDL_SetWindowFullscreen(WndHandle, Flag) != 0) {
    return false;
  }

  if (FullScreenFlag) {
    SDL_SetWindowSize(WndHandle, 1280, 720);
    SDL_SetWindowPosition(WndHandle, SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED);
  }

  FullScreenFlag = !FullScreenFlag;

  return true;
}

void WindowSDL2::getClientSize(int &OutWidth, int &OutHeight) const {
  SDL_GetWindowSize(WndHandle, &OutWidth, &OutHeight);
}

void WindowSDL2::getCursorPosition(int &OutX, int &OutY) const {
  // relative to the client area like GetCursorPos + ScreenToClient
  int GlobalX = 0;
  int GlobalY = 0;
  int WndX = 0;
  int WndY = 0;
  SDL_GetGlobalMouseState(&GlobalX, &GlobalY);
  SDL_GetWindowPosition(WndHandle, &WndX, &WndY);
  OutX = GlobalX - WndX;
  OutY = GlobalY - WndY;
}
