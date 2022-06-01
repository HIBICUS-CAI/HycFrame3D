#pragma once

#include "UiComponent.h"
#include "SoundHelper.h"
#include <unordered_map>

class UAudioComponent :public UiComponent
{
public:
    UAudioComponent(std::string&& _compName, class UiObject* _uiOwner);
    UAudioComponent(std::string& _compName, class UiObject* _uiOwner);
    virtual ~UAudioComponent();

    UAudioComponent& operator=(const UAudioComponent& _source)
    {
        if (this == &_source) { return *this; }
        mAudioMap = _source.mAudioMap;
        UiComponent::operator=(_source);
        return *this;
    }

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void AddAudio(std::string&& _audioName, class SceneNode& _scene);
    void AddAudio(std::string& _audioName, class SceneNode& _scene);

    void PlayBgm(std::string&& _bgmName, float _volume);
    void PlayBgm(std::string& _bgmName, float _volume);
    void PlaySe(std::string&& _seName, float _volume);
    void PlaySe(std::string& _seName, float _volume);

    void StopBgm();
    void StopBgm(std::string&& _bgmName);
    void StopBgm(std::string& _bgmName);

private:
    std::unordered_map<std::string, SOUND_HANDLE> mAudioMap;
};
