#include "AParticleComponent.h"
#include "ActorObject.h"
#include <vector>
#include "RSParticleEmitter.h"

AParticleComponent::AParticleComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mRSParticleEmitterPtr(nullptr)
{

}

AParticleComponent::AParticleComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mRSParticleEmitterPtr(nullptr)
{

}

AParticleComponent::~AParticleComponent()
{

}

bool AParticleComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void AParticleComponent::Update(Timer& _timer)
{

}

void AParticleComponent::Destory()
{

}

void AParticleComponent::CreateEmitter(PARTICLE_EMITTER_INFO* _lightInfo)
{

}

void AParticleComponent::ResetEmitter(PARTICLE_EMITTER_INFO* _lightInfo)
{

}

const RS_PARTICLE_EMITTER_INFO& AParticleComponent::GetEmitterInfo()
{
#ifdef _DEBUG
    assert(mRSParticleEmitterPtr);
#endif // _DEBUG
    return mRSParticleEmitterPtr->GetRSParticleEmitterInfo();
}
