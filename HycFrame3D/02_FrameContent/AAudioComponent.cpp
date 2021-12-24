#include "AAudioComponent.h"
#include "ActorObject.h"

AAudioComponent::AAudioComponent(std::string&& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner), mAudioMap({})
{

}

AAudioComponent::AAudioComponent(std::string& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner), mAudioMap({})
{

}

AAudioComponent::~AAudioComponent()
{

}

bool AAudioComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void AAudioComponent::Update(Timer& _timer)
{

}

void AAudioComponent::Destory()
{

}

void AAudioComponent::CheckAudioInAssetsPool(std::string&& _audioName)
{

}

void AAudioComponent::CheckAudioInAssetsPool(std::string& _audioName)
{

}

void AAudioComponent::PlayBgm(std::string&& _bgmName, float _volume)
{

}

void AAudioComponent::PlayBgm(std::string& _bgmName, float _volume)
{

}

void AAudioComponent::PlaySe(std::string&& _seName, float _volume)
{

}

void AAudioComponent::PlaySe(std::string& _seName, float _volume)
{

}

void AAudioComponent::StopBgm()
{

}
