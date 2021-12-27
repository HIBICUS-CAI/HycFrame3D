#pragma once

#include "System.h"
#include <vector>

class InstanceSystem :public System
{
public:
    InstanceSystem(class SystemExecutive* _sysExecutive);
    virtual ~InstanceSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    std::vector<class ATransformComponent>* mATransVecPtr;
    std::vector<class UTransformComponent>* mUTransVecPtr;
    std::vector<class AMeshComponent>* mAMeshVecPtr;
    std::vector<class ALightComponent>* mALightVecPtr;
    std::vector<class AParticleComponent>* mAParitcleVecPtr;
};
