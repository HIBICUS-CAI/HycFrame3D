#pragma once

#include "System.h"

#include <vector>

class ButtonSystem : public System {
private:
  std::vector<class UButtonComponent> *UBtnArrayPtr;

public:
  ButtonSystem(class SystemExecutive *SysExecutive);
  virtual ~ButtonSystem();

public:
  virtual bool
  init();
  virtual void
  run(Timer &Timer);
  virtual void
  destory();
};
