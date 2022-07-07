#pragma once

#include "System.h"

#include <vector>

class AnimationSystem : public System {
private:
  std::vector<class AAnimateComponent> *AAnimateArrayPtr;

public:
  AnimationSystem(class SystemExecutive *SysExecutive);
  virtual ~AnimationSystem();

public:
  virtual bool
  init();
  virtual void
  run(Timer &Timer);
  virtual void
  destory();
};
