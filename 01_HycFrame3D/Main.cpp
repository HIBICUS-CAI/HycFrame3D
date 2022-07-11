#include "RootSystem.h"

int WinMain(_In_ HINSTANCE Instance,
            _In_opt_ HINSTANCE PrevInstance,
            _In_ LPSTR CmdLine,
            _In_ int CmdShow) {
  RootSystem Root;

  if (Root.startUp(Instance, CmdShow)) {
    Root.runGameLoop();
  }

  Root.cleanAndStop();

  return 0;
}
