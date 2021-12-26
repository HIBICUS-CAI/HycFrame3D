#include "AMeshComponent.h"
#include "ActorComponent.h"

AMeshComponent::AMeshComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mMeshesName({}), mInstancesIndex({})
{

}

AMeshComponent::AMeshComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mMeshesName({}), mInstancesIndex({})
{

}

AMeshComponent::~AMeshComponent()
{

}

bool AMeshComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void AMeshComponent::Update(Timer& _timer)
{

}

void AMeshComponent::Destory()
{

}

bool AMeshComponent::BindInstanceToAssetsPool(std::string&& _meshName)
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

bool AMeshComponent::BindInstanceToAssetsPool(std::string& _meshName)
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void AMeshComponent::SyncTransformDataToInstance()
{

}
