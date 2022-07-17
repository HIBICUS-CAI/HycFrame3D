#include "MiscProcess.h"

void RegisterMiscProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(DragonInit),DragonInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(DragonUpdate),DragonUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(DragonDestory),DragonDestory });
    _factory->getUInitMapPtr().insert(
        { FUNC_NAME(HillInfoInit),HillInfoInit });
    _factory->getUUpdateMapPtr().insert(
        { FUNC_NAME(HillInfoUpdate),HillInfoUpdate });
    _factory->getUDestoryMapPtr().insert(
        { FUNC_NAME(HillInfoDestory),HillInfoDestory });
    _factory->getUInitMapPtr().insert(
        { FUNC_NAME(ResultInit),ResultInit });
    _factory->getUUpdateMapPtr().insert(
        { FUNC_NAME(ResultUpdate),ResultUpdate });
    _factory->getUDestoryMapPtr().insert(
        { FUNC_NAME(ResultDestory),ResultDestory });
    _factory->getUInitMapPtr().insert(
        { FUNC_NAME(LogoFadeInit),LogoFadeInit });
    _factory->getUUpdateMapPtr().insert(
        { FUNC_NAME(LogoFadeUpdate),LogoFadeUpdate });
    _factory->getUDestoryMapPtr().insert(
        { FUNC_NAME(LogoFadeDestory),LogoFadeDestory });
}

static ATransformComponent* g_DragonAtc = nullptr;

bool DragonInit(AInteractComponent* _aitc)
{
    g_DragonAtc = _aitc->getActorOwner()->
        getComponent<ATransformComponent>();
    if (!g_DragonAtc) { return false; }
    return true;
}

void DragonUpdate(AInteractComponent* _aitc, const Timer& _timer)
{
    g_DragonAtc->rotateYAsix(_timer.floatDeltaTime() / 8000.f);
}

void DragonDestory(AInteractComponent* _aitc)
{
    g_DragonAtc = nullptr;
}

static UButtonComponent* g_TutorialUbc = nullptr;
static UButtonComponent* g_Route1Ubc = nullptr;
static UButtonComponent* g_Route2Ubc = nullptr;
static USpriteComponent* g_TutorialInfoUsc = nullptr;
static USpriteComponent* g_Route1InfoUsc = nullptr;
static USpriteComponent* g_Route2InfoUsc = nullptr;

bool HillInfoInit(UInteractComponent* _uitc)
{
    g_TutorialUbc = nullptr;
    g_Route1Ubc = nullptr;
    g_Route2Ubc = nullptr;
    g_TutorialInfoUsc = nullptr;
    g_Route1InfoUsc = nullptr;
    g_Route2InfoUsc = nullptr;
    return true;
}

void HillInfoUpdate(UInteractComponent* _uitc, const Timer& _timer)
{
    if (!g_TutorialUbc)
    {
        g_TutorialUbc = _uitc->getUiOwner()->getSceneNode().
            getUiObject("tutorial-btn-ui")->
            getComponent<UButtonComponent>();
    }
    if (!g_Route1Ubc)
    {
        g_Route1Ubc = _uitc->getUiOwner()->getSceneNode().
            getUiObject("route1-btn-ui")->
            getComponent<UButtonComponent>();
    }
    if (!g_Route2Ubc)
    {
        g_Route2Ubc = _uitc->getUiOwner()->getSceneNode().
            getUiObject("route2-btn-ui")->
            getComponent<UButtonComponent>();
    }
    if (!g_TutorialInfoUsc)
    {
        g_TutorialInfoUsc = _uitc->getUiOwner()->getSceneNode().
            getUiObject("text-tutorial-ui")->
            getComponent<USpriteComponent>();
    }
    if (!g_Route1InfoUsc)
    {
        g_Route1InfoUsc = _uitc->getUiOwner()->getSceneNode().
            getUiObject("text-route1-ui")->
            getComponent<USpriteComponent>();
    }
    if (!g_Route2InfoUsc)
    {
        g_Route2InfoUsc = _uitc->getUiOwner()->getSceneNode().
            getUiObject("text-route2-ui")->
            getComponent<USpriteComponent>();
    }

    g_TutorialInfoUsc->setOffsetColor({ 1.f,1.f,1.f,0.f });
    g_Route1InfoUsc->setOffsetColor({ 1.f,1.f,1.f,0.f });
    g_Route2InfoUsc->setOffsetColor({ 1.f,1.f,1.f,0.f });
    if (g_TutorialUbc->isBeingSelected())
    {
        g_TutorialInfoUsc->setOffsetColor({ 1.f,1.f,1.f,1.f });
    }
    if (g_Route1Ubc->isBeingSelected())
    {
        g_Route1InfoUsc->setOffsetColor({ 1.f,1.f,1.f,1.f });
    }
    if (g_Route2Ubc->isBeingSelected())
    {
        g_Route2InfoUsc->setOffsetColor({ 1.f,1.f,1.f,1.f });
    }
}

void HillInfoDestory(UInteractComponent* _uitc)
{
    g_TutorialUbc = nullptr;
    g_Route1Ubc = nullptr;
    g_Route2Ubc = nullptr;
    g_TutorialInfoUsc = nullptr;
    g_Route1InfoUsc = nullptr;
    g_Route2InfoUsc = nullptr;
}

bool ResultInit(UInteractComponent* _uitc)
{
    _uitc->getUiOwner()->getComponent<UAnimateComponent>()->
        changeAnimateTo("success");
    return true;
}

void ResultUpdate(UInteractComponent* _uitc, const Timer& _timer)
{

}

void ResultDestory(UInteractComponent* _uitc)
{

}

static USpriteComponent* g_LogoFadeUsc = nullptr;
static float g_LogoTimer = 0.f;

bool LogoFadeInit(UInteractComponent* _uitc)
{
    g_LogoTimer = 0.f;
    g_LogoFadeUsc = _uitc->getUiOwner()->
        getComponent<USpriteComponent>();
    if (!g_LogoFadeUsc) { return false; }
    return true;
}

void LogoFadeUpdate(UInteractComponent* _uitc, const Timer& _timer)
{
    if (g_LogoTimer < 1000.f)
    {
        g_LogoFadeUsc->setOffsetColor(
            { 1.f,1.f,1.f,1.f - (g_LogoTimer / 1000.f) });
    }
    else if (g_LogoTimer > 2500.f && g_LogoTimer < 3500.f)
    {
        g_LogoFadeUsc->setOffsetColor(
            { 1.f,1.f,1.f,(g_LogoTimer - 2500.f) / 1000.f });
    }
    else if (g_LogoTimer > 3500.f)
    {
        _uitc->getUiOwner()->getSceneNode().getSceneManager()->
            loadSceneNode("title-scene", "title-scene.json");
        stopBGM();
        setVolume("title", 0.2f);
        playBGM("title");
    }

    g_LogoTimer += _timer.floatDeltaTime();
}

void LogoFadeDestory(UInteractComponent* _uitc)
{
    g_LogoTimer = 0.f;
    g_LogoFadeUsc = nullptr;
}
