#pragma once

#include "Component.h"

class ActorComponent : public Component {
private:
  class ActorObject *ActorOwner;

public:
  ActorComponent(const std::string &CompName, class ActorObject *ActorOwner);
  virtual ~ActorComponent();

  ActorComponent &
  operator=(const ActorComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    ActorOwner = Source.ActorOwner;
    Component::operator=(Source);
    return *this;
  }

  class SceneNode &
  getSceneNode() const;

  class ActorObject *
  getActorOwner() const;

  void
  resetActorOwner(class ActorObject *Owner);

public:
  virtual bool
  init() = 0;

  virtual void
  update(Timer &Timer) = 0;

  virtual void
  destory() = 0;
};
