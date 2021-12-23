#pragma once

#include "Hyc3DCommon.h"
#include <string>

class Component
{
public:
    Component(std::string&& _compName);
    Component(std::string& _compName);
    virtual ~Component();

    const std::string& GetCompName() const;

    STATUS GetCompStatus() const;
    void SetCompStatus(STATUS _compStatus);

public:
    virtual bool Init() = 0;
    virtual void Update(Timer& _timer) = 0;
    virtual void Destory() = 0;

private:
    const std::string mComponentName;
    STATUS mComponentStatus;
};
