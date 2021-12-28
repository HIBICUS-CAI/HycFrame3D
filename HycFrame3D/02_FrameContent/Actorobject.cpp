#include "ActorObject.h"
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "ActorComponent.h"

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
    std::string compName = GetObjectName();

    switch (_compType)
    {
    case COMP_TYPE::A_TRANSFORM: compName += "-transform"; break;
    case COMP_TYPE::A_INPUT: compName += "-input"; break;
    case COMP_TYPE::A_INTERACT: compName += "-interact"; break;
    case COMP_TYPE::A_TIMER: compName += "-timer"; break;
    case COMP_TYPE::A_COLLISION: compName += "-collision"; break;
    case COMP_TYPE::A_MESH: compName += "-mesh"; break;
    case COMP_TYPE::A_LIGHT: compName += "-light"; break;
    case COMP_TYPE::A_AUDIO: compName += "-audio"; break;
    case COMP_TYPE::A_PARTICLE: compName += "-particle"; break;
    default: break;
    }

    mActorCompMap.insert({ _compType,compName });
}

bool ActorObject::Init()
{
    auto compContainer = GetSceneNode().GetComponentContainer();

    for (auto& compInfo : mActorCompMap)
    {
        auto comp = compContainer->GetComponent(compInfo.second);
#ifdef _DEBUG
        assert(comp);
#endif // _DEBUG
        ((ActorComponent*)comp)->ResetActorOwner(this);
        if (!comp->Init()) { return false; }
        comp->SetCompStatus(STATUS::ACTIVE);
    }

    SetObjectStatus(STATUS::ACTIVE);

    return true;
}

void ActorObject::Destory()
{
    auto compContainer = GetSceneNode().GetComponentContainer();

    for (auto& compInfo : mActorCompMap)
    {
        auto comp = compContainer->GetComponent(compInfo.second);
#ifdef _DEBUG
        assert(comp);
#endif // _DEBUG
        ((ActorComponent*)comp)->ResetActorOwner(this);
        comp->Destory();
        comp->SetCompStatus(STATUS::NEED_DESTORY);
        compContainer->DeleteComponent(compInfo.first, compInfo.second);
    }

    mActorCompMap.clear();
}

void ActorObject::SyncStatusToAllComps()
{
    auto compContainer = GetSceneNode().GetComponentContainer();

    for (auto& compInfo : mActorCompMap)
    {
        auto comp = compContainer->GetComponent(compInfo.second);
#ifdef _DEBUG
        assert(comp);
#endif // _DEBUG
        comp->SetCompStatus(GetObjectStatus());
    }
}
