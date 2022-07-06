#include "AParticleComponent.h"

#include "ATransformComponent.h"
#include "ActorObject.h"

#include <RSParticleEmitter.h>
#include <RSParticlesContainer.h>
#include <RSRoot_DX11.h>

#include <vector>

AParticleComponent::AParticleComponent(const std::string &CompName,
                                       ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), RSParticleEmitterPtr(nullptr) {}

AParticleComponent::~AParticleComponent() {}

bool
AParticleComponent::init() {
  if (RSParticleEmitterPtr) {
    return true;
  } else {
    return false;
  }
}

void
AParticleComponent::update(Timer &Timer) {
  syncDataFromTransform();
}

void
AParticleComponent::destory() {
  getRSDX11RootInstance()->getParticlesContainer()->deleteRSParticleEmitter(
      getCompName());
}

void
AParticleComponent::createEmitter(const PARTICLE_EMITTER_INFO *EmitterInfo) {
  RSParticleEmitterPtr =
      getRSDX11RootInstance()->getParticlesContainer()->createRSParticleEmitter(
          getCompName(), EmitterInfo);
}

void
AParticleComponent::resetEmitter(const PARTICLE_EMITTER_INFO *EmitterInfo) {
  RSParticleEmitterPtr->resetParticleEmitterInfo(EmitterInfo);
}

RS_PARTICLE_EMITTER_INFO &
AParticleComponent::getEmitterInfo() {
#ifdef _DEBUG
  assert(RSParticleEmitterPtr);
#endif // _DEBUG
  return RSParticleEmitterPtr->getRSParticleEmitterInfo();
}

void
AParticleComponent::syncDataFromTransform() {
  ATransformComponent *Atc =
      getActorOwner()->GetComponent<ATransformComponent>();
#ifdef _DEBUG
  assert(Atc);
  assert(RSParticleEmitterPtr);
#endif // _DEBUG

  DirectX::XMFLOAT3 World = Atc->getPosition();
  RSParticleEmitterPtr->setEmitterPosition(World);
}
