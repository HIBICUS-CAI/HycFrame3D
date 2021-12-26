#pragma once

#include "Object.h"
#include <unordered_map>
#include "SceneNode.h"
#include "ComponentContainer.h"

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

        switch (_type)
        {
        case COMP_TYPE::U_TRANSFORM: name += "-transform"; break;
        case COMP_TYPE::U_SPRITE: name += "-sprite"; break;
        case COMP_TYPE::U_ANIMATE: name += "-animate"; break;
        case COMP_TYPE::U_TIMER: name += "-timer"; break;
        case COMP_TYPE::U_INPUT: name += "-input"; break;
        case COMP_TYPE::U_INTERACT: name += "-interact"; break;
        case COMP_TYPE::U_BUTTON: name += "-button"; break;
        case COMP_TYPE::U_AUDIO: name += "-audio"; break;
        default: break;
        }

        return (T*)(container->GetComponent(name));
    }

public:
    virtual bool Init();
    virtual void Destory();

private:
    std::unordered_map<COMP_TYPE, std::string> mUiCompMap;
};
