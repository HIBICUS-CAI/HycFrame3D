#pragma once

#include "System.h"

#include <vector>

class TimerSystem : public System {
private:
  std::vector<class ATimerComponent> *ATimerArrayPtr;
  std::vector<class UTimerComponent> *UTimerArrayPtr;

public:
  TimerSystem(class SystemExecutive *SysExecutive);
  virtual ~TimerSystem();

public:
  virtual bool init();
  virtual void run(const Timer &Timer);
  virtual void destory();
};
