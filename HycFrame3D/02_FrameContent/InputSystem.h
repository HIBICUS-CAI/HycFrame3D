#pragma once

#include "System.h"
#include <vector>

class InputSystem : public System {
private:
  std::vector<class AInputComponent> *AInputArrayPtr;
  std::vector<class UInputComponent> *UInputArrayPtr;

public:
  InputSystem(class SystemExecutive *SysExecutive);
  virtual ~InputSystem();

public:
  virtual bool init();
  virtual void run(Timer &Timer);
  virtual void destory();
};
