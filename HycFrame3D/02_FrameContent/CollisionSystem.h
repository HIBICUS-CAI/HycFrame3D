#pragma once

#include "System.h"

#include <vector>

class CollisionSystem : public System {
private:
  std::vector<class ACollisionComponent> *ACollisionArrayPtr;

public:
  CollisionSystem(class SystemExecutive *SysExecutive);
  virtual ~CollisionSystem();

public:
  virtual bool init();
  virtual void run(const Timer &Timer);
  virtual void destory();
};
