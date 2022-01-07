#include "MiscProcess.h"

void RegisterMiscProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(DragonInit),DragonInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(DragonUpdate),DragonUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(DragonDestory),DragonDestory });
    _factory->GetUInitMapPtr()->insert(
        { FUNC_NAME(HillInfoInit),HillInfoInit });
    _factory->GetUUpdateMapPtr()->insert(
        { FUNC_NAME(HillInfoUpdate),HillInfoUpdate });
    _factory->GetUDestoryMapPtr()->insert(
        { FUNC_NAME(HillInfoDestory),HillInfoDestory });
}

static ATransformComponent* g_DragonAtc = nullptr;

bool DragonInit(AInteractComponent* _aitc)
{
    g_DragonAtc = _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    if (!g_DragonAtc) { return false; }
    return true;
}

void DragonUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    g_DragonAtc->RotateYAsix(_timer.FloatDeltaTime() / 8000.f);
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

void HillInfoUpdate(UInteractComponent* _uitc, Timer& _timer)
{
    if (!g_TutorialUbc)
    {
        g_TutorialUbc = _uitc->GetUiOwner()->GetSceneNode().
            GetUiObject("tutorial-btn-ui")->
            GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    }
    if (!g_Route1Ubc)
    {
        g_Route1Ubc = _uitc->GetUiOwner()->GetSceneNode().
            GetUiObject("route1-btn-ui")->
            GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    }
    if (!g_Route2Ubc)
    {
        g_Route2Ubc = _uitc->GetUiOwner()->GetSceneNode().
            GetUiObject("route2-btn-ui")->
            GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    }
    if (!g_TutorialInfoUsc)
    {
        g_TutorialInfoUsc = _uitc->GetUiOwner()->GetSceneNode().
            GetUiObject("text-tutorial-ui")->
            GetUComponent<USpriteComponent>(COMP_TYPE::U_SPRITE);
    }
    if (!g_Route1InfoUsc)
    {
        g_Route1InfoUsc = _uitc->GetUiOwner()->GetSceneNode().
            GetUiObject("text-route1-ui")->
            GetUComponent<USpriteComponent>(COMP_TYPE::U_SPRITE);
    }
    if (!g_Route2InfoUsc)
    {
        g_Route2InfoUsc = _uitc->GetUiOwner()->GetSceneNode().
            GetUiObject("text-route2-ui")->
            GetUComponent<USpriteComponent>(COMP_TYPE::U_SPRITE);
    }

    g_TutorialInfoUsc->SetOffsetColor({ 1.f,1.f,1.f,0.f });
    g_Route1InfoUsc->SetOffsetColor({ 1.f,1.f,1.f,0.f });
    g_Route2InfoUsc->SetOffsetColor({ 1.f,1.f,1.f,0.f });
    if (g_TutorialUbc->IsBeingSelected())
    {
        g_TutorialInfoUsc->SetOffsetColor({ 1.f,1.f,1.f,1.f });
    }
    if (g_Route1Ubc->IsBeingSelected())
    {
        g_Route1InfoUsc->SetOffsetColor({ 1.f,1.f,1.f,1.f });
    }
    if (g_Route2Ubc->IsBeingSelected())
    {
        g_Route2InfoUsc->SetOffsetColor({ 1.f,1.f,1.f,1.f });
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
