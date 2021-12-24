#include "AInputComponent.h"
#include "ActorObject.h"

AInputComponent::AInputComponent(std::string&& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner), mInputPrecessFunctionPtr(nullptr)
{

}

AInputComponent::AInputComponent(std::string& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner), mInputPrecessFunctionPtr(nullptr)
{

}

AInputComponent::~AInputComponent()
{

}

bool AInputComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void AInputComponent::Update(Timer& _timer)
{
    if (mInputPrecessFunctionPtr) { mInputPrecessFunctionPtr(this, _timer); }
}

void AInputComponent::Destory()
{

}

void AInputComponent::SetInputFunction(ActorInputProcessFuncType _func)
{
#ifdef _DEBUG
    assert(_func);
#endif // _DEBUG
    mInputPrecessFunctionPtr = _func;
}

void AInputComponent::ClearInputFunction()
{
    mInputPrecessFunctionPtr = nullptr;
}
