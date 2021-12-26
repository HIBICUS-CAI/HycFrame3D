#include "UiComponent.h"
#include "UiObject.h"

UiComponent::UiComponent(std::string&& _compName, UiObject* _uiOwner) :
    Component(_compName), mUiOwner(_uiOwner)
{

}

UiComponent::UiComponent(std::string& _compName, UiObject* _uiOwner) :
    Component(_compName), mUiOwner(_uiOwner)
{

}

UiComponent::~UiComponent()
{

}

UiObject* UiComponent::GetUiOwner() const
{
    return mUiOwner;
}

void UiComponent::ResetUiOwner(UiObject* _owner)
{
    mUiOwner = _owner;
}
