#pragma once

#include "Component.h"

class UiComponent :public Component
{
public:
    UiComponent(std::string&& _compName, class UiObject* _uiOwner);
    UiComponent(std::string& _compName, class UiObject* _uiOwner);
    virtual ~UiComponent();

    UiComponent& operator=(const UiComponent& _source)
    {
        if (this == &_source) { return *this; }
        mUiOwner = _source.mUiOwner;
        Component::operator=(_source);
        return *this;
    }

    class SceneNode& GetSceneNode() const;
    class UiObject* GetUiOwner() const;
    void ResetUiOwner(class UiObject* _owner);

public:
    virtual bool Init() = 0;
    virtual void Update(Timer& _timer) = 0;
    virtual void Destory() = 0;

private:
    class UiObject* mUiOwner;
};
