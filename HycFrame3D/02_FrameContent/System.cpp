#include "System.h"

#include "SystemExecutive.h"

System::System(const std::string &SysName, SystemExecutive *SysExecutive)
    : SystemName(SysName), SystemExecutivePtr(SysExecutive) {}

System::~System() {}

const std::string &
System::getSystemName() const {
  return SystemName;
}

SystemExecutive *
System::getSystemExecutive() const {
  return SystemExecutivePtr;
}
