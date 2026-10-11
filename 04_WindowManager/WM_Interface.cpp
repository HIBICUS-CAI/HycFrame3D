#include "WM_Common.h"

#include "WM_Interface.h"

#include <SDL.h>

namespace window {

WindowSDL2 *G_Wnd = nullptr;

bool startUp() {
  if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    return false;
  }

  G_Wnd = new WindowSDL2();

  if (G_Wnd) {
    return true;
  } else {
    return false;
  }
}

bool createWindow(const char *WndName,
                  unsigned int Width,
                  unsigned int Height,
                  bool AsFullScreen) {
  if (!G_Wnd) {
    return false;
  }

  if (!G_Wnd->createWindow(WndName, Width, Height, AsFullScreen)) {
    delete G_Wnd;
    G_Wnd = nullptr;
    return false;
  }

  return true;
}

WindowSDL2 *getWindowPtr() { return G_Wnd; }

bool pollMessages() {
  SDL_Event Event = {};
  bool KeepRunning = true;
  // other modules observe events through SDL event watchers
  while (SDL_PollEvent(&Event)) {
    if (Event.type == SDL_QUIT) {
      KeepRunning = false;
    }
  }

  return KeepRunning;
}

void cleanAndStop() {
  if (G_Wnd) {
    delete G_Wnd;
    G_Wnd = nullptr;
  }
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
}

} // namespace window
