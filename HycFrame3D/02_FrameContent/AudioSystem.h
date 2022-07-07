#pragma once

#include "System.h"

#include <vector>

class AudioSystem : public System {
private:
  std::vector<class AAudioComponent> *AAudioArrayPtr;
  std::vector<class UAudioComponent> *UAudioArrayPtr;

public:
  AudioSystem(class SystemExecutive *SysExecutive);
  virtual ~AudioSystem();

public:
  virtual bool
  init();
  virtual void
  run(Timer &Timer);
  virtual void
  destory();
};
