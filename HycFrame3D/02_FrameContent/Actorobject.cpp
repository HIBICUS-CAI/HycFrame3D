#include "ActorObject.h"
#include "SceneNode.h"

ActorObject::ActorObject(std::string&& _actorName, SceneNode& _sceneNode) :
    Object(_actorName, _sceneNode), mActorCompMap({})
{

}

ActorObject::ActorObject(std::string& _actorName, SceneNode& _sceneNode) :
    Object(_actorName, _sceneNode), mActorCompMap({})
{

}

ActorObject::~ActorObject()
{

}

void ActorObject::AddAComponent(COMP_TYPE _compType)
{

}

bool ActorObject::Init()
{
    // TEMP-----------------------------
    return true;
    // TEMP-----------------------------
}

void ActorObject::Destory()
{

}
