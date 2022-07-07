#pragma once

#include "System.h"

#include <vector>

class InteractSystem : public System {
private:
  std::vector<class AInteractComponent> *AInterArrayPtr;
  std::vector<class UInteractComponent> *UInterArrayPtr;

public:
  InteractSystem(class SystemExecutive *SysExecutive);
  virtual ~InteractSystem();

public:
  virtual bool init();
  virtual void run(const Timer &Timer);
  virtual void destory();
};
