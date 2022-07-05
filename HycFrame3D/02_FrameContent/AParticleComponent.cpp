#include "AParticleComponent.h"
#include "ActorObject.h"
#include "ATransformComponent.h"
#include <vector>
#include "RSParticleEmitter.h"
#include "RSRoot_DX11.h"
#include "RSParticlesContainer.h"

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
    if (mRSParticleEmitterPtr) { return true; }
    else { return false; }
}

void AParticleComponent::Update(Timer& _timer)
{
    SyncDataFromTransform();
}

void AParticleComponent::Destory()
{
    std::string name = GetCompName();
    getRSDX11RootInstance()->getParticlesContainer()->
        deleteRSParticleEmitter(name);
}

void AParticleComponent::CreateEmitter(PARTICLE_EMITTER_INFO* _emitterInfo)
{
    std::string name = GetCompName();
    mRSParticleEmitterPtr = getRSDX11RootInstance()->getParticlesContainer()->
        createRSParticleEmitter(name, _emitterInfo);
}

void AParticleComponent::ResetEmitter(PARTICLE_EMITTER_INFO* _emitterInfo)
{
    mRSParticleEmitterPtr->resetParticleEmitterInfo(_emitterInfo);
}

RS_PARTICLE_EMITTER_INFO& AParticleComponent::GetEmitterInfo()
{
#ifdef _DEBUG
    assert(mRSParticleEmitterPtr);
#endif // _DEBUG
    return mRSParticleEmitterPtr->getRSParticleEmitterInfo();
}

void AParticleComponent::SyncDataFromTransform()
{
    ATransformComponent* atc = GetActorOwner()->
        GetComponent<ATransformComponent>();
#ifdef _DEBUG
    assert(atc);
    assert(mRSParticleEmitterPtr);
#endif // _DEBUG

    DirectX::XMFLOAT3 world = atc->GetPosition();
    mRSParticleEmitterPtr->setEmitterPosition(world);
}
