#include "WM_Common.h"

#include "WM_Interface.h"

namespace window {

WindowWIN32 *G_Wnd = nullptr;

bool startUp() {
  G_Wnd = new WindowWIN32();

  if (G_Wnd) {
    return true;
  } else {
    return false;
  }
}

bool createWindow(const char *WndName,
                  HINSTANCE HInstance,
                  int CmdShow,
                  UINT Width,
                  UINT Height,
                  bool AsFullScreen) {
  if (!G_Wnd) {
    return false;
  }

  HRESULT Hr = G_Wnd->createWindow(WndName, HInstance, CmdShow, Width, Height,
                                   AsFullScreen);

  if (FAILED(Hr)) {
    delete G_Wnd;
    G_Wnd = nullptr;
    return false;
  }

  return true;
}

WindowWIN32 *getWindowPtr() { return G_Wnd; }

void cleanAndStop() {
  if (G_Wnd) {
    delete G_Wnd;
    G_Wnd = nullptr;
  }
}

} // namespace window