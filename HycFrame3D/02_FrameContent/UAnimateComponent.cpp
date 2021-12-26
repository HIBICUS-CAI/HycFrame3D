#include "UAnimateComponent.h"
#include "UiObject.h"
#include <vector>
#include <RSCommon.h>

UAnimateComponent::UAnimateComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mAnimateMap({}), mCurrentAnimateCut(0), mCurrentAnimate(nullptr),
    mAnimateChangedFlg(false), mTimeCounter(0.f)
{

}

UAnimateComponent::UAnimateComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner),
    mAnimateMap({}), mCurrentAnimateCut(0), mCurrentAnimate(nullptr),
    mAnimateChangedFlg(false), mTimeCounter(0.f)
{

}

UAnimateComponent::~UAnimateComponent()
{

}

bool UAnimateComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void UAnimateComponent::Update(Timer& _timer)
{

}

void UAnimateComponent::Destory()
{

}

bool UAnimateComponent::LoadAnimate(std::string _aniName, std::string _aniPath,
    DirectX::XMFLOAT2 _stride, UINT _maxCount,
    bool _repeatFlg, float _switchTime)
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void UAnimateComponent::DeleteAnimate(std::string&& _aniName)
{

}

void UAnimateComponent::DeleteAnimate(std::string& _aniName)
{

}

void UAnimateComponent::ResetCurrentAnimate()
{

}

void UAnimateComponent::ChangeAnimateTo(std::string&& _aniName)
{

}

void UAnimateComponent::ChangeAnimateTo(std::string& _aniName)
{

}

void UAnimateComponent::SyncAniInfoToSprite()
{

}
