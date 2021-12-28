#pragma once

#include "ActorComponent.h"
#include <DirectXMath.h>
#include "RSCommon.h"

constexpr auto BOX_BLOOM_MESH_NAME = "box-bloom-mesh";

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
    void AddLight(LIGHT_INFO& _lightInfo,
        bool _setBloom, bool _setCamera, CAM_INFO& _camInfo);
    void AddLight(LIGHT_INFO& _lightInfo,
        bool _setBloom, bool _setCamera, CAM_INFO&& _camInfo);

    void ResetLight(LIGHT_INFO* _lightInfo);

    class RSLight* GetLightInfo();

private:
    void CreateLight();
    void SyncDataFromTransform();

private:
    std::string mLightName;
    class RSLight* mRSLightPtr;

    bool mCanCreateLight;
    LIGHT_INFO mLightInfoForInit;
    CAM_INFO mLightCamInfoForInit;
    bool mIsBloom;
    bool mIsCamera;
};
