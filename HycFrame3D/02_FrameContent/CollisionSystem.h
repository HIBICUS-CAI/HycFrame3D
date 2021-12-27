#pragma once

#include "System.h"
#include <vector>

class CollisionSystem :public System
{
public:
    CollisionSystem(class SystemExecutive* _sysExecutive);
    virtual ~CollisionSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    std::vector<class ACollisionComponent>* mACollisionVecPtr;
};
