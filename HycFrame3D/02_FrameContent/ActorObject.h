#pragma once

#include "Object.h"

#include "ComponentContainer.h"
#include "ComponentGetter.h"
#include "SceneNode.h"

#include <unordered_map>

class ActorObject : public Object {
private:
  std::unordered_map<COMP_TYPE, std::string> ActorCompMap;

public:
  ActorObject(const std::string &ActorName, class SceneNode &SceneNode);
  virtual ~ActorObject();

  void
  addAComponent(COMP_TYPE CompType);

  template <typename T>
  inline T *
  getComponent() {
    auto Container = getSceneNode().GetComponentContainer();
    std::string Name = getObjectName();
    component_name::generateCompName<T>(Name);

    return static_cast<T *>(Container->getComponent(Name));
  }

public:
  virtual bool
  init();
  virtual void
  destory();

protected:
  virtual void
  syncStatusToAllComps();
};
