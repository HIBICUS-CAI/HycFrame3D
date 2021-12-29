#pragma once

#include "System.h"
#include <vector>

class AudioSystem :public System
{
public:
    AudioSystem(class SystemExecutive* _sysExecutive);
    virtual ~AudioSystem();

public:
    virtual bool Init();
    virtual void Run(Timer& _timer);
    virtual void Destory();

private:
    std::vector<class AAudioComponent>* mAAudioVecPtr;
    std::vector<class UAudioComponent>* mUAudioVecPtr;
};
