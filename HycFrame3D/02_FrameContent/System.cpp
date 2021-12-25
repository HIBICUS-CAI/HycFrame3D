#include "System.h"
#include "SystemExecutive.h"

System::System(std::string _sysName, SystemExecutive* _sysExecutive) :
    mSystemName(_sysName), mSystemExecutivePtr(_sysExecutive)
{

}

System::~System()
{

}

const std::string& System::GetSystemName() const
{
    return mSystemName;
}

SystemExecutive* System::GetSystemExecutive() const
{
    return mSystemExecutivePtr;
}
