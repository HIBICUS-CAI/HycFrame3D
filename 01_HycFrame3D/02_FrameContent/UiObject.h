#pragma once

#include "Object.h"

#include "ComponentContainer.h"
#include "ComponentGetter.h"
#include "SceneNode.h"

#include <unordered_map>

class UiObject : public Object {
private:
  std::unordered_map<COMP_TYPE, std::string> UiCompMap;

public:
  UiObject(const std::string &UiName, class SceneNode &SceneNode);
  virtual ~UiObject();

  void addUComponent(COMP_TYPE CompType);

  template <typename T>
  inline T *getComponent() {
    auto Container = getSceneNode().getComponentContainer();
    std::string Name = getObjectName();
    component_name::generateCompName<T>(Name);

    return static_cast<T *>(Container->getComponent(Name));
  }

public:
  virtual bool init();
  virtual void destory();

protected:
  virtual void syncStatusToAllComps();
};
