#pragma once

#include "Component.h"

class ActorComponent :public Component
{
public:
    ActorComponent(std::string&& _compName, class ActorObject* _actorOwner);
    ActorComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~ActorComponent();

    ActorComponent& operator=(const ActorComponent& _source)
    {
        if (this == &_source) { return *this; }
        mActorOwner = _source.mActorOwner;
        Component::operator=(_source);
        return *this;
    }

    class ActorObject* GetActorOwner() const;
    void ResetActorOwner(class ActorObject* _owner);

public:
    virtual bool Init() = 0;
    virtual void Update(Timer& _timer) = 0;
    virtual void Destory() = 0;

private:
    class ActorObject* mActorOwner;
};
