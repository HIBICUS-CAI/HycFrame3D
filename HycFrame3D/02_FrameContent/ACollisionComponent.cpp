#include "ACollisionComponent.h"
#include "ActorObject.h"

ACollisionComponent::ACollisionComponent(std::string&& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mCollisionObject(nullptr), mCollisionShape(nullptr)
{

}

ACollisionComponent::ACollisionComponent(std::string& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mCollisionObject(nullptr), mCollisionShape(nullptr)
{

}

ACollisionComponent::~ACollisionComponent()
{

}

bool ACollisionComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void ACollisionComponent::Update(Timer& _timer)
{

}

void ACollisionComponent::Destory()
{

}

bool ACollisionComponent::CheckCollisionWith(std::string&& _actorName)
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

bool ACollisionComponent::CheckCollisionWith(std::string& _actorName)
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void ACollisionComponent::CheckCollisionShape(COLLISION_SHAPE _type, DirectX::XMFLOAT3 _size)
{

}

void ACollisionComponent::AddCollisionObjectToWorld()
{

}

void ACollisionComponent::SyncDataFromTransform()
{

}
