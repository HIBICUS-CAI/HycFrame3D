#include "SleepCrystal.h"
#include <unordered_map>
#include "BulletProcess.h"
#include "PlayerProcess.h"

static const DirectX::XMFLOAT3 SCRYSTAL_BEFIRE_COLOR = { 0.9f,0.f,0.9f };
static const DirectX::XMFLOAT3 SCRYSTAL_AFTER_COLOR = { 0.f,0.f,0.9f };
static std::unordered_map<AInteractComponent*, float> g_SCrystalTimerMap = {};
static std::unordered_map<AInteractComponent*, BOOL> g_SCrystalActiveMap = {};

void RegisterSleepCrystal(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(SCrystalInit),SCrystalInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(SCrystalUpdate),SCrystalUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(SCrystalDestory),SCrystalDestory });
}

bool SCrystalInit(AInteractComponent* _aitc)
{
    g_SCrystalTimerMap.insert({ _aitc,5.f });
    g_SCrystalActiveMap.insert({ _aitc,FALSE });

    return true;
}

void SCrystalUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    auto found = g_SCrystalTimerMap.find(_aitc);
    BOOL isActive = g_SCrystalActiveMap[_aitc];
    auto alc = _aitc->GetActorOwner()->
        GetAComponent<ALightComponent>(COMP_TYPE::A_LIGHT);

    if (isActive && found->second > 0.f)
    {
        float ratio = 1.f - ((5.f - found->second) / 5.f) + 0.1f;
        found->second -= _timer.FloatDeltaTime() / 1000.f;
        alc->GetLightInfo()->SetRSLightStrength(
            { 0.f, 0.f, SCRYSTAL_AFTER_COLOR.z * ratio });
        alc->GetLightInfo()->UpdateBloomColor();
    }
    else if (isActive && found->second <= 0.f)
    {
        g_SCrystalActiveMap[_aitc] = FALSE;
        found->second = 0.f;
        alc->GetLightInfo()->SetRSLightStrength({ 0.f, 0.f, 0.f });
        alc->GetLightInfo()->UpdateBloomColor();
    }
    else if (found->second < 5.f)
    {
        float ratio = (found->second / 5.f) - 0.2f;
        found->second += _timer.FloatDeltaTime() / 1000.f;
        alc->GetLightInfo()->SetRSLightStrength(
            { SCRYSTAL_BEFIRE_COLOR.x * ratio, 0.f,
            SCRYSTAL_BEFIRE_COLOR.z * ratio });
        alc->GetLightInfo()->UpdateBloomColor();
    }
    else
    {
        found->second = 5.f;
        alc->GetLightInfo()->SetRSLightStrength(SCRYSTAL_BEFIRE_COLOR);
        alc->GetLightInfo()->UpdateBloomColor();
    }

    isActive = g_SCrystalActiveMap[_aitc];
    auto acc = _aitc->GetActorOwner()->
        GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
    if (!isActive && found->second >= 5.f)
    {
        if (CheckCollisionWithBullet(acc))
        {
            g_SCrystalActiveMap[_aitc] = TRUE;
            found->second = 5.f;
        }
    }

    if (g_SCrystalActiveMap[_aitc] && !GetPlayerDashFlg() &&
        acc->CheckCollisionWith(PLAYER_NAME))
    {
        SetPlayerDashFlg(true);
        g_SCrystalActiveMap[_aitc] = FALSE;
        found->second = 0.f;
    }
}

void SCrystalDestory(AInteractComponent* _aitc)
{
    g_SCrystalTimerMap.erase(_aitc);
    g_SCrystalActiveMap.erase(_aitc);
}
