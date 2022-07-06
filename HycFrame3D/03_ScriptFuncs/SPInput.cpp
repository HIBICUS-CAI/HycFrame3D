#include "SPInput.h"
#include "RSRoot_DX11.h"
#include "RSPipelinesManager.h"
#include "RSMeshHelper.h"

void RegisterSPInput(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getAInputMapPtr().insert(
        { FUNC_NAME(TestASpInput),TestASpInput });
    _factory->getAInputMapPtr().insert(
        { FUNC_NAME(TempToTitle),TempToTitle });
    _factory->getAInputMapPtr().insert(
        { FUNC_NAME(TempToSelect),TempToSelect });
    _factory->getAInputMapPtr().insert(
        { FUNC_NAME(TempToRun),TempToRun });
    _factory->getAInputMapPtr().insert(
        { FUNC_NAME(TempToResult),TempToResult });
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(TestASpInit),TestASpInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(TestASpUpdate),TestASpUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(TestASpDestory),TestASpDestory });
    _factory->getUInputMapPtr().insert(
        { FUNC_NAME(TestUSpInput),TestUSpInput });
    _factory->getUInputMapPtr().insert(
        { FUNC_NAME(TestUSpBtnInput),TestUSpBtnInput });
    _factory->getUInitMapPtr().insert(
        { FUNC_NAME(TestUSpInit),TestUSpInit });
    _factory->getUUpdateMapPtr().insert(
        { FUNC_NAME(TestUSpUpdate),TestUSpUpdate });
    _factory->getUDestoryMapPtr().insert(
        { FUNC_NAME(TestUSpDestory),TestUSpDestory });

    _factory->getAInitMapPtr().insert({ FUNC_NAME(AniInit),AniInit });
    _factory->getAUpdateMapPtr().insert({ FUNC_NAME(AniUpdate),AniUpdate });
    _factory->getADestoryMapPtr().insert({ FUNC_NAME(AniDestory),AniDestory });
    _factory->getAInitMapPtr().insert({ FUNC_NAME(BBInit),BBInit });
    _factory->getAUpdateMapPtr().insert({ FUNC_NAME(BBUpdate),BBUpdate });
    _factory->getADestoryMapPtr().insert({ FUNC_NAME(BBDestory),BBDestory });
}

void TestASpInput(AInputComponent* _aic, Timer& _timer)
{
    if (input::isKeyDownInSingle(M_LEFTBTN))
    {
        auto mouseOffset = input::getMouseOffset();
        float horiR = -mouseOffset.x * _timer.floatDeltaTime() / 800.f;
        _aic->getSceneNode().GetMainCamera()->rotateRSCamera(0.f, horiR);
    }

    if (input::isKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test2\n");
        _aic->getSceneNode().GetSceneManager()->
            LoadSceneNode("sample2-scene", "sample2-scene.json");
    }

    if (input::isKeyDownInSingle(KB_W))
    {
        _aic->getActorObject("sp-point-light-actor")->
            getComponent<ATransformComponent>()->
            translateZAsix(0.1f * _timer.floatDeltaTime());
    }
    if (input::isKeyDownInSingle(KB_A))
    {
        _aic->getActorObject("sp-point-light-actor")->
            getComponent<ATransformComponent>()->
            translateXAsix(-0.1f * _timer.floatDeltaTime());
    }
    if (input::isKeyDownInSingle(KB_S))
    {
        _aic->getActorObject("sp-point-light-actor")->
            getComponent<ATransformComponent>()->
            translateZAsix(-0.1f * _timer.floatDeltaTime());
    }
    if (input::isKeyDownInSingle(KB_D))
    {
        _aic->getActorObject("sp-point-light-actor")->
            getComponent<ATransformComponent>()->
            translateXAsix(0.1f * _timer.floatDeltaTime());
    }
    if (input::isKeyPushedInSingle(KB_P))
    {
        static bool simp = true;
        std::string basic = "light-pipeline";
        std::string simple = "simple-pipeline";
        if (simp)
        {
            getRSDX11RootInstance()->getPipelinesManager()->setPipeline(basic);
        }
        else
        {
            getRSDX11RootInstance()->getPipelinesManager()->setPipeline(simple);
        }
        simp = !simp;
    }
}

bool TestASpInit(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "a sp init\n");

    _aitc->getActorOwner()->
        getComponent<ATimerComponent>()->
        startTimer("timer1");

    return true;
}
void TestASpUpdate(AInteractComponent* _aitc, Timer&)
{
    //P_LOG(LOG_DEBUG, "a sp update\n");
    /*float time0 = _aitc->GetActorOwner()->
        GetComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        GetTimer("timer0")->mTime;
    float time1 = _aitc->GetActorOwner()->
        GetComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        GetTimer("timer1")->mTime;
    P_LOG(LOG_DEBUG, "timer0 : %f , timer1 : %f\n", time0, time1);*/

    CONTACT_PONT_PAIR contact = {};
    if (_aitc->getActorObject("sp-point-light-actor")->
        getComponent<ACollisionComponent>()->
        checkCollisionWith("sp-actor", &contact))
    {
        _aitc->getActorObject("sp-point-light-actor")->
            getComponent<ATransformComponent>()->
            rollBackPosition();
        P_LOG(LOG_DEBUG, "a : %f, %f, %f ; b : %f, %f, %f\n",
            contact.first.x, contact.first.y, contact.first.z,
            contact.second.x, contact.second.y, contact.second.z);
        auto center = ACollisionComponent::calcCenterOfContact(contact);
        P_LOG(LOG_DEBUG, "center of contact : %f, %f, %f\n",
            center.x, center.y, center.z);
    }

    _aitc->getActorObject("sp-particle-actor")->
        getComponent<ATransformComponent>()->
        setPosition(_aitc->getActorObject("sp-point-light-actor")->
            getComponent<ATransformComponent>()->
            getProcessingPosition());
    _aitc->getActorObject("sp-particle-actor")->
        getComponent<ATransformComponent>()->
        translateYAsix(5.f);
}

void TestASpDestory(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "a sp destory\n");
}

void TestUSpInput(UInputComponent* _uic, Timer& _timer)
{
    float delta = _timer.floatDeltaTime();
    auto utc = _uic->getUiOwner()->
        getComponent<UTransformComponent>();

    if (input::isKeyDownInSingle(KB_W))
    {
        utc->translateYAsix(0.1f * delta);
    }
    if (input::isKeyDownInSingle(KB_A))
    {
        utc->translateXAsix(-0.1f * delta);
    }
    if (input::isKeyDownInSingle(KB_S))
    {
        utc->translateYAsix(-0.1f * delta);
    }
    if (input::isKeyDownInSingle(KB_D))
    {
        utc->translateXAsix(0.1f * delta);
    }

    if (input::isKeyPushedInSingle(KB_Z))
    {
        _uic->getUiOwner()->
            getComponent<USpriteComponent>()->
            resetTexture();
    }
    if (input::isKeyPushedInSingle(KB_X))
    {
        _uic->getUiOwner()->
            getComponent<UAnimateComponent>()->
            changeAnimateTo("number");
    }
    if (input::isKeyPushedInSingle(KB_C))
    {
        _uic->getUiOwner()->
            getComponent<UAnimateComponent>()->
            changeAnimateTo("runman");
    }

    if (input::isKeyPushedInSingle(KB_N))
    {
        _uic->getUiOwner()->
            getComponent<UAudioComponent>()->
            playBgm("test", 0.8f);
    }
    if (input::isKeyPushedInSingle(KB_M))
    {
        _uic->getUiOwner()->
            getComponent<UAudioComponent>()->
            playBgm("test", 0.4f);
    }

    if (input::isKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test1\n");
        _uic->getSceneNode().GetSceneManager()->
            LoadSceneNode("sample1-scene", "sample1-scene.json");
    }
}

void TestUSpBtnInput(UInputComponent* _uic, Timer& _timer)
{
    auto ubc = _uic->getUiOwner()->
        getComponent<UButtonComponent>();
    if (!ubc) { return; }

    if (input::isKeyPushedInSingle(KB_UP))
    {
        ubc->selectUpBtn();
    }
    if (input::isKeyPushedInSingle(KB_LEFT))
    {
        ubc->selectLeftBtn();
    }
    if (input::isKeyPushedInSingle(KB_DOWN))
    {
        ubc->selectDownBtn();
    }
    if (input::isKeyPushedInSingle(KB_RIGHT))
    {
        ubc->selectRightBtn();
    }

    if (ubc->isCursorOnBtn() && input::isKeyPushedInSingle(M_LEFTBTN))
    {
        P_LOG(LOG_DEBUG, "this btn has been click : %s\n", ubc->getCompName().c_str());
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
    if (input::isKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to title\n");
        _aic->getSceneNode().GetSceneManager()->
            LoadSceneNode("title-scene", "title-scene.json");
    }
}

void TempToSelect(AInputComponent* _aic, Timer&)
{
    if (input::isKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to select\n");
        _aic->getSceneNode().GetSceneManager()->
            LoadSceneNode("select-scene", "select-scene.json");
    }
}

void TempToRun(AInputComponent* _aic, Timer&)
{
    if (input::isKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to run\n");
        _aic->getSceneNode().GetSceneManager()->
            LoadSceneNode("run-scene", "run-scene.json");
    }
}

void TempToResult(AInputComponent* _aic, Timer&)
{
    if (input::isKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to result\n");
        _aic->getSceneNode().GetSceneManager()->
            LoadSceneNode("result-scene", "result-scene.json");
    }
}

static float g_AniSpdFactor = 50.f;
static AAnimateComponent* g_Aanc = nullptr;

bool AniInit(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "animate init\n");

    g_Aanc = _aitc->getActorOwner()->
        getComponent<AAnimateComponent>();
    if (!g_Aanc) { return false; }

    return true;
}

void AniUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    _aitc->getActorOwner()->
        getComponent<ATransformComponent>()->
        rotateYAsix(_timer.floatDeltaTime() / 1000.f);

    if (input::isKeyPushedInSingle(KB_1))
    {
        g_Aanc->changeAnimationTo("run");
    }
    else if (input::isKeyPushedInSingle(KB_2))
    {
        g_Aanc->changeAnimationTo("bite");
    }
    else if (input::isKeyPushedInSingle(KB_3))
    {
        g_Aanc->changeAnimationTo("roar");
    }
    else if (input::isKeyPushedInSingle(KB_4))
    {
        g_Aanc->changeAnimationTo("attack_tail");
    }
    else if (input::isKeyPushedInSingle(KB_5))
    {
        g_Aanc->changeAnimationTo("idle");
    }
    else if (input::isKeyPushedInSingle(KB_UP))
    {
        g_AniSpdFactor += 10.f;
        g_Aanc->SetSpeedFactor(g_AniSpdFactor);
    }
    else if (input::isKeyPushedInSingle(KB_DOWN))
    {
        g_AniSpdFactor -= 10.f;
        g_Aanc->SetSpeedFactor(g_AniSpdFactor);
    }
}

void AniDestory(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "animate destory\n");
    g_Aanc = nullptr;
}

static ATransformComponent* g_BBAtc = nullptr;
static float g_XFactor = 0.f;

bool BBInit(AInteractComponent* _aitc)
{
    g_XFactor = -1.f;
    g_BBAtc = _aitc->getActorOwner()->
        getComponent<ATransformComponent>();
    if (!g_BBAtc) { return false; }

    return true;
}

void BBUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    g_BBAtc->translateXAsix(_timer.floatDeltaTime() * g_XFactor * 0.01f);
    if (fabsf(g_BBAtc->getProcessingPosition().x) > 18.f)
    {
        g_BBAtc->rollBackPositionX();
        g_XFactor *= -1.f;
    }
}

void BBDestory(AInteractComponent* _aitc)
{
    g_BBAtc = nullptr;
}
