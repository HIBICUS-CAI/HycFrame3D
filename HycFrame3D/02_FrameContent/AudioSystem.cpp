#include "AudioSystem.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "ComponentContainer.h"
#include "SoundHelper.h"
#include "AAudioComponent.h"
#include "UAudioComponent.h"

AudioSystem::AudioSystem(SystemExecutive* _sysExecutive) :
    System("audio-system", _sysExecutive),
    mAAudioVecPtr(nullptr), mUAudioVecPtr(nullptr)
{

}

AudioSystem::~AudioSystem()
{

}

bool AudioSystem::Init()
{
    if (!soundHasInited()) { if (!initSound()) { return false; } }

#ifdef _DEBUG
    assert(GetSystemExecutive());
#endif // _DEBUG

    mAAudioVecPtr = (std::vector<AAudioComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->getCompVecPtr(COMP_TYPE::A_AUDIO);
    mUAudioVecPtr = (std::vector<UAudioComponent>*)GetSystemExecutive()->
        GetSceneManager()->GetCurrentSceneNode()->
        GetComponentContainer()->getCompVecPtr(COMP_TYPE::U_AUDIO);

    if (!(mAAudioVecPtr && mUAudioVecPtr)) { return false; }

    return true;
}

void AudioSystem::Run(Timer& _timer)
{
    for (auto& aac : *mAAudioVecPtr)
    {
        if (aac.getCompStatus() == STATUS::ACTIVE) { aac.update(_timer); }
    }

    for (auto& uac : *mUAudioVecPtr)
    {
        if (uac.getCompStatus() == STATUS::ACTIVE) { uac.update(_timer); }
    }
}

void AudioSystem::Destory()
{
    uninitSound();
}
