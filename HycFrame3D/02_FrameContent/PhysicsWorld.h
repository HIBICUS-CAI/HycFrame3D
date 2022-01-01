#pragma once

#include "Hyc3DCommon.h"
#include <set>
#include <map>
#include <DirectXMath.h>

using COLLIED_PAIR = std::pair<class btCollisionObject*,
    class btCollisionObject*>;
using CONTACT_PONT_PAIR = std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3>;

class PhysicsWorld
{
public:
    PhysicsWorld(class SceneNode& _sceneNode);
    ~PhysicsWorld();

    void CreatePhysicsWorld();

    void AddCollisionObject(class btCollisionObject* _colliObj);
    void DeleteCollisionObject(class btCollisionObject* _colliObj);

    void DetectCollision();

    bool CheckCollisionResult(COLLIED_PAIR& _pair,
        CONTACT_PONT_PAIR* _contactPair);

    void DeletePhysicsWorld();

private:
    const class SceneNode& mSceneNodeOwner;

    std::set<COLLIED_PAIR> mColliedPair;
    std::map<COLLIED_PAIR, CONTACT_PONT_PAIR> mContactPointMap;

    class btCollisionWorld* mCollisionWorld;

    class btDefaultCollisionConfiguration* mCollisionConfig;
    class btCollisionDispatcher* mCollisionDispatcher;
    class btBroadphaseInterface* mBroadphaseInterface;
};
