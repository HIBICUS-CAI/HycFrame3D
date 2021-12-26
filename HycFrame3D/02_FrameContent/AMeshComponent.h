#pragma once

#include "ActorComponent.h"
#include <vector>

class AMeshComponent :public ActorComponent
{
public:
    AMeshComponent(std::string&& _compName, class ActorObject* _actorOwner);
    AMeshComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~AMeshComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    bool BindInstanceToAssetsPool(std::string&& _meshName);
    bool BindInstanceToAssetsPool(std::string& _meshName);

private:
    void SyncTransformDataToInstance();

private:
    std::vector<std::string> mMeshesName;
    std::vector<size_t> mInstancesIndex;
};
