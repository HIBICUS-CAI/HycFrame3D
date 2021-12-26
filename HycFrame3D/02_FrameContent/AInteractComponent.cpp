#include "AInteractComponent.h"
#include "ActorObject.h"

AInteractComponent::AInteractComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mInitProcessFunctionPtr(nullptr),
    mUpdateProcessFunctionPtr(nullptr),
    mDestoryProcessFunctionPtr(nullptr)
{

}

AInteractComponent::AInteractComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mInitProcessFunctionPtr(nullptr),
    mUpdateProcessFunctionPtr(nullptr),
    mDestoryProcessFunctionPtr(nullptr)
{

}

AInteractComponent::~AInteractComponent()
{

}

bool AInteractComponent::Init()
{
    if (mInitProcessFunctionPtr) { return mInitProcessFunctionPtr(this); }
    else { return false; }
}

void AInteractComponent::Update(Timer& _timer)
{
    if (mUpdateProcessFunctionPtr) { return mUpdateProcessFunctionPtr(this, _timer); }
}

void AInteractComponent::Destory()
{
    if (mDestoryProcessFunctionPtr) { return mDestoryProcessFunctionPtr(this); }
}

void AInteractComponent::SetInitFunction(ActorInteractInitFuncType _initFunc)
{
#ifdef _DEBUG
    assert(_initFunc);
#endif // _DEBUG
    mInitProcessFunctionPtr = _initFunc;
}

void AInteractComponent::SetUpdateFunction(ActorInteractUpdateFuncType _updateFunc)
{
#ifdef _DEBUG
    assert(_updateFunc);
#endif // _DEBUG
    mUpdateProcessFunctionPtr = _updateFunc;
}

void AInteractComponent::SetDestoryFunction(ActorInteractDestoryFuncType _destoryFunc)
{
#ifdef _DEBUG
    assert(_destoryFunc);
#endif // _DEBUG
    mDestoryProcessFunctionPtr = _destoryFunc;
}

void AInteractComponent::ClearInitFunction()
{
    mInitProcessFunctionPtr = nullptr;
}

void AInteractComponent::ClearUpdateFunction()
{
    mUpdateProcessFunctionPtr = nullptr;
}

void AInteractComponent::ClearDestoryFunction()
{
    mDestoryProcessFunctionPtr = nullptr;
}
