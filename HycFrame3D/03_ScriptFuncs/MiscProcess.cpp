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
