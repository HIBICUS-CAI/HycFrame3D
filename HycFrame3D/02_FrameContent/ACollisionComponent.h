#pragma once

#include "ActorComponent.h"
#include <DirectXMath.h>

enum class COLLISION_SHAPE
{
    BOX,
    SPHERE,

    SIZE
};

class ACollisionComponent :public ActorComponent
{
public:
    ACollisionComponent(std::string&& _compName, class ActorObject* _actorOwner);
    ACollisionComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~ACollisionComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void CreateCollisionShape(COLLISION_SHAPE _type, DirectX::XMFLOAT3 _size);

    bool CheckCollisionWith(std::string&& _actorName);
    bool CheckCollisionWith(std::string& _actorName);

private:
    void AddCollisionObjectToWorld();
    void SyncDataFromTransform();
    class btCollisionObject* GetCollisionObject() const;

private:
    class btCollisionObject* mCollisionObject;
    class btCollisionShape* mCollisionShape;
};
