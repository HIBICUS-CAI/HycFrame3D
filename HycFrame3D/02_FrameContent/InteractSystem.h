#pragma once

#include "System.h"
#include <vector>

class InteractSystem :public System
{
public:
    InteractSystem(class SystemExecutive* _sysExecutive);
    virtual ~InteractSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    std::vector<class AInteractComponent>* mAInterVecPtr;
    std::vector<class UInteractComponent>* mUInterVecPtr;
};
