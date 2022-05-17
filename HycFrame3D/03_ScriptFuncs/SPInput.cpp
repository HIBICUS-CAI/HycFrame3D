#include "SPInput.h"
#include "RSRoot_DX11.h"
#include "RSPipelinesManager.h"

void RegisterSPInput(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TestASpInput),TestASpInput });
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TempToTitle),TempToTitle });
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TempToSelect),TempToSelect });
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TempToRun),TempToRun });
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TempToResult),TempToResult });
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(TestASpInit),TestASpInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(TestASpUpdate),TestASpUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(TestASpDestory),TestASpDestory });
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(TestUSpInput),TestUSpInput });
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(TestUSpBtnInput),TestUSpBtnInput });
    _factory->GetUInitMapPtr()->insert(
        { FUNC_NAME(TestUSpInit),TestUSpInit });
    _factory->GetUUpdateMapPtr()->insert(
        { FUNC_NAME(TestUSpUpdate),TestUSpUpdate });
    _factory->GetUDestoryMapPtr()->insert(
        { FUNC_NAME(TestUSpDestory),TestUSpDestory });

    _factory->GetAInitMapPtr()->insert({ FUNC_NAME(AniInit),AniInit });
    _factory->GetAUpdateMapPtr()->insert({ FUNC_NAME(AniUpdate),AniUpdate });
    _factory->GetADestoryMapPtr()->insert({ FUNC_NAME(AniDestory),AniDestory });
}

void TestASpInput(AInputComponent* _aic, Timer& _timer)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test2\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("sample2-scene", "sample2-scene.json");
    }

    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        _aic->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            TranslateZAsix(0.1f * _timer.FloatDeltaTime());
    }
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        _aic->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            TranslateXAsix(-0.1f * _timer.FloatDeltaTime());
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        _aic->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            TranslateZAsix(-0.1f * _timer.FloatDeltaTime());
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        _aic->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            TranslateXAsix(0.1f * _timer.FloatDeltaTime());
    }
    if (InputInterface::IsKeyPushedInSingle(KB_P))
    {
        static bool simp = true;
        std::string basic = "light-pipeline";
        std::string simple = "simple-pipeline";
        if (simp)
        {
            GetRSRoot_DX11_Singleton()->PipelinesManager()->SetPipeline(basic);
        }
        else
        {
            GetRSRoot_DX11_Singleton()->PipelinesManager()->SetPipeline(simple);
        }
        simp = !simp;
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

    CONTACT_PONT_PAIR contact = {};
    if (_aitc->GetActorOwner()->GetSceneNode().
        GetActorObject("sp-point-light-actor")->
        GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION)->
        CheckCollisionWith("sp-actor", &contact))
    {
        _aitc->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            RollBackPosition();
        P_LOG(LOG_DEBUG, "a : %f, %f, %f ; b : %f, %f, %f\n",
            contact.first.x, contact.first.y, contact.first.z,
            contact.second.x, contact.second.y, contact.second.z);
        auto center = ACollisionComponent::CalcCenterOfContact(contact);
        P_LOG(LOG_DEBUG, "center of contact : %f, %f, %f\n",
            center.x, center.y, center.z);
    }

    _aitc->GetActorOwner()->GetSceneNode().
        GetActorObject("sp-particle-actor")->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
        SetPosition(_aitc->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            GetProcessingPosition());
    _aitc->GetActorOwner()->GetSceneNode().
        GetActorObject("sp-particle-actor")->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
        TranslateYAsix(5.f);
}

void TestASpDestory(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "a sp destory\n");
}

void TestUSpInput(UInputComponent* _uic, Timer& _timer)
{
    float delta = _timer.FloatDeltaTime();
    auto utc = _uic->GetUiOwner()->
        GetUComponent<UTransformComponent>(COMP_TYPE::U_TRANSFORM);

    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        utc->TranslateYAsix(0.1f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        utc->TranslateXAsix(-0.1f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        utc->TranslateYAsix(-0.1f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        utc->TranslateXAsix(0.1f * delta);
    }

    if (InputInterface::IsKeyPushedInSingle(KB_Z))
    {
        _uic->GetUiOwner()->
            GetUComponent<USpriteComponent>(COMP_TYPE::U_SPRITE)->
            ResetTexture();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_X))
    {
        _uic->GetUiOwner()->
            GetUComponent<UAnimateComponent>(COMP_TYPE::U_ANIMATE)->
            ChangeAnimateTo("number");
    }
    if (InputInterface::IsKeyPushedInSingle(KB_C))
    {
        _uic->GetUiOwner()->
            GetUComponent<UAnimateComponent>(COMP_TYPE::U_ANIMATE)->
            ChangeAnimateTo("runman");
    }

    if (InputInterface::IsKeyPushedInSingle(KB_N))
    {
        _uic->GetUiOwner()->
            GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlayBgm("test", 0.8f);
    }
    if (InputInterface::IsKeyPushedInSingle(KB_M))
    {
        _uic->GetUiOwner()->
            GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlayBgm("test", 0.4f);
    }

    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test1\n");
        _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("sample1-scene", "sample1-scene.json");
    }
}

void TestUSpBtnInput(UInputComponent* _uic, Timer& _timer)
{
    auto ubc = _uic->GetUiOwner()->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc) { return; }

    if (InputInterface::IsKeyPushedInSingle(KB_UP))
    {
        ubc->SelectUpBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_LEFT))
    {
        ubc->SelectLeftBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_DOWN))
    {
        ubc->SelectDownBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_RIGHT))
    {
        ubc->SelectRightBtn();
    }

    if (ubc->IsCursorOnBtn() && InputInterface::IsKeyPushedInSingle(M_LEFTBTN))
    {
        P_LOG(LOG_DEBUG, "this btn has been click : %s\n", ubc->GetCompName().c_str());
    }
}

bool TestUSpInit(UInteractComponent* _uitc)
{
    P_LOG(LOG_DEBUG, "u sp init\n");
    return true;
}

void TestUSpUpdate(UInteractComponent* _uitc, Timer& _timer)
{

}

void TestUSpDestory(UInteractComponent* _uitc)
{
    P_LOG(LOG_DEBUG, "u sp destory\n");
}

void TempToTitle(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to title\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("title-scene", "title-scene.json");
    }
}

void TempToSelect(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to select\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("select-scene", "select-scene.json");
    }
}

void TempToRun(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to run\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("run-scene", "run-scene.json");
    }
}

void TempToResult(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to result\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("result-scene", "result-scene.json");
    }
}

static float g_AniSpdFactor = 50.f;
static AAnimateComponent* g_Aanc = nullptr;

SUBMESH_BONES* TempGetBoneData()
{
    return g_Aanc->GetBonesData();
}

bool AniInit(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "animate init\n");

    std::string aniCompName = _aitc->GetActorOwner()->GetObjectName() +
        "-animate";
    COMP_TYPE aniCompType = COMP_TYPE::A_ANIMATE;
    AAnimateComponent aanc(aniCompName, _aitc->GetActorOwner());
    aanc.ChangeAnimationTo("run");
    aanc.SetSpeedFactor(50.f);
    _aitc->GetActorOwner()->AddAComponent(aniCompType);
    _aitc->GetActorOwner()->GetSceneNode().GetComponentContainer()->
        AddComponent(aniCompType, aanc);

    g_Aanc = _aitc->GetActorOwner()->
        GetAComponent<AAnimateComponent>(COMP_TYPE::A_ANIMATE);
    if (!g_Aanc) { return false; }

    return true;
}

void AniUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
        RotateYAsix(_timer.FloatDeltaTime() / 1000.f);

    if (InputInterface::IsKeyPushedInSingle(KB_1))
    {
        g_Aanc->ChangeAnimationTo("run");
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_2))
    {
        g_Aanc->ChangeAnimationTo("bite");
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_3))
    {
        g_Aanc->ChangeAnimationTo("roar");
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_4))
    {
        g_Aanc->ChangeAnimationTo("attack_tail");
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_5))
    {
        g_Aanc->ChangeAnimationTo("idle");
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_UP))
    {
        g_AniSpdFactor += 50.f;
        g_Aanc->SetSpeedFactor(g_AniSpdFactor);
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_DOWN))
    {
        g_AniSpdFactor -= 50.f;
        g_Aanc->SetSpeedFactor(g_AniSpdFactor);
    }
}

void AniDestory(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "animate destory\n");
}
