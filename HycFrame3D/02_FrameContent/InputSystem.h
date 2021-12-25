#pragma once

#include "System.h"
#include <vector>

class InputSystem :public System
{
public:
    InputSystem(class SystemExecutive* _sysExecutive);
    virtual ~InputSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    std::vector<class AInputComponent>* mAInputVecPtr;
    std::vector<class UInputComponent>* mUInputVecPtr;
};
