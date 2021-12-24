#include "ALightComponent.h"
#include "ActorObject.h"
#include <vector>
#include "RSLight.h"

ALightComponent::ALightComponent(std::string&& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner), mRSLightPtr(nullptr)
{

}

ALightComponent::ALightComponent(std::string& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner), mRSLightPtr(nullptr)
{

}

ALightComponent::~ALightComponent()
{

}

bool ALightComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void ALightComponent::Update(Timer& _timer)
{

}

void ALightComponent::Destory()
{

}

void ALightComponent::CreateLight(LIGHT_INFO* _lightInfo)
{

}

void ALightComponent::ResetLight(LIGHT_INFO* _lightInfo)
{

}

const RS_LIGHT_INFO* ALightComponent::GetLightInfo()
{
#ifdef _DEBUG
    assert(mRSLightPtr);
#endif // _DEBUG
    return mRSLightPtr->GetRSLightInfo();
}

void ALightComponent::SyncDataFromTransform()
{

}
