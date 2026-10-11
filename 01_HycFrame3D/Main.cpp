#include "RootSystem.h"

int main(int Argc, char **Argv) {
  (void)Argc;
  (void)Argv;

  RootSystem Root;

  if (Root.startUp()) {
    Root.runGameLoop();
  }

  Root.cleanAndStop();

  return 0;
}
