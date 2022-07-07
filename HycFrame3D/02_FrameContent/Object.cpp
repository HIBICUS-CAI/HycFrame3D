#include "Object.h"

#include "SceneNode.h"

Object::Object(const std::string &ObjName, SceneNode &SceneNode)
    : ObjectName(ObjName), ObjectStatus(STATUS::NEED_INIT),
      SceneNodeOwner(SceneNode) {}

Object::~Object() {}

const std::string &Object::getObjectName() const { return ObjectName; }

STATUS
Object::getObjectStatus() const { return ObjectStatus; }

void Object::setObjectStatus(STATUS ObjStatus) {
  ObjectStatus = ObjStatus;
  syncStatusToAllComps();
}

SceneNode &Object::getSceneNode() const { return SceneNodeOwner; }
