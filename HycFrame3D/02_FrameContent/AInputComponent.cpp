#include "AInputComponent.h"
#include "ActorObject.h"

AInputComponent::AInputComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mInputPrecessFunctionPtr(nullptr)
{

}

AInputComponent::AInputComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mInputPrecessFunctionPtr(nullptr)
{

}

AInputComponent::~AInputComponent()
{

}

bool AInputComponent::Init()
{
    if (!mInputPrecessFunctionPtr)
    {
        P_LOG(LOG_ERROR, "there's still no input func in : %s\n",
            GetCompName().c_str());
    }

    return true;
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
