#include "SleepCrystal.h"
#include <unordered_map>
#include "BulletProcess.h"
#include "PlayerProcess.h"

#include <RSUtilityFunctions.h>

static const DirectX::XMFLOAT3 SCRYSTAL_BEFIRE_COLOR = { 1.f,0.f,1.f };
static const DirectX::XMFLOAT3 SCRYSTAL_AFTER_COLOR = {0.f, 0.f, 1.f};
static const DirectX::XMFLOAT3 SCRYSTAL_SLEEP_COLOR = {1.f, 0.f, 0.f};
static std::unordered_map<AInteractComponent*, float> g_SCrystalTimerMap = {};
static std::unordered_map<AInteractComponent*, BOOL> g_SCrystalActiveMap = {};

void RegisterSleepCrystal(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(SCrystalInit),SCrystalInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(SCrystalUpdate),SCrystalUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(SCrystalDestory),SCrystalDestory });
}

bool SCrystalInit(AInteractComponent* _aitc)
{
    g_SCrystalTimerMap.insert({ _aitc,5.f });
    g_SCrystalActiveMap.insert({ _aitc,FALSE });

    return true;
}

void SCrystalUpdate(AInteractComponent* _aitc, const Timer& _timer)
{
    auto found = g_SCrystalTimerMap.find(_aitc);
    BOOL isActive = g_SCrystalActiveMap[_aitc];
    auto alc = _aitc->getActorOwner()->
        getComponent<ALightComponent>();

    if (isActive && found->second > 0.f)
    {
        float ratio = 1.f - ((5.f - found->second) / 5.f);
        ratio =
          (ratio > 0.2f) ? 1.f : rs_tool::lerp(1.f, 0.f, (0.2f - ratio) / 0.2f);
        found->second -= _timer.floatDeltaTime() / 1000.f;
        dx::XMFLOAT3 Color = {0.f, 0.f, 0.f};
        Color.x = rs_tool::lerp(SCRYSTAL_SLEEP_COLOR.x, SCRYSTAL_AFTER_COLOR.x, ratio);
        Color.z =
            rs_tool::lerp(SCRYSTAL_SLEEP_COLOR.z, SCRYSTAL_AFTER_COLOR.z, ratio);
        alc->getLightInfo()->setRSLightAlbedo(Color);
        alc->getLightInfo()->setRSLightIntensity(900.f);
        alc->getLightInfo()->updateBloomColor();
    }
    else if (isActive && found->second <= 0.f)
    {
        g_SCrystalActiveMap[_aitc] = FALSE;
        found->second = 0.f;
        alc->getLightInfo()->setRSLightAlbedo(SCRYSTAL_SLEEP_COLOR);
        alc->getLightInfo()->setRSLightIntensity(900.f);
        alc->getLightInfo()->updateBloomColor();
    }
    else if (found->second < 5.f)
    {
        float ratio = found->second / 5.f;
        ratio = (ratio < 0.5f)
                       ? 0.f
                       : rs_tool::lerp(0.f, 1.f, (ratio - 0.5f) / 0.5f);
        found->second += _timer.floatDeltaTime() / 1000.f;
        dx::XMFLOAT3 Color = {1.f, 0.f, 0.f};
        Color.z = rs_tool::lerp(SCRYSTAL_SLEEP_COLOR.z, SCRYSTAL_BEFIRE_COLOR.z,
                                ratio);
        alc->getLightInfo()->setRSLightAlbedo(Color);
        alc->getLightInfo()->setRSLightIntensity(
            rs_tool ::lerp(900.f, 600.f, ratio));
        alc->getLightInfo()->updateBloomColor();
    }
    else
    {
        found->second = 5.f;
        alc->getLightInfo()->setRSLightAlbedo(SCRYSTAL_BEFIRE_COLOR);
        alc->getLightInfo()->setRSLightIntensity(600.f);
        alc->getLightInfo()->updateBloomColor();
    }

    isActive = g_SCrystalActiveMap[_aitc];
    auto acc = _aitc->getActorOwner()->
        getComponent<ACollisionComponent>();
    if (!isActive && found->second >= 5.f)
    {
        if (CheckCollisionWithBullet(acc))
        {
            _aitc->getActorOwner()->
                getComponent<AAudioComponent>()->
                playSe("wake-crystal", 0.5f);
            g_SCrystalActiveMap[_aitc] = TRUE;
            found->second = 5.f;
        }
    }

    if (g_SCrystalActiveMap[_aitc] && !GetPlayerDashFlg() &&
        acc->checkCollisionWith(PLAYER_NAME))
    {
        _aitc->getActorOwner()->
            getComponent<AAudioComponent>()->
            playSe("crystal-hit", 0.7f);
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
