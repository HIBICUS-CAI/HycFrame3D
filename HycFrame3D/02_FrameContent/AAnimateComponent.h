#pragma once

#include "ActorComponent.h"

class AAnimateComponent :public ActorComponent
{
public:
    AAnimateComponent(std::string&& _compName, class ActorObject* _actorOwner);
    AAnimateComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~AAnimateComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:


private:

};
