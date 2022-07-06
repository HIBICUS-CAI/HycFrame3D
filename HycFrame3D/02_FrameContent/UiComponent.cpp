#include "UiComponent.h"

#include "UiObject.h"

UiComponent::UiComponent(const std::string &CompName, UiObject *UiOwner)
    : Component(CompName), UiOwner(UiOwner) {}

UiComponent::~UiComponent() {}

SceneNode &
UiComponent::getSceneNode() const {
  return UiOwner->GetSceneNode();
}

UiObject *
UiComponent::getUiOwner() const {
  return UiOwner;
}

void
UiComponent::resetUiOwner(UiObject *Owner) {
  UiOwner = Owner;
}
