#pragma once

#include "ActorComponent.h"

using ActorInteractInitFuncType = bool(*)(class AInteractComponent*);
using ActorInteractUpdateFuncType = void(*)(class AInteractComponent*, Timer&);
using ActorInteractDestoryFuncType = void(*)(class AInteractComponent*);

class AInteractComponent :public ActorComponent
{
public:
    AInteractComponent(std::string&& _compName, class ActorObject* _actorOwner);
    AInteractComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~AInteractComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void SetInitFunction(ActorInteractInitFuncType _initFunc);
    void SetUpdateFunction(ActorInteractUpdateFuncType _updateFunc);
    void SetDestoryFunction(ActorInteractDestoryFuncType _destoryFunc);
    void ClearInitFunction();
    void ClearUpdateFunction();
    void ClearDestoryFunction();

private:
    ActorInteractInitFuncType mInitProcessFunctionPtr;
    ActorInteractUpdateFuncType mUpdateProcessFunctionPtr;
    ActorInteractDestoryFuncType mDestoryProcessFunctionPtr;
};
