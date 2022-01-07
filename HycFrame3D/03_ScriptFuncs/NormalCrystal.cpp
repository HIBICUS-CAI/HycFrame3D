#include "NormalCrystal.h"
#include "PlayerProcess.h"
#include <unordered_map>

static const DirectX::XMFLOAT3 NCRYSTAL_COLOR = { 0.f,0.9f,0.f };
static std::unordered_map<AInteractComponent*, float> g_NCrystalTimerMap = {};

void RegisterNormalCrystal(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(NCrystalInit),NCrystalInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(NCrystalUpdate),NCrystalUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(NCrystalDestory),NCrystalDestory });
}

bool NCrystalInit(AInteractComponent* _aitc)
{
    g_NCrystalTimerMap.insert({ _aitc,5.f });

    return true;
}

void NCrystalUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    auto acc = _aitc->GetActorOwner()->
        GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
    auto found = g_NCrystalTimerMap.find(_aitc);
    bool active = found->second >= 5.f;

    if (!GetPlayerDashFlg() && active && acc->CheckCollisionWith(PLAYER_NAME))
    {
        _aitc->GetActorOwner()->
            GetAComponent<AAudioComponent>(COMP_TYPE::A_AUDIO)->
            PlaySe("crystal-hit", 0.7f);
        SetPlayerDashFlg(true);
        found->second = 0.f;
    }

    float lightRatio = found->second / 5.f - 0.5f;
    if (active) { lightRatio = 1.f; }
    auto alc = _aitc->GetActorOwner()->
        GetAComponent<ALightComponent>(COMP_TYPE::A_LIGHT);
    alc->GetLightInfo()->SetRSLightStrength(
        { 0.f, NCRYSTAL_COLOR.y * lightRatio, 0.f });
    alc->GetLightInfo()->UpdateBloomColor();

    found->second += _timer.FloatDeltaTime() / 1000.f;
    if (found->second >= 5.f) { found->second = 5.f; }
}

void NCrystalDestory(AInteractComponent* _aitc)
{
    g_NCrystalTimerMap.erase(_aitc);
}
