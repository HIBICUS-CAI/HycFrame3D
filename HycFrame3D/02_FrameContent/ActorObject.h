#pragma once

#include "Object.h"
#include <unordered_map>
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "ComponentGetter.h"

class ActorObject :public Object
{
public:
    ActorObject(std::string&& _actorName, class SceneNode& _sceneNode);
    ActorObject(std::string& _actorName, class SceneNode& _sceneNode);
    virtual ~ActorObject();

    void AddAComponent(COMP_TYPE _compType);

    template <typename T>
    inline T* GetComponent()
    {
        auto container = GetSceneNode().GetComponentContainer();
        std::string name = GetObjectName();
        ComponentGetter::GenerateCompName<T>(name);

        return (T*)(container->GetComponent(name));
    }

public:
    virtual bool Init();
    virtual void Destory();

protected:
    virtual void SyncStatusToAllComps();

private:
    std::unordered_map<COMP_TYPE, std::string> mActorCompMap;
};
