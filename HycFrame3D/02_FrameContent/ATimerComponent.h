#pragma once

#include "ActorComponent.h"
#include <unordered_map>

struct ACTOR_TIMER
{
    std::string mName = "";
    bool mActive = false;
    float mTime = 0.f;

    bool IsGreaterThan(float _value) { return (mTime > _value); }
};

class ATimerComponent :public ActorComponent
{
public:
    ATimerComponent(std::string&& _compName, class ActorObject* _actorOwner);
    ATimerComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~ATimerComponent();

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
    ACTOR_TIMER* GetTimer(std::string&& _timerName);
    ACTOR_TIMER* GetTimer(std::string& _timerName);

private:
    std::unordered_map<std::string, ACTOR_TIMER> mTimerMap;
};
