#pragma once

#include "Object.h"
#include <unordered_map>
#include "SceneNode.h"
#include "ComponentContainer.h"

class ActorObject :public Object
{
public:
    ActorObject(std::string&& _actorName, class SceneNode& _sceneNode);
    ActorObject(std::string& _actorName, class SceneNode& _sceneNode);
    virtual ~ActorObject();

    void AddAComponent(COMP_TYPE _compType);

    template <typename T>
    inline T* GetAComponent(COMP_TYPE _type)
    {
        auto container = GetSceneNode().GetComponentContainer();
        std::string name = GetObjectName();

        switch (_type)
        {
        case COMP_TYPE::A_TRANSFORM: name += "-transform"; break;
        case COMP_TYPE::A_INPUT: name += "-input"; break;
        case COMP_TYPE::A_INTERACT: name += "-interact"; break;
        case COMP_TYPE::A_TIMER: name += "-timer"; break;
        case COMP_TYPE::A_COLLISION: name += "-collision"; break;
        case COMP_TYPE::A_MESH: name += "-mesh"; break;
        case COMP_TYPE::A_LIGHT: name += "-light"; break;
        case COMP_TYPE::A_AUDIO: name += "-audio"; break;
        case COMP_TYPE::A_PARTICLE: name += "-particle"; break;
        default: break;
        }

        return (T*)(container->GetComponent(name));
    }

public:
    virtual bool Init();
    virtual void Destory();

private:
    std::unordered_map<COMP_TYPE, std::string> mActorCompMap;
};
