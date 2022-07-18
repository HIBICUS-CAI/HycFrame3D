#include "NormalCrystal.h"
#include "PlayerProcess.h"
#include <RSUtilityFunctions.h>
#include <unordered_map>

static std::unordered_map<AInteractComponent*, float> g_NCrystalTimerMap = {};

void RegisterNormalCrystal(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(NCrystalInit),NCrystalInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(NCrystalUpdate),NCrystalUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(NCrystalDestory),NCrystalDestory });
}

bool NCrystalInit(AInteractComponent* _aitc)
{
    g_NCrystalTimerMap.insert({ _aitc,5.f });

    return true;
}

void NCrystalUpdate(AInteractComponent* _aitc, const Timer& _timer)
{
    auto acc = _aitc->getActorOwner()->
        getComponent<ACollisionComponent>();
    auto found = g_NCrystalTimerMap.find(_aitc);
    bool active = found->second >= 5.f;

    if (!GetPlayerDashFlg() && active && acc->checkCollisionWith(PLAYER_NAME))
    {
        _aitc->getActorOwner()->
            getComponent<AAudioComponent>()->
            playSe("crystal-hit", 0.7f);
        SetPlayerDashFlg(true);
        found->second = 0.f;
    }

    float lightRatio = found->second / 5.f;
    if (active) { lightRatio = 1.f; } 
    else {
      lightRatio = (lightRatio < 0.5f)
                       ? 0.f
                       : rs_tool::lerp(0.f, 1.f, (lightRatio - 0.5f) / 0.5f);
    }
    auto alc = _aitc->getActorOwner()->
        getComponent<ALightComponent>();
    dx::XMFLOAT3 Color = {};
    Color.x = rs_tool::lerp(1.f, 0.f, lightRatio);
    Color.y = rs_tool::lerp(0.f, 1.f, lightRatio);
    alc->getLightInfo()->setRSLightAlbedo(Color);
    alc->getLightInfo()->setRSLightIntensity(
        rs_tool::lerp(800.f, 300.f, lightRatio));
    alc->getLightInfo()->updateBloomColor();

    found->second += _timer.floatDeltaTime() / 1000.f;
    if (found->second >= 5.f) { found->second = 5.f; }
}

void NCrystalDestory(AInteractComponent* _aitc)
{
    g_NCrystalTimerMap.erase(_aitc);
}
