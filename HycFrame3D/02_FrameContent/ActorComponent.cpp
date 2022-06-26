#include "ActorComponent.h"
#include "ActorObject.h"

ActorComponent::ActorComponent(std::string&& _compName, ActorObject* _actorOwner) :
    Component(_compName), mActorOwner(_actorOwner)
{

}

ActorComponent::ActorComponent(std::string& _compName, ActorObject* _actorOwner) :
    Component(_compName), mActorOwner(_actorOwner)
{

}

ActorComponent::~ActorComponent()
{

}

SceneNode& ActorComponent::GetSceneNode() const
{
    return mActorOwner->GetSceneNode();
}

ActorObject* ActorComponent::GetActorOwner() const
{
    return mActorOwner;
}

void ActorComponent::ResetActorOwner(ActorObject* _owner)
{
    mActorOwner = _owner;
}
