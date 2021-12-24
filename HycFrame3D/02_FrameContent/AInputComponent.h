#pragma once

#include "ActorComponent.h"

using ActorInputProcessFuncType = void(*)(class AInputComponent*, Timer&);

class AInputComponent :public ActorComponent
{
public:
    AInputComponent(std::string&& _compName, class ActorObject& _actorOwner);
    AInputComponent(std::string& _compName, class ActorObject& _actorOwner);
    virtual ~AInputComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void SetInputFunction(ActorInputProcessFuncType _func);
    void ClearInputFunction();

private:
    ActorInputProcessFuncType mInputPrecessFunctionPtr;
};
