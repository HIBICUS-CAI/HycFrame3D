#include "UButtonComponent.h"
#include "UiObject.h"

UButtonComponent::UButtonComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mSurroundBtnCompNames({ NULL_BTN,NULL_BTN,NULL_BTN,NULL_BTN }),
    mIsSelected(false)
{

}

UButtonComponent::UButtonComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mSurroundBtnCompNames({ NULL_BTN,NULL_BTN,NULL_BTN,NULL_BTN }),
    mIsSelected(false)
{

}

UButtonComponent::~UButtonComponent()
{

}

bool UButtonComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void UButtonComponent::Update(Timer& _timer)
{

}

void UButtonComponent::Destory()
{

}

void UButtonComponent::SetIsBeingSelected(bool _beingSelected)
{

}

bool UButtonComponent::IsBeingSelected() const
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void UButtonComponent::SelectUpBtn()
{

}

void UButtonComponent::SelectDownBtn()
{

}

void UButtonComponent::SelectLeftBtn()
{

}

void UButtonComponent::SelectRightBtn()
{

}

UButtonComponent* UButtonComponent::GetUpBtn()
{
    // TEMP-----------------
    return nullptr;
    // TEMP-----------------
}

UButtonComponent* UButtonComponent::GetDownBtn()
{
    // TEMP-----------------
    return nullptr;
    // TEMP-----------------
}

UButtonComponent* UButtonComponent::GetLeftBtn()
{
    // TEMP-----------------
    return nullptr;
    // TEMP-----------------
}

UButtonComponent* UButtonComponent::GetRightBtn()
{
    // TEMP-----------------
    return nullptr;
    // TEMP-----------------
}
