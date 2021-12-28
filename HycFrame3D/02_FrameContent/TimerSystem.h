#pragma once

#include "System.h"
#include <vector>

class TimerSystem :public System
{
public:
    TimerSystem(class SystemExecutive* _sysExecutive);
    virtual ~TimerSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    std::vector<class ATimerComponent>* mATimerVecPtr;
};
