#include "Object.h"
#include "SceneNode.h"

Object::Object(std::string&& _objName, SceneNode& _sceneNode) :
    mObjectName(_objName), mSceneNodeOwner(_sceneNode),
    mObjectStatus(STATUS::NEED_INIT)
{

}

Object::Object(std::string& _objName, SceneNode& _sceneNode) :
    mObjectName(_objName), mSceneNodeOwner(_sceneNode),
    mObjectStatus(STATUS::NEED_INIT)
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
}

SceneNode& Object::GetSceneNode() const
{
    return mSceneNodeOwner;
}
