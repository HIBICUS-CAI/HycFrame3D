#include "DestProcess.h"
#include "PlayerProcess.h"
#include "FadeProcess.h"

void RegisterDestProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(DestInit),DestInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(DestUpdate),DestUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(DestDestory),DestDestory });
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(DestPtcInit),DestPtcInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(DestPtcUpdate),DestPtcUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(DestPtcDestory),DestPtcDestory });
}

static ACollisionComponent* g_DestAcc = nullptr;
static ATransformComponent* g_DestAtc = nullptr;
static ACollisionComponent* g_DestPtcAcc = nullptr;
static AParticleComponent* g_DestPtcApc = nullptr;

static const UINT DEST_REACH = 0;

bool DestInit(AInteractComponent* _aitc)
{
    g_DestAcc = _aitc->getActorOwner()->
        getComponent<ACollisionComponent>();
    if (!g_DestAcc) { return false; }

    g_DestAtc = _aitc->getActorOwner()->
        getComponent<ATransformComponent>();
    if (!g_DestAtc) { return false; }

    return true;
}


void DestUpdate(AInteractComponent* _aitc, const Timer& _timer)
{
    g_DestAtc->rotateYAsix(_timer.floatDeltaTime() / 1500.f);

    if (g_DestAcc->checkCollisionWith(PLAYER_NAME))
    {
        stopBGM();
        _aitc->getActorOwner()->
            getComponent<AAudioComponent>()->
            playSe("goal", 0.3f);
        SetSceneOutFlg(true, DEST_REACH);
    }

    if (GetSceneOutFinish(DEST_REACH))
    {
        P_LOG(LOG_DEBUG, "to result\n");
        _aitc->getActorOwner()->getSceneNode().getSceneManager()->
            loadSceneNode("result-scene", "result-scene.json");
        setVolume("result", 0.2f);
        playBGM("result");
    }
}

void DestDestory(AInteractComponent* _aitc)
{
    g_DestAcc = nullptr;
    g_DestAtc = nullptr;
}

bool DestPtcInit(AInteractComponent* _aitc)
{
    g_DestPtcApc = _aitc->getActorOwner()->
        getComponent<AParticleComponent>();
    if (!g_DestPtcApc) { return false; }

    g_DestPtcAcc = _aitc->getActorOwner()->
        getComponent<ACollisionComponent>();
    if (!g_DestPtcAcc) { return false; }

    return true;
}

void DestPtcUpdate(AInteractComponent* _aitc, const Timer& _timer)
{
    if (g_DestPtcAcc->checkCollisionWith(PLAYER_NAME))
    {
        g_DestPtcApc->getEmitterInfo().EmitNumPerSecond = 2400.f;
    }
    else
    {
        g_DestPtcApc->getEmitterInfo().EmitNumPerSecond = 800.f;
    }
}

void DestPtcDestory(AInteractComponent* _aitc)
{
    g_DestAcc = nullptr;
    g_DestPtcAcc = nullptr;
    g_DestPtcApc = nullptr;
}
