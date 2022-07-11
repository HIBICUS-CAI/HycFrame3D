#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "ACollisionComponent.h"

#include "ATransformComponent.h"
#include "ActorObject.h"
#include "PhysicsWorld.h"
#include "SceneNode.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-include"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wreorder-ctor"
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
#endif // __clang__
#include <bullet/btBulletCollisionCommon.h>
#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

using namespace dx;

ACollisionComponent::ACollisionComponent(const std::string &CompName,
                                         ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), CollisionObject(nullptr),
      CollisionShape(nullptr) {}

ACollisionComponent::~ACollisionComponent() {}

bool ACollisionComponent::init() {
  if (!CollisionObject || !CollisionShape) {
    P_LOG(LOG_ERROR, "invalid collision object in {}", getCompName());
    return false;
  }

  addCollisionObjectToWorld();

  return true;
}

void ACollisionComponent::update(const Timer &Timer) {
  syncDataFromTransform();
}

void ACollisionComponent::destory() {
  getActorOwner()->getSceneNode().getPhysicsWorld()->deleteCollisionObject(
      CollisionObject);
  delete CollisionObject;
  delete CollisionShape;
}

bool ACollisionComponent::checkCollisionWith(
    const std::string &ActorName,
    CONTACT_PONT_PAIR *OutContactPair) {
  auto Actor = getActorOwner()->getSceneNode().getActorObject(ActorName);
  if (!Actor) {
    P_LOG(LOG_MESSAGE, "doesnt exist a actor name : {}", ActorName);
    return false;
  }

  auto Acc = Actor->getComponent<ACollisionComponent>();
  if (!Acc) {
    P_LOG(LOG_WARNING, "doesnt exist a collision comp in actor name : {}",
          ActorName);
    return false;
  }

  COLLIED_PAIR Pair = {};
  if (CollisionObject < Acc->getCollisionObject()) {
    Pair = {CollisionObject, Acc->getCollisionObject()};
  } else {
    Pair = {Acc->getCollisionObject(), CollisionObject};
  }

  return getActorOwner()
      ->getSceneNode()
      .getPhysicsWorld()
      ->checkCollisionResult(Pair, OutContactPair);
}

void ACollisionComponent::createCollisionShape(COLLISION_SHAPE Type,
                                               dx::XMFLOAT3 Size) {
  switch (Type) {
  case COLLISION_SHAPE::BOX:
    CollisionShape = new btBoxShape({Size.x / 2.f, Size.y / 2.f, Size.z / 2.f});
    break;
  case COLLISION_SHAPE::SPHERE:
    CollisionShape = new btSphereShape(Size.x);
    break;
  default:
    assert(CollisionShape);
    break;
  }
  CollisionShape->setMargin(0.f);
  CollisionObject = new btCollisionObject();
  CollisionObject->setCollisionShape(CollisionShape);
}

void ACollisionComponent::addCollisionObjectToWorld() {
  getActorOwner()->getSceneNode().getPhysicsWorld()->addCollisionObject(
      CollisionObject);
}

void ACollisionComponent::syncDataFromTransform() {
  ATransformComponent *Atc =
      getActorOwner()->getComponent<ATransformComponent>();
#ifdef _DEBUG
  assert(Atc);
#endif // _DEBUG

  dx::XMFLOAT3 World = Atc->getProcessingPosition();
  dx::XMFLOAT3 Angle = Atc->getProcessingRotation();

  btTransform Trans = {};
  Trans.setIdentity();
  Trans.setOrigin(btVector3(World.x, World.y, -World.z));
  Trans.setRotation(btQuaternion(Angle.y, Angle.x, -Angle.z));
  CollisionObject->setWorldTransform(Trans);
}

btCollisionObject *ACollisionComponent::getCollisionObject() const {
  return CollisionObject;
}

dx::XMFLOAT3
ACollisionComponent::calcCenterOfContact(const CONTACT_PONT_PAIR &Pair) {
  dx::XMFLOAT3 Center = {};
  dx::XMVECTOR ContactA = dx::XMLoadFloat3(&Pair.first);
  dx::XMVECTOR ContactB = dx::XMLoadFloat3(&Pair.second);
  dx::XMVECTOR CenterV = {};
  CenterV = (ContactA + ContactB) / 2.f;
  dx::XMStoreFloat3(&Center, CenterV);
  return Center;
}
