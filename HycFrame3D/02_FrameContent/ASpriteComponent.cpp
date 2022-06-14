#include "ASpriteComponent.h"
#include "ActorObject.h"
#include "SceneNode.h"

ASpriteComponent::ASpriteComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner)
{

}

ASpriteComponent::ASpriteComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner)
{

}

ASpriteComponent::~ASpriteComponent()
{

}

bool ASpriteComponent::Init()
{
    return true;
}

void ASpriteComponent::Update(Timer& _timer)
{

}

void ASpriteComponent::Destory()
{

}
