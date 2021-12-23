#pragma once

#include "Component.h"

class ActorComponent :public Component
{
public:
    ActorComponent(std::string&& _compName, class ActorObject& _actorOwner);
    ActorComponent(std::string& _compName, class ActorObject& _actorOwner);
    virtual ~ActorComponent();

    class ActorObject& GetActorOwner() const;

public:
    virtual bool Init() = 0;
    virtual void Update(Timer& _timer) = 0;
    virtual void Destory() = 0;

private:
    class ActorObject& mActorOwner;
};
