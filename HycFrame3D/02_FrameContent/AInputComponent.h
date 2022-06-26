#pragma once

#include "ActorComponent.h"

using ActorInputProcessFuncType = void(*)(class AInputComponent*, Timer&);

class AInputComponent :public ActorComponent
{
public:
    AInputComponent(std::string&& _compName, class ActorObject* _actorOwner);
    AInputComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~AInputComponent();

    AInputComponent& operator=(const AInputComponent& _source)
    {
        if (this == &_source) { return *this; }
        mInputPrecessFunctionPtr = _source.mInputPrecessFunctionPtr;
        ActorComponent::operator=(_source);
        return *this;
    }

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void SetInputFunction(ActorInputProcessFuncType _func);
    void ClearInputFunction();

    class ActorObject* GetActorObject(std::string&& _actorName);
    class ActorObject* GetActorObject(std::string& _actorName);
    class UiObject* GetUiObject(std::string&& _uiName);
    class UiObject* GetUiObject(std::string& _uiName);

private:
    ActorInputProcessFuncType mInputPrecessFunctionPtr;
};
