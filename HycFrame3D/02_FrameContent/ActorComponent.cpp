#include "ActorComponent.h"

#include "ActorObject.h"

ActorComponent::ActorComponent(const std::string &CompName,
                               ActorObject *ActorOwner)
    : Component(CompName), ActorOwner(ActorOwner) {}

ActorComponent::~ActorComponent() {}

SceneNode &
ActorComponent::getSceneNode() const {
  return ActorOwner->getSceneNode();
}

ActorObject *
ActorComponent::getActorOwner() const {
  return ActorOwner;
}

void
ActorComponent::resetActorOwner(ActorObject *Owner) {
  ActorOwner = Owner;
}
