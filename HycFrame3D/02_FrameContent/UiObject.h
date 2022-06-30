#pragma once

#include "Object.h"
#include <unordered_map>
#include "SceneNode.h"
#include "ComponentContainer.h"
#include "ComponentGetter.h"

class UiObject :public Object
{
public:
    UiObject(std::string&& _uiName, class SceneNode& _sceneNode);
    UiObject(std::string& _uiName, class SceneNode& _sceneNode);
    virtual ~UiObject();

    void AddUComponent(COMP_TYPE _compType);

    template <typename T>
    inline T* GetUComponent(COMP_TYPE _type)
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
    std::unordered_map<COMP_TYPE, std::string> mUiCompMap;
};
