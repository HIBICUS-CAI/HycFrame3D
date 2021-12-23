#include "ActorComponent.h"
#include "ActorObject.h"

ActorComponent::ActorComponent(std::string&& _compName, ActorObject& _actorOwner) :
    Component(_compName), mActorOwner(_actorOwner)
{

}

ActorComponent::ActorComponent(std::string& _compName, ActorObject& _actorOwner) :
    Component(_compName), mActorOwner(_actorOwner)
{

}

ActorComponent::~ActorComponent()
{

}

ActorObject& ActorComponent::GetActorOwner() const
{
    return mActorOwner;
}
