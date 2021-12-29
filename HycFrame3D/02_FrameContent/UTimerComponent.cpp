#include "UTimerComponent.h"
#include "UiObject.h"

UTimerComponent::UTimerComponent(std::string&& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner), mTimerMap({})
{

}

UTimerComponent::UTimerComponent(std::string& _compName,
    UiObject* _uiOwner) :
    UiComponent(_compName, _uiOwner), mTimerMap({})
{

}

UTimerComponent::~UTimerComponent()
{

}

bool UTimerComponent::Init()
{
    for (auto& t : mTimerMap) { t.second.mTime = 0.f; }

    return true;
}

void UTimerComponent::Update(Timer& _timer)
{
    for (auto& t : mTimerMap)
    {
        if (t.second.mActive)
        {
            t.second.mTime += _timer.FloatDeltaTime() / 1000.f;
        }
    }
}

void UTimerComponent::Destory()
{
    mTimerMap.clear();
}

void UTimerComponent::AddTimer(std::string&& _timerName)
{
    UI_TIMER t = {};
    t.mName = _timerName;
    mTimerMap.insert({ _timerName,t });
}

void UTimerComponent::AddTimer(std::string& _timerName)
{
    UI_TIMER t = {};
    t.mName = _timerName;
    mTimerMap.insert({ _timerName,t });
}

void UTimerComponent::StartTimer(std::string&& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = true;
    }
}

void UTimerComponent::StartTimer(std::string& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = true;
    }
}

void UTimerComponent::PauseTimer(std::string&& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = false;
    }
}

void UTimerComponent::PauseTimer(std::string& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = false;
    }
}

void UTimerComponent::ResetTimer(std::string&& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = false;
        mTimerMap[_timerName].mTime = 0.f;
    }
}

void UTimerComponent::ResetTimer(std::string& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        mTimerMap[_timerName].mActive = false;
        mTimerMap[_timerName].mTime = 0.f;
    }
}

UI_TIMER* UTimerComponent::GetTimer(std::string&& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        return &(mTimerMap[_timerName]);
    }
    else
    {
        P_LOG(LOG_WARNING, "this timer doesnt exist : %s\n",
            _timerName.c_str());
        return nullptr;
    }
}

UI_TIMER* UTimerComponent::GetTimer(std::string& _timerName)
{
    if (mTimerMap.find(_timerName) != mTimerMap.end())
    {
        return &(mTimerMap[_timerName]);
    }
    else
    {
        P_LOG(LOG_WARNING, "this timer doesnt exist : %s\n",
            _timerName.c_str());
        return nullptr;
    }
}
