#include "WM_Common.h"

#include "WindowWIN32.h"

#include <string.h>

HRESULT WindowWIN32::createWindow(const char *WndName,
                                  HINSTANCE HInstance,
                                  int CmdShow,
                                  UINT Width,
                                  UINT Height,
                                  bool AsFullScreen) {
  char ClassName[128] = "";
  strcpy_s(ClassName, sizeof(ClassName), WndName);
  strcat_s(ClassName, sizeof(ClassName), " CLASS");

  WNDCLASSEX Wcex = {};
  Wcex.cbSize = sizeof(WNDCLASSEX);
  Wcex.style = CS_HREDRAW | CS_VREDRAW;
  Wcex.lpfnWndProc = wndProc;
  Wcex.cbClsExtra = 0;
  Wcex.cbWndExtra = 0;
  Wcex.hInstance = HInstance;
  Wcex.hIcon = LoadIcon(HInstance, IDI_APPLICATION);
  Wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  Wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  Wcex.lpszMenuName = nullptr;
  Wcex.lpszClassName = ClassName;
  Wcex.hIconSm = LoadIcon(Wcex.hInstance, IDI_APPLICATION);

  if (!RegisterClassEx(&Wcex)) {
    return E_FAIL;
  }

  this->Instance = HInstance;

  RECT Rc = {0, 0, (LONG)Width, (LONG)Height};
  AdjustWindowRect(&Rc, WS_OVERLAPPEDWINDOW, FALSE);
  this->WndHandle =
      CreateWindow(ClassName, WndName,
                   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                   CW_USEDEFAULT, CW_USEDEFAULT, Rc.right - Rc.left,
                   Rc.bottom - Rc.top, nullptr, nullptr, HInstance, nullptr);

  if (!this->WndHandle) {
    return E_FAIL;
  }

  ShowWindow(this->WndHandle, CmdShow);
  HWND HDesk = GetDesktopWindow();
  GetWindowRect(HDesk, &Rc);
  UINT OffsetX = Rc.right / 2;
  UINT OffsetY = Rc.bottom / 2;
  SetWindowLong(this->WndHandle, GWL_STYLE, WS_OVERLAPPED);
  SetWindowPos(this->WndHandle, HWND_NOTOPMOST, OffsetX - Width / 2,
               OffsetY - Height / 2, Width, Height, SWP_SHOWWINDOW);

  if (AsFullScreen) {
    return switchWindowSize();
  }

  return S_OK;
}

LRESULT CALLBACK WindowWIN32::wndProc(HWND HWnd,
                                      UINT Message,
                                      WPARAM WParam,
                                      LPARAM LParam) {
  PAINTSTRUCT Ps = {};
  HDC Hdc = {};

  switch (Message) {
  case WM_PAINT:
    Hdc = BeginPaint(HWnd, &Ps);
    EndPaint(HWnd, &Ps);
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(HWnd, Message, WParam, LParam);
  }

  (void)Hdc;

  return 0;
}

HRESULT WindowWIN32::switchWindowSize() {
  if (this->FullScreenFlag) {
    HWND HDesk = GetDesktopWindow();
    RECT Rc = {};
    GetWindowRect(HDesk, &Rc);
    UINT OffsetX = Rc.right / 2;
    UINT OffsetY = Rc.bottom / 2;
    UINT Width = 1280;
    UINT Height = 720;

    SetWindowLong(this->WndHandle, GWL_STYLE, WS_OVERLAPPED);
    SetWindowPos(this->WndHandle, HWND_NOTOPMOST, OffsetX - Width / 2,
                 OffsetY - Height / 2, Width, Height, SWP_SHOWWINDOW);
  } else {
    HWND HDesk = GetDesktopWindow();
    RECT Rc = {};
    GetWindowRect(HDesk, &Rc);

    SetWindowLong(this->WndHandle, GWL_STYLE, WS_POPUP);
    SetWindowPos(this->WndHandle, HWND_NOTOPMOST, 0, 0, Rc.right, Rc.bottom,
                 SWP_SHOWWINDOW);
  }

  FullScreenFlag = !FullScreenFlag;

  return S_OK;
}
