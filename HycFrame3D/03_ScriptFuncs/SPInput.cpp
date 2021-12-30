#include "SPInput.h"

void RegisterSPInput(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TestASpInput),TestASpInput });
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(TestASpInit),TestASpInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(TestASpUpdate),TestASpUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(TestASpDestory),TestASpDestory });
}

void TestASpInput(AInputComponent* _aic, Timer& _timer)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test2\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("test2", "test2");
    }
}

bool TestASpInit(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "a sp init\n");

    _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        StartTimer("timer1");

    return true;
}
void TestASpUpdate(AInteractComponent* _aitc, Timer&)
{
    //P_LOG(LOG_DEBUG, "a sp update\n");
    /*float time0 = _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        GetTimer("timer0")->mTime;
    float time1 = _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        GetTimer("timer1")->mTime;
    P_LOG(LOG_DEBUG, "timer0 : %f , timer1 : %f\n", time0, time1);*/
}

void TestASpDestory(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "a sp destory\n");
}
