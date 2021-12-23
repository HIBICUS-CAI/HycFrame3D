#pragma once

#include "Object.h"
#include <unordered_map>

class ActorObject :public Object
{
public:
    ActorObject(std::string&& _actorName, class SceneNode& _sceneNode);
    ActorObject(std::string& _actorName, class SceneNode& _sceneNode);
    virtual ~ActorObject();

    void AddAComponent(COMP_TYPE _compType);
    // TEMP-------------------------------
    template <typename T>
    inline T* GetAComponent(COMP_TYPE _type)
    {
        return nullptr;
    }
    // TEMP-------------------------------

public:
    virtual bool Init();
    virtual void Destory();

private:
    std::unordered_map<COMP_TYPE, std::string> mActorCompMap;
};
