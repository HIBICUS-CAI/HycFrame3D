#pragma once

#include "ActorComponent.h"

class AParticleComponent :public ActorComponent
{
public:
    AParticleComponent(std::string&& _compName, class ActorObject* _actorOwner);
    AParticleComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~AParticleComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void CreateEmitter(struct PARTICLE_EMITTER_INFO* _emitterInfo);
    void ResetEmitter(struct PARTICLE_EMITTER_INFO* _emitterInfo);

    struct RS_PARTICLE_EMITTER_INFO& GetEmitterInfo();

private:
    void SyncDataFromTransform();

private:
    class RSParticleEmitter* mRSParticleEmitterPtr;
};
