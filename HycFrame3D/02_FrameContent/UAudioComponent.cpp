#include "UAudioComponent.h"
#include "UiObject.h"
#include "SceneNode.h"
#include "AssetsPool.h"

UAudioComponent::UAudioComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner), mAudioMap({})
{

}

UAudioComponent::UAudioComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner), mAudioMap({})
{

}

UAudioComponent::~UAudioComponent()
{

}

bool UAudioComponent::Init()
{
    if (mAudioMap.size()) { return true; }
    else { return false; }
}

void UAudioComponent::Update(Timer& _timer)
{

}

void UAudioComponent::Destory()
{
    mAudioMap.clear();
}

void UAudioComponent::AddAudio(std::string&& _audioName, SceneNode& _scene)
{
    SOUND_HANDLE audio = _scene.GetAssetsPool()->getSoundIfExisted(_audioName);
#ifdef _DEBUG
    assert(audio);
#endif // _DEBUG
    mAudioMap.insert({ _audioName,audio });
}

void UAudioComponent::AddAudio(std::string& _audioName, SceneNode& _scene)
{
    SOUND_HANDLE audio = _scene.GetAssetsPool()->getSoundIfExisted(_audioName);
#ifdef _DEBUG
    assert(audio);
#endif // _DEBUG
    mAudioMap.insert({ _audioName,audio });
}

void UAudioComponent::PlayBgm(std::string&& _bgmName, float _volume)
{
    setVolume(_bgmName, _volume);
    playBGM(_bgmName);
}

void UAudioComponent::PlayBgm(std::string& _bgmName, float _volume)
{
    setVolume(_bgmName, _volume);
    playBGM(_bgmName);
}

void UAudioComponent::PlaySe(std::string&& _seName, float _volume)
{
    setVolume(_seName, _volume);
    playSE(_seName);
}

void UAudioComponent::PlaySe(std::string& _seName, float _volume)
{
    setVolume(_seName, _volume);
    playSE(_seName);
}

void UAudioComponent::StopBgm()
{
    stopBGM();
}

void UAudioComponent::StopBgm(std::string&& _bgmName)
{
    stopBGM(_bgmName);
}

void UAudioComponent::StopBgm(std::string& _bgmName)
{
    stopBGM(_bgmName);
}
