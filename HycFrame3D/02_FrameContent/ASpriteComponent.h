#pragma once

#include "ActorComponent.h"
#include <vector>

class ASpriteComponent :public ActorComponent
{
public:
    ASpriteComponent(std::string&& _compName, class ActorObject* _actorOwner);
    ASpriteComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~ASpriteComponent();

    ASpriteComponent& operator=(const ASpriteComponent& _source)
    {
        if (this == &_source) { return *this; }
        ActorComponent::operator=(_source);
        return *this;
    }

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:

private:
    //void SyncTransformDataToInstance();

private:

};
