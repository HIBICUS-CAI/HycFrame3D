#include "UAudioComponent.h"
#include "UiObject.h"

UAudioComponent::UAudioComponent(std::string&& _compName,
    UiObject& _uiOwner) :
    UiComponent(_compName, _uiOwner), mAudioMap({})
{

}

UAudioComponent::UAudioComponent(std::string& _compName,
    UiObject& _uiOwner) :
    UiComponent(_compName, _uiOwner), mAudioMap({})
{

}

UAudioComponent::~UAudioComponent()
{

}

bool UAudioComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void UAudioComponent::Update(Timer& _timer)
{

}

void UAudioComponent::Destory()
{

}

void UAudioComponent::CheckAudioInAssetsPool(std::string&& _audioName)
{

}

void UAudioComponent::CheckAudioInAssetsPool(std::string& _audioName)
{

}

void UAudioComponent::PlayBgm(std::string&& _bgmName, float _volume)
{

}

void UAudioComponent::PlayBgm(std::string& _bgmName, float _volume)
{

}

void UAudioComponent::PlaySe(std::string&& _seName, float _volume)
{

}

void UAudioComponent::PlaySe(std::string& _seName, float _volume)
{

}

void UAudioComponent::StopBgm()
{

}
