#pragma once

#include "Hyc3DCommon.h"
#include <string>

class System
{
public:
    System(std::string _sysName, class SystemExecutive& _sysExecutive);
    virtual ~System();

    const std::string& GetSystemName() const;
    class SystemExecutive& GetSystemExecutive() const;

public:
    virtual bool Init() = 0;
    virtual void Run() = 0;
    virtual void Destory() = 0;

private:
    const std::string mSystemName;
    class SystemExecutive& mSystemExecutivePtr;
};
