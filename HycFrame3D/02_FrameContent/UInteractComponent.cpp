#include "UInteractComponent.h"
#include "UiObject.h"

UInteractComponent::UInteractComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mInitProcessFunctionPtr(nullptr),
    mUpdateProcessFunctionPtr(nullptr),
    mDestoryProcessFunctionPtr(nullptr)
{

}

UInteractComponent::UInteractComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mInitProcessFunctionPtr(nullptr),
    mUpdateProcessFunctionPtr(nullptr),
    mDestoryProcessFunctionPtr(nullptr)
{

}

UInteractComponent::~UInteractComponent()
{

}

bool UInteractComponent::Init()
{
    if (mInitProcessFunctionPtr) { return mInitProcessFunctionPtr(this); }
    else { return false; }
}

void UInteractComponent::Update(Timer& _timer)
{
    if (mUpdateProcessFunctionPtr) { return mUpdateProcessFunctionPtr(this, _timer); }
}

void UInteractComponent::Destory()
{
    if (mDestoryProcessFunctionPtr) { return mDestoryProcessFunctionPtr(this); }
}

void UInteractComponent::SetInitFunction(UiInteractInitFuncType _initFunc)
{
#ifdef _DEBUG
    assert(_initFunc);
#endif // _DEBUG
    mInitProcessFunctionPtr = _initFunc;
}

void UInteractComponent::SetUpdateFunction(UiInteractUpdateFuncType _updateFunc)
{
#ifdef _DEBUG
    assert(_updateFunc);
#endif // _DEBUG
    mUpdateProcessFunctionPtr = _updateFunc;
}

void UInteractComponent::SetDestoryFunction(UiInteractDestoryFuncType _destoryFunc)
{
#ifdef _DEBUG
    assert(_destoryFunc);
#endif // _DEBUG
    mDestoryProcessFunctionPtr = _destoryFunc;
}

void UInteractComponent::ClearInitFunction()
{
    mInitProcessFunctionPtr = nullptr;
}

void UInteractComponent::ClearUpdateFunction()
{
    mUpdateProcessFunctionPtr = nullptr;
}

void UInteractComponent::ClearDestoryFunction()
{
    mDestoryProcessFunctionPtr = nullptr;
}
