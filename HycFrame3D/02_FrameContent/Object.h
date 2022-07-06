#pragma once

#include "Hyc3DCommon.h"

#include <string>

class Object {
private:
  const std::string ObjectName;
  STATUS ObjectStatus;
  class SceneNode &SceneNodeOwner;

public:
  Object(const std::string &ObjName, class SceneNode &SceneNode);
  virtual ~Object();

  const std::string &
  getObjectName() const;

  STATUS
  getObjectStatus() const;
  void
  setObjectStatus(STATUS ObjStatus);

  class SceneNode &
  getSceneNode() const;

public:
  virtual bool
  init() = 0;
  virtual void
  destory() = 0;

protected:
  virtual void
  syncStatusToAllComps() = 0;
};
