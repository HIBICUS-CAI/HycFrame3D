#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "ACollisionComponent.h"

#include "ATransformComponent.h"
#include "ActorObject.h"
#include "PhysicsWorld.h"
#include "SceneNode.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-include"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wreorder-ctor"
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
#include <bullet/btBulletCollisionCommon.h>
#pragma clang diagnostic pop

using namespace dx;

ACollisionComponent::ACollisionComponent(const std::string &CompName,
                                         ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), CollisionObject(nullptr),
      CollisionShape(nullptr) {}

ACollisionComponent::~ACollisionComponent() {}

bool
ACollisionComponent::init() {
  if (!CollisionObject || !CollisionShape) {
    P_LOG(LOG_ERROR, "invalid collision object in %s\n", getCompName().c_str());
    return false;
  }

  addCollisionObjectToWorld();

  return true;
}

void
ACollisionComponent::update(Timer &Timer) {
  syncDataFromTransform();
}

void
ACollisionComponent::destory() {
  getActorOwner()->getSceneNode().GetPhysicsWorld()->DeleteCollisionObject(
      CollisionObject);
  delete CollisionObject;
  delete CollisionShape;
}

bool
ACollisionComponent::checkCollisionWith(const std::string &ActorName,
                                        CONTACT_PONT_PAIR *ContactPair) {
  auto Actor = getActorOwner()->getSceneNode().GetActorObject(ActorName);
  if (!Actor) {
    P_LOG(LOG_ERROR, "doesnt exist a actor name : %s\n", ActorName.c_str());
    return false;
  }

  auto Acc = Actor->getComponent<ACollisionComponent>();
  if (!Acc) {
    P_LOG(LOG_ERROR, "doesnt exist a collision comp in actor name : %s\n",
          ActorName.c_str());
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
      .GetPhysicsWorld()
      ->CheckCollisionResult(Pair, ContactPair);
}

void
ACollisionComponent::createCollisionShape(COLLISION_SHAPE Type,
                                          DirectX::XMFLOAT3 Size) {
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

void
ACollisionComponent::addCollisionObjectToWorld() {
  getActorOwner()->getSceneNode().GetPhysicsWorld()->AddCollisionObject(
      CollisionObject);
}

void
ACollisionComponent::syncDataFromTransform() {
  ATransformComponent *Atc =
      getActorOwner()->getComponent<ATransformComponent>();
#ifdef _DEBUG
  assert(Atc);
#endif // _DEBUG

  DirectX::XMFLOAT3 World = Atc->getProcessingPosition();
  DirectX::XMFLOAT3 Angle = Atc->getProcessingRotation();

  btTransform Trans = {};
  Trans.setIdentity();
  Trans.setOrigin(btVector3(World.x, World.y, -World.z));
  Trans.setRotation(btQuaternion(Angle.y, Angle.x, -Angle.z));
  CollisionObject->setWorldTransform(Trans);
}

btCollisionObject *
ACollisionComponent::getCollisionObject() const {
  return CollisionObject;
}

DirectX::XMFLOAT3
ACollisionComponent::calcCenterOfContact(const CONTACT_PONT_PAIR &Pair) {
  DirectX::XMFLOAT3 Center = {};
  DirectX::XMVECTOR ContactA = DirectX::XMLoadFloat3(&Pair.first);
  DirectX::XMVECTOR ContactB = DirectX::XMLoadFloat3(&Pair.second);
  DirectX::XMVECTOR CenterV = {};
  CenterV = (ContactA + ContactB) / 2.f;
  DirectX::XMStoreFloat3(&Center, CenterV);
  return Center;
}
