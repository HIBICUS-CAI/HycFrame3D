#include "AAnimateComponent.h"
#include "ActorObject.h"

AAnimateComponent::AAnimateComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner)
{

}

AAnimateComponent::AAnimateComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner)
{

}

AAnimateComponent::~AAnimateComponent()
{

}

bool AAnimateComponent::Init()
{
    return true;
}

void AAnimateComponent::Update(Timer& _timer)
{

}

void AAnimateComponent::Destory()
{

}
