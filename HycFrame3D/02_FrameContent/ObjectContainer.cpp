#include "ObjectContainer.h"
#include "SceneNode.h"
#include "ActorObject.h"
#include "UiObject.h"

ObjectContainer::ObjectContainer(SceneNode& _sceneNode) :
    mSceneNodeOwner(_sceneNode),
    mActorObjMap({}), mUiObjMap({}),
    mNewActorObjVector({}), mNewUiObjVector({}),
    mDeadActorObjVector({}), mDeadUiObjVector({})
{

}

ObjectContainer::~ObjectContainer()
{

}

ActorObject* ObjectContainer::GetActorObject(std::string&& _actorName)
{
    auto found = mActorObjMap.find(_actorName);
    if (found != mActorObjMap.end())
    {
        return &(found->second);
    }
    else
    {
        P_LOG(LOG_WARNING,
            "cannot find actor object name : %s\n",
            _actorName.c_str());
        return nullptr;
    }
}

ActorObject* ObjectContainer::GetActorObject(std::string& _actorName)
{
    auto found = mActorObjMap.find(_actorName);
    if (found != mActorObjMap.end())
    {
        return &(found->second);
    }
    else
    {
        P_LOG(LOG_WARNING,
            "cannot find actor object name : %s\n",
            _actorName.c_str());
        return nullptr;
    }
}

void ObjectContainer::AddActorObject(ActorObject& _newActor)
{
    mNewActorObjVector.emplace_back(_newActor);
}

void ObjectContainer::DeleteActorObject(std::string&& _actorName)
{
    auto found = mActorObjMap.find(_actorName);
    if (found != mActorObjMap.end())
    {
        mDeadActorObjVector.emplace_back(found->second);
        mActorObjMap.erase(_actorName);
    }
    else
    {
        P_LOG(LOG_WARNING,
            "cannot find actor object name : %s\n",
            _actorName.c_str());
    }
}

void ObjectContainer::DeleteActorObject(std::string& _actorName)
{
    auto found = mActorObjMap.find(_actorName);
    if (found != mActorObjMap.end())
    {
        mDeadActorObjVector.emplace_back(found->second);
        mActorObjMap.erase(_actorName);
    }
    else
    {
        P_LOG(LOG_WARNING,
            "cannot find actor object name : %s\n",
            _actorName.c_str());
    }
}

UiObject* ObjectContainer::GetUiObject(std::string&& _uiName)
{
    auto found = mUiObjMap.find(_uiName);
    if (found != mUiObjMap.end())
    {
        return &(found->second);
    }
    else
    {
        P_LOG(LOG_WARNING,
            "cannot find ui object name : %s\n",
            _uiName.c_str());
        return nullptr;
    }
}

UiObject* ObjectContainer::GetUiObject(std::string& _uiName)
{
    auto found = mUiObjMap.find(_uiName);
    if (found != mUiObjMap.end())
    {
        return &(found->second);
    }
    else
    {
        P_LOG(LOG_WARNING,
            "cannot find ui object name : %s\n",
            _uiName.c_str());
        return nullptr;
    }
}

void ObjectContainer::AddUiObject(UiObject& _newUi)
{
    mNewUiObjVector.emplace_back(_newUi);
}

void ObjectContainer::DeleteUiObject(std::string&& _uiName)
{
    auto found = mUiObjMap.find(_uiName);
    if (found != mUiObjMap.end())
    {
        mDeadUiObjVector.emplace_back(found->second);
        mUiObjMap.erase(_uiName);
    }
    else
    {
        P_LOG(LOG_WARNING,
            "cannot find ui object name : %s\n",
            _uiName.c_str());
    }
}

void ObjectContainer::DeleteUiObject(std::string& _uiName)
{
    auto found = mUiObjMap.find(_uiName);
    if (found != mUiObjMap.end())
    {
        mDeadUiObjVector.emplace_back(found->second);
        mUiObjMap.erase(_uiName);
    }
    else
    {
        P_LOG(LOG_WARNING,
            "cannot find ui object name : %s\n",
            _uiName.c_str());
    }
}

void ObjectContainer::DeleteAllActor()
{
    for (auto& actor : mActorObjMap)
    {
        mDeadActorObjVector.emplace_back(actor.second);
    }
    mActorObjMap.clear();
}

void ObjectContainer::DeleteAllUi()
{
    for (auto& ui : mUiObjMap)
    {
        mDeadUiObjVector.emplace_back(ui.second);
    }
    mUiObjMap.clear();
}


void ObjectContainer::InitAllNewObjects()
{
    for (auto& newActor : mNewActorObjVector)
    {
        bool result = newActor.Init();
#ifdef _DEBUG
        assert(result);
#endif // _DEBUG
        mActorObjMap.insert({ newActor.GetObjectName(),newActor });
    }
    mNewActorObjVector.clear();

    for (auto& newUi : mNewUiObjVector)
    {
        bool result = newUi.Init();
#ifdef _DEBUG
        assert(result);
#endif // _DEBUG
        mUiObjMap.insert({ newUi.GetObjectName(),newUi });
    }
    mNewUiObjVector.clear();
}

void ObjectContainer::DeleteAllDeadObjects()
{
    for (auto& deadActor : mDeadActorObjVector)
    {
        deadActor.Destory();
    }
    mDeadActorObjVector.clear();

    for (auto& deadUi : mDeadUiObjVector)
    {
        deadUi.Destory();
    }
    mDeadUiObjVector.clear();
}
