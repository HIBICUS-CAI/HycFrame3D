#pragma once

#include "ActorComponent.h"
#include <vector>
#include <DirectXMath.h>

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
    void AddMeshInfo(std::string&& _meshName,
        DirectX::XMFLOAT3 _offset = { 0.f,0.f,0.f });
    void AddMeshInfo(std::string& _meshName,
        DirectX::XMFLOAT3 _offset = { 0.f,0.f,0.f });

private:
    bool BindInstanceToAssetsPool(std::string& _meshName);
    void SyncTransformDataToInstance();

private:
    friend class AAnimateComponent;
    std::vector<std::string> mMeshesName;
    std::vector<std::string> mSubMeshesName;
    std::vector<DirectX::XMFLOAT3> mOffsetPosition;
};
