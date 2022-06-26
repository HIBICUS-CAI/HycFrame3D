#pragma once

#include "UiComponent.h"

using UiInputProcessFuncType = void(*)(class UInputComponent*, Timer&);

class UInputComponent :public UiComponent
{
public:
    UInputComponent(std::string&& _compName, class UiObject* _uiOwner);
    UInputComponent(std::string& _compName, class UiObject* _uiOwner);
    virtual ~UInputComponent();

    UInputComponent& operator=(const UInputComponent& _source)
    {
        if (this == &_source) { return *this; }
        mInputPrecessFunctionPtr = _source.mInputPrecessFunctionPtr;
        UiComponent::operator=(_source);
        return *this;
    }

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void SetInputFunction(UiInputProcessFuncType _func);
    void ClearInputFunction();

    class ActorObject* GetActorObject(std::string&& _actorName);
    class ActorObject* GetActorObject(std::string& _actorName);
    class UiObject* GetUiObject(std::string&& _uiName);
    class UiObject* GetUiObject(std::string& _uiName);

private:
    UiInputProcessFuncType mInputPrecessFunctionPtr;
};
