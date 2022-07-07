#pragma once

#include "Hyc3DCommon.h"

#include <string>

class System {
private:
  const std::string SystemName;
  class SystemExecutive *SystemExecutivePtr;

public:
  System(const std::string &SysName, class SystemExecutive *SysExecutive);
  virtual ~System();

  const std::string &getSystemName() const;
  class SystemExecutive *getSystemExecutive() const;

public:
  virtual bool init() = 0;
  virtual void run(Timer &Timer) = 0;
  virtual void destory() = 0;
};
