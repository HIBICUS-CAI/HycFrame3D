#pragma once

#include "ActorComponent.h"

class AParticleComponent : public ActorComponent {
private:
  class RSParticleEmitter *RSParticleEmitterPtr;

public:
  AParticleComponent(const std::string &CompName,
                     class ActorObject *ActorOwner);
  virtual ~AParticleComponent();

  AParticleComponent &
  operator=(const AParticleComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    RSParticleEmitterPtr = Source.RSParticleEmitterPtr;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool
  init();
  virtual void
  update(Timer &Timer);
  virtual void
  destory();

public:
  void
  createEmitter(const struct PARTICLE_EMITTER_INFO *EmitterInfo);
  void
  resetEmitter(const struct PARTICLE_EMITTER_INFO *EmitterInfo);

  struct RS_PARTICLE_EMITTER_INFO &
  getEmitterInfo();

private:
  void
  syncDataFromTransform();
};
