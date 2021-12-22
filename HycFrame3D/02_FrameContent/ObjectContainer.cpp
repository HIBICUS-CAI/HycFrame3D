#include "ObjectContainer.h"
#include "SceneNode.h"

ObjectContainer::ObjectContainer(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode),
    mActorObjMap({}), mUiObjMap({})/*, mActorObjVector({}), mUiObjVector({}),
    mNewActorObjVector({}), mNewUiObjVector({}),
    mDeadActorObjVector({}), mDeadUiObjVector({})*/
{

}

ObjectContainer::~ObjectContainer()
{

}

ActorObject* ObjectContainer::GetActorObject(std::string&& _actorName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

ActorObject* ObjectContainer::GetActorObject(std::string& _actorName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

void ObjectContainer::AddActorObject(ActorObject& _newActor)
{

}

void ObjectContainer::DeleteActorObject(std::string&& _actorName)
{

}

void ObjectContainer::DeleteActorObject(std::string& _actorName_actorName)
{

}

UiObject* ObjectContainer::GetUiObject(std::string&& _uiName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

UiObject* ObjectContainer::GetUiObject(std::string& _uiName)
{
    // TEMP---------------------------
    return nullptr;
    // TEMP---------------------------
}

void ObjectContainer::AddUiObject(UiObject& _newUi)
{

}

void ObjectContainer::DeleteUiObject(std::string&& _uiName)
{

}

void ObjectContainer::DeleteUiObject(std::string& _uiName)
{

}

void ObjectContainer::DeleteAllActor()
{

}

void ObjectContainer::DeleteAllUi()
{

}


void ObjectContainer::InitAllNewObjects()
{

}

void ObjectContainer::DeleteAllDeadObjects()
{

}
