#pragma once

#include "UiComponent.h"
#include <unordered_map>

struct UI_TIMER
{
    std::string mName = "";
    bool mActive = false;
    float mTime = 0.f;

    bool IsGreaterThan(float _value) { return (mTime > _value); }
};

class UTimerComponent :public UiComponent
{
public:
    UTimerComponent(std::string&& _compName, class UiObject& _uiOwner);
    UTimerComponent(std::string& _compName, class UiObject& _uiOwner);
    virtual ~UTimerComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void AddTimer(std::string&& _timerName);
    void AddTimer(std::string& _timerName);
    void StartTimer(std::string&& _timerName);
    void StartTimer(std::string& _timerName);
    void PauseTimer(std::string&& _timerName);
    void PauseTimer(std::string& _timerName);
    void ResetTimer(std::string&& _timerName);
    void ResetTimer(std::string& _timerName);
    UI_TIMER* GetTimer(std::string&& _timerName);
    UI_TIMER* GetTimer(std::string& _timerName);

private:
    std::unordered_map<std::string, UI_TIMER> mTimerMap;
};
