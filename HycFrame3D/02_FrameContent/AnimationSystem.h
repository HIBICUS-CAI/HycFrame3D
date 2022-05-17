#pragma once

#include "System.h"
#include <vector>

class AnimationSystem :public System
{
public:
    AnimationSystem(class SystemExecutive* _sysExecutive);
    virtual ~AnimationSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    std::vector<class AAnimateComponent>* mAAnimateVecPtr;
};
