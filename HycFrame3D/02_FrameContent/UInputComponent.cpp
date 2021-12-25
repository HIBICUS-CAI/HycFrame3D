#include "UInputComponent.h"
#include "ActorObject.h"

UInputComponent::UInputComponent(std::string&& _compName,
    UiObject& _uiOwner) :
    UiComponent(_compName, _uiOwner), mInputPrecessFunctionPtr(nullptr)
{

}

UInputComponent::UInputComponent(std::string& _compName,
    UiObject& _uiOwner) :
    UiComponent(_compName, _uiOwner), mInputPrecessFunctionPtr(nullptr)
{

}

UInputComponent::~UInputComponent()
{

}

bool UInputComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void UInputComponent::Update(Timer& _timer)
{
    if (mInputPrecessFunctionPtr) { mInputPrecessFunctionPtr(this, _timer); }
}

void UInputComponent::Destory()
{

}

void UInputComponent::SetInputFunction(UiInputProcessFuncType _func)
{
#ifdef _DEBUG
    assert(_func);
#endif // _DEBUG
    mInputPrecessFunctionPtr = _func;
}

void UInputComponent::ClearInputFunction()
{
    mInputPrecessFunctionPtr = nullptr;
}