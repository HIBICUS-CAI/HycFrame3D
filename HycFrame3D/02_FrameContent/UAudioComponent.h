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

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void CheckAudioInAssetsPool(std::string&& _audioName);
    void CheckAudioInAssetsPool(std::string& _audioName);

    void PlayBgm(std::string&& _bgmName, float _volume);
    void PlayBgm(std::string& _bgmName, float _volume);
    void PlaySe(std::string&& _seName, float _volume);
    void PlaySe(std::string& _seName, float _volume);

    void StopBgm();

private:
    std::unordered_map<std::string, SOUND_HANDLE> mAudioMap;
};
