#pragma once

#include "ActorComponent.h"
#include "SoundHelper.h"
#include <unordered_map>

class AAudioComponent :public ActorComponent
{
public:
    AAudioComponent(std::string&& _compName, class ActorObject* _actorOwner);
    AAudioComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~AAudioComponent();

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
