#pragma once

#include "ActorComponent.h"
#include <DirectXMath.h>

class ALightComponent :public ActorComponent
{
public:
    ALightComponent(std::string&& _compName, class ActorObject* _actorOwner);
    ALightComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~ALightComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void CreateLight(struct LIGHT_INFO* _lightInfo);
    void ResetLight(struct LIGHT_INFO* _lightInfo);

    const struct RS_LIGHT_INFO* GetLightInfo();

private:
    void SyncDataFromTransform();

private:
    class RSLight* mRSLightPtr;
};
