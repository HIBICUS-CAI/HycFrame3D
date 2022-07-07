#pragma once

#include "WM_ExportMacro.h"

#include <Windows.h>

class WINDOWMANAGER_EXPORT WindowWIN32 {
private:
  HINSTANCE Instance;
  HWND WndHandle;
  bool FullScreenFlag;

public:
  WindowWIN32()
      : Instance(nullptr), WndHandle(nullptr), FullScreenFlag(false) {}

  ~WindowWIN32() {}

  HRESULT createWindow(const char *WndName,
                       HINSTANCE HInstance,
                       int CmdShow,
                       UINT Width,
                       UINT Height,
                       bool AsFullScreen);

  static LRESULT CALLBACK wndProc(HWND HWnd,
                                  UINT Message,
                                  WPARAM WParam,
                                  LPARAM LParam);

  HRESULT switchWindowSize();

  HINSTANCE getWndInstance() const { return Instance; }

  HWND getWndHandle() const { return WndHandle; }

  bool isFullScreen() const { return FullScreenFlag; }
};
