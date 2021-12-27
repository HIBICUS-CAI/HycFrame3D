#pragma once

#include "Hyc3DCommon.h"
#include <set>

using COLLIED_PAIR = std::pair<class btCollisionObject*,
    class btCollisionObject*>;

class PhysicsWorld
{
public:
    PhysicsWorld(class SceneNode& _sceneNode);
    ~PhysicsWorld();

    void CreatePhysicsWorld();

    void AddCollisionObject(class btCollisionObject* _colliObj);
    void DeleteCollisionObject(class btCollisionObject* _colliObj);

    void DetectCollision();

    bool CheckCollisionResult(COLLIED_PAIR& _pair0, COLLIED_PAIR& _pair1);

    void DeletePhysicsWorld();

private:
    const class SceneNode& mSceneNodeOwner;

    std::set<COLLIED_PAIR> mColliedPair;

    class btCollisionWorld* mCollisionWorld;

    class btDefaultCollisionConfiguration* mCollisionConfig;
    class btCollisionDispatcher* mCollisionDispatcher;
    class btBroadphaseInterface* mBroadphaseInterface;
};
