#pragma once

#include "Hyc3DCommon.h"

#include <directxmath.h>
#include <map>
#include <set>

namespace dx = DirectX;

using COLLIED_PAIR =
    std::pair<const class btCollisionObject *, const class btCollisionObject *>;
using CONTACT_PONT_PAIR = std::pair<dx::XMFLOAT3, dx::XMFLOAT3>;

class PhysicsWorld {
private:
  const class SceneNode &SceneNodeOwner;

  std::set<COLLIED_PAIR> ColliedPair;
  std::map<COLLIED_PAIR, CONTACT_PONT_PAIR> ContactPointMap;

  class btCollisionWorld *CollisionWorld;

  class btDefaultCollisionConfiguration *CollisionConfig;
  class btCollisionDispatcher *CollisionDispatcher;
  class btBroadphaseInterface *BroadphaseInterface;

public:
  PhysicsWorld(class SceneNode &SceneNode);
  ~PhysicsWorld();

  void createPhysicsWorld();

  void addCollisionObject(class btCollisionObject *ColliObj);
  void deleteCollisionObject(class btCollisionObject *ColliObj);

  void detectCollision();

  bool checkCollisionResult(const COLLIED_PAIR &Pair,
                            CONTACT_PONT_PAIR *OutContactPair);

  void deletePhysicsWorld();
};
