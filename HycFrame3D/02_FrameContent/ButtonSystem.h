#pragma once

#include "System.h"
#include <vector>

class ButtonSystem :public System
{
public:
    ButtonSystem(class SystemExecutive* _sysExecutive);
    virtual ~ButtonSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    std::vector<class UButtonComponent>* mUBtnVecPtr;
};
