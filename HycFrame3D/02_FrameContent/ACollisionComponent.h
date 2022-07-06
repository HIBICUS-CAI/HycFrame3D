#pragma once

#include "ActorComponent.h"

#include "PhysicsWorld.h"

#include <DirectXMath.h>

namespace dx = DirectX;

enum class COLLISION_SHAPE {
  BOX,
  SPHERE,

  SIZE
};

class ACollisionComponent : public ActorComponent {
private:
  class btCollisionObject *CollisionObject;
  class btCollisionShape *CollisionShape;

public:
  ACollisionComponent(const std::string &CompName,
                      class ActorObject *ActorOwner);
  virtual ~ACollisionComponent();

  ACollisionComponent &
  operator=(const ACollisionComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    CollisionObject = Source.CollisionObject;
    CollisionShape = Source.CollisionShape;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool
  init();
  virtual void
  update(Timer &Timer);
  virtual void
  destory();

public:
  void
  createCollisionShape(COLLISION_SHAPE Type, dx::XMFLOAT3 Size);

  bool
  checkCollisionWith(const std::string &ActorName,
                     CONTACT_PONT_PAIR *OutContactPair = nullptr);

  static dx::XMFLOAT3
  calcCenterOfContact(const CONTACT_PONT_PAIR &Pair);

private:
  void
  addCollisionObjectToWorld();
  void
  syncDataFromTransform();
  class btCollisionObject *
  getCollisionObject() const;
};
