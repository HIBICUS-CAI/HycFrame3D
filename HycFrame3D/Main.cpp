#include "RootSystem.h"

int
WinMain(_In_ HINSTANCE Instance,
        _In_opt_ HINSTANCE PrevInstance,
        _In_ LPSTR CmdLine,
        _In_ int CmdShow) {
  RootSystem root;

  if (root.startUp(Instance, CmdShow)) {
    root.runGameLoop();
  }

  root.cleanAndStop();

  return 0;
}
