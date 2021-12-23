#pragma once

#include "Component.h"

class UiComponent :public Component
{
public:
    UiComponent(std::string&& _compName, class UiObject& _uiOwner);
    UiComponent(std::string& _compName, class UiObject& _uiOwner);
    virtual ~UiComponent();

    class UiObject& GetUiOwner() const;

public:
    virtual bool Init() = 0;
    virtual void Update(Timer& _timer) = 0;
    virtual void Destory() = 0;

private:
    class UiObject& mUiOwner;
};
