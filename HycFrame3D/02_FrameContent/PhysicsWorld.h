#pragma once

#include "Hyc3DCommon.h"

class PhysicsWorld
{
public:
    PhysicsWorld(class SceneNode& _sceneNode);
    ~PhysicsWorld();

    void CreatePhysicsWorld();

    void AddCollisionObject();

    void DetectCollision();

    void DeletePhysicsWorld();

private:
    const class SceneNode& mSceneNodeOwner;

    class btCollisionWorld* mCollisionWorld;
};
