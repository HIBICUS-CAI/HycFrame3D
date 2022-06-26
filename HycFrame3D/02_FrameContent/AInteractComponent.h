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

    AInteractComponent& operator=(const AInteractComponent& _source)
    {
        if (this == &_source) { return *this; }
        mInitProcessFunctionPtr = _source.mInitProcessFunctionPtr;
        mUpdateProcessFunctionPtr = _source.mUpdateProcessFunctionPtr;
        mDestoryProcessFunctionPtr = _source.mDestoryProcessFunctionPtr;
        ActorComponent::operator=(_source);
        return *this;
    }

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

    class ActorObject* GetActorObject(std::string&& _actorName);
    class ActorObject* GetActorObject(std::string& _actorName);
    class UiObject* GetUiObject(std::string&& _uiName);
    class UiObject* GetUiObject(std::string& _uiName);

private:
    ActorInteractInitFuncType mInitProcessFunctionPtr;
    ActorInteractUpdateFuncType mUpdateProcessFunctionPtr;
    ActorInteractDestoryFuncType mDestoryProcessFunctionPtr;
};
