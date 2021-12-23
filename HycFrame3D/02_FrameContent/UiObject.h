#pragma once

#include "Object.h"
#include <unordered_map>

class UiObject :public Object
{
public:
    UiObject(std::string&& _uiName, class SceneNode& _sceneNode);
    UiObject(std::string& _uiName, class SceneNode& _sceneNode);
    virtual ~UiObject();

    void AddUComponent(COMP_TYPE _compType);
    // TEMP-------------------------------
    template <typename T>
    inline T* GetUComponent(COMP_TYPE _type)
    {
        return nullptr;
    }
    // TEMP-------------------------------

public:
    virtual bool Init();
    virtual void Destory();

private:
    std::unordered_map<COMP_TYPE, std::string> mUiCompMap;
};
