#include "Object.h"
#include "SceneNode.h"

Object::Object(std::string&& _objName, SceneNode& _sceneNode) :
    mObjectName(_objName),
    mObjectStatus(STATUS::NEED_INIT),
    mSceneNodeOwner(_sceneNode)
{

}

Object::Object(std::string& _objName, SceneNode& _sceneNode) :
    mObjectName(_objName),
    mObjectStatus(STATUS::NEED_INIT),
    mSceneNodeOwner(_sceneNode)
{

}

Object::~Object()
{

}

const std::string& Object::GetObjectName() const
{
    return mObjectName;
}

STATUS Object::GetObjectStatus() const
{
    return mObjectStatus;
}

void Object::SetObjectStatus(STATUS _objStatus)
{
    mObjectStatus = _objStatus;
    SyncStatusToAllComps();
}

SceneNode& Object::GetSceneNode() const
{
    return mSceneNodeOwner;
}
