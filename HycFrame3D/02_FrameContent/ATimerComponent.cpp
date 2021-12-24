#include "ATimerComponent.h"
#include "ActorObject.h"

ATimerComponent::ATimerComponent(std::string&& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner), mTimerMap({})
{

}

ATimerComponent::ATimerComponent(std::string& _compName,
    ActorObject& _actorOwner) :
    ActorComponent(_compName, _actorOwner), mTimerMap({})
{

}

ATimerComponent::~ATimerComponent()
{

}

bool ATimerComponent::Init()
{
    // TEMP-----------------
    return true;
    // TEMP-----------------
}

void ATimerComponent::Update(Timer& _timer)
{
    for (auto& t : mTimerMap)
    {
        if (t.second.mActive) { t.second.mTime += _timer.FloatDeltaTime(); }
    }
}

void ATimerComponent::Destory()
{
    mTimerMap.clear();
}

void ATimerComponent::AddTimer(std::string&& _timerName)
{
    ACTOR_TIMER t = {};
    t.mName = _timerName;
    mTimerMap.insert({ _timerName,t });
}

void ATimerComponent::AddTimer(std::string& _timerName)
{
    ACTOR_TIMER t = {};
    t.mName = _timerName;
    mTimerMap.insert({ _timerName,t });
}

void ATimerComponent::StartTimer(std::string&& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = true;
    }
}

void ATimerComponent::StartTimer(std::string& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = true;
    }
}

void ATimerComponent::PauseTimer(std::string&& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = false;
    }
}

void ATimerComponent::PauseTimer(std::string& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = false;
    }
}

void ATimerComponent::ResetTimer(std::string&& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = false;
        mTimerMap[_timerName].mTime = 0.f;
    }
}

void ATimerComponent::ResetTimer(std::string& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = false;
        mTimerMap[_timerName].mTime = 0.f;
    }
}

ACTOR_TIMER* ATimerComponent::GetTimer(std::string&& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        return &(mTimerMap[_timerName]);
    }
    else
    {
        return nullptr;
    }
}

ACTOR_TIMER* ATimerComponent::GetTimer(std::string& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        return &(mTimerMap[_timerName]);
    }
    else
    {
        return nullptr;
    }
}
