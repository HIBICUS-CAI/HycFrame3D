#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "PhysicsWorld.h"
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-include"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wreorder-ctor"
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
#endif // __clang__
#include "bullet/btBulletCollisionCommon.h"
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

using namespace dx;

PhysicsWorld::PhysicsWorld(class SceneNode &SceneNode)
    : SceneNodeOwner(SceneNode), ColliedPair({}), ContactPointMap({}),
      CollisionWorld(nullptr), CollisionConfig(nullptr),
      CollisionDispatcher(nullptr), BroadphaseInterface(nullptr) {
  createPhysicsWorld();
}

PhysicsWorld::~PhysicsWorld() {}

void PhysicsWorld::createPhysicsWorld() {
  CollisionConfig = new btDefaultCollisionConfiguration();
  CollisionDispatcher = new btCollisionDispatcher(CollisionConfig);
  BroadphaseInterface = new btDbvtBroadphase();

  CollisionWorld = new btCollisionWorld(CollisionDispatcher,
                                        BroadphaseInterface, CollisionConfig);

#ifdef _DEBUG
  assert(CollisionConfig && CollisionDispatcher && BroadphaseInterface &&
         CollisionWorld);
#endif // _DEBUG
}

void PhysicsWorld::addCollisionObject(btCollisionObject *ColliObj) {
  if (!ColliObj) {
    P_LOG(LOG_ERROR, "passing a null collision object to collision world");
    return;
  }

  CollisionWorld->addCollisionObject(ColliObj);
}

void PhysicsWorld::deleteCollisionObject(btCollisionObject *ColliObj) {
  if (!ColliObj) {
    P_LOG(LOG_ERROR, "searching a null collision object in collision world");
    return;
  }

  CollisionWorld->removeCollisionObject(ColliObj);
}

void PhysicsWorld::detectCollision() {
  ColliedPair.clear();
  ContactPointMap.clear();
  CollisionWorld->performDiscreteCollisionDetection();

  int NumManifolds = CollisionWorld->getDispatcher()->getNumManifolds();

  for (int I = 0; I < NumManifolds; I++) {
    btPersistentManifold *ContactManifold =
        CollisionWorld->getDispatcher()->getManifoldByIndexInternal(I);
    const btCollisionObject *ObjA = ContactManifold->getBody0();
    const btCollisionObject *ObjB = ContactManifold->getBody1();
    int NumContacts = ContactManifold->getNumContacts();

    COLLIED_PAIR Pair = {};
    if (ObjA < ObjB) {
      Pair = {ObjA, ObjB};
    } else {
      Pair = {ObjB, ObjA};
    }

    if (NumContacts && ColliedPair.find(Pair) == ColliedPair.end()) {
      ColliedPair.insert(Pair);
    }

    for (int J = 0; J < NumContacts; J++) {
      btManifoldPoint &Point = ContactManifold->getContactPoint(J);
      if (Point.getDistance() <= 0.f) {
        btVector3 PosA = Point.getPositionWorldOnA();
        btVector3 PosB = Point.getPositionWorldOnB();
        CONTACT_PONT_PAIR ContPair = {{PosA.getX(), PosA.getY(), -PosA.getZ()},
                                      {PosB.getX(), PosB.getY(), -PosB.getZ()}};
        auto Found = ContactPointMap.find(Pair);
        if (Found == ContactPointMap.end()) {
          ContactPointMap.insert({Pair, ContPair});
        } else {
          dx::XMVECTOR BeforeA = dx::XMLoadFloat3(&Found->second.first);
          dx::XMVECTOR NewoneA = dx::XMLoadFloat3(&ContPair.first);
          dx::XMVECTOR BeforeB = dx::XMLoadFloat3(&Found->second.second);
          dx::XMVECTOR NewoneB = dx::XMLoadFloat3(&ContPair.second);
          BeforeA += NewoneA;
          BeforeB += NewoneB;
          dx::XMStoreFloat3(&Found->second.first, BeforeA);
          dx::XMStoreFloat3(&Found->second.second, BeforeB);
        }
      }
    }
    if (NumContacts) {
      auto Found = ContactPointMap.find(Pair);
      if (Found == ContactPointMap.end()) {
        ColliedPair.erase(Pair);
        continue;
      }
      dx::XMVECTOR ContactA = dx::XMLoadFloat3(&Found->second.first);
      dx::XMVECTOR ContactB = dx::XMLoadFloat3(&Found->second.second);
      ContactA /= static_cast<float>(NumContacts);
      ContactB /= static_cast<float>(NumContacts);
      dx::XMStoreFloat3(&Found->second.first, ContactA);
      dx::XMStoreFloat3(&Found->second.second, ContactB);
    }
  }
}

void PhysicsWorld::deletePhysicsWorld() {
  delete CollisionWorld;
  delete BroadphaseInterface;
  delete CollisionDispatcher;
  delete CollisionConfig;
}

bool PhysicsWorld::checkCollisionResult(const COLLIED_PAIR &_pair,
                                        CONTACT_PONT_PAIR *_contactPair) {
  if (ColliedPair.find(_pair) == ColliedPair.end()) {
    return false;
  } else {
    if (_contactPair) {
      *_contactPair = ContactPointMap.find(_pair)->second;
    }
    return true;
  }
}
