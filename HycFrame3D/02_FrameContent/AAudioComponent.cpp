#include "AAudioComponent.h"
#include "ActorObject.h"
#include "SceneNode.h"
#include "AssetsPool.h"

AAudioComponent::AAudioComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mAudioMap({})
{

}

AAudioComponent::AAudioComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner), mAudioMap({})
{

}

AAudioComponent::~AAudioComponent()
{

}

bool AAudioComponent::Init()
{
    if (mAudioMap.size()) { return true; }
    else { return false; }
}

void AAudioComponent::Update(Timer& _timer)
{

}

void AAudioComponent::Destory()
{
    mAudioMap.clear();
}

void AAudioComponent::AddAudio(std::string&& _audioName, SceneNode& _scene)
{
    SOUND_HANDLE audio = _scene.GetAssetsPool()->GetSoundIfExisted(_audioName);
#ifdef _DEBUG
    assert(audio);
#endif // _DEBUG
    mAudioMap.insert({ _audioName,audio });
}

void AAudioComponent::AddAudio(std::string& _audioName, SceneNode& _scene)
{
    SOUND_HANDLE audio = _scene.GetAssetsPool()->GetSoundIfExisted(_audioName);
#ifdef _DEBUG
    assert(audio);
#endif // _DEBUG
    mAudioMap.insert({ _audioName,audio });
}

void AAudioComponent::PlayBgm(std::string&& _bgmName, float _volume)
{
    SetVolume(_bgmName, _volume);
    PlayBGM(_bgmName);
}

void AAudioComponent::PlayBgm(std::string& _bgmName, float _volume)
{
    SetVolume(_bgmName, _volume);
    PlayBGM(_bgmName);
}

void AAudioComponent::PlaySe(std::string&& _seName, float _volume)
{
    SetVolume(_seName, _volume);
    PlaySE(_seName);
}

void AAudioComponent::PlaySe(std::string& _seName, float _volume)
{
    SetVolume(_seName, _volume);
    PlaySE(_seName);
}

void AAudioComponent::StopBgm()
{
    StopBGM();
}

void AAudioComponent::StopBgm(std::string&& _bgmName)
{
    StopBGM(_bgmName);
}

void AAudioComponent::StopBgm(std::string& _bgmName)
{
    StopBGM(_bgmName);
}
