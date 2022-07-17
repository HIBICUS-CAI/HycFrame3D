#include "FadeProcess.h"
#include "PlayerProcess.h"

void RegisterFadeProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getUInitMapPtr().insert(
        { FUNC_NAME(DeadFadeInit),DeadFadeInit });
    _factory->getUUpdateMapPtr().insert(
        { FUNC_NAME(DeadFadeUpdate),DeadFadeUpdate });
    _factory->getUDestoryMapPtr().insert(
        { FUNC_NAME(DeadFadeDestory),DeadFadeDestory });
    _factory->getUInitMapPtr().insert(
        { FUNC_NAME(SceneFadeInit),SceneFadeInit });
    _factory->getUUpdateMapPtr().insert(
        { FUNC_NAME(SceneFadeUpdate),SceneFadeUpdate });
    _factory->getUDestoryMapPtr().insert(
        { FUNC_NAME(SceneFadeDestory),SceneFadeDestory });
}

static UTransformComponent* g_DeadFadesUtc[10] = { nullptr };

static bool g_DeadFadeRun = false;
static bool g_DeadFadeDestDown = true;

bool DeadFadeInit(UInteractComponent* _uitc)
{
    auto compContainer = _uitc->getUiOwner()->getSceneNode().
        getComponentContainer();
    for (UINT i = 0; i < ARRAYSIZE(g_DeadFadesUtc); i++)
    {
        std::string compName =
            "dead-black-" + std::to_string(i) + "-ui-transform";
        g_DeadFadesUtc[i] = (UTransformComponent*)(compContainer->
            getComponent(compName));
        if (!g_DeadFadesUtc[i]) { return false; }
    }

    g_DeadFadeRun = false;

    return true;
}

void DeadFadeUpdate(UInteractComponent* _uitc, const Timer& _timer)
{
    static float runTime = 0.f;
    if (g_DeadFadeRun)
    {
        float dest = g_DeadFadeDestDown ? 1.f : -1.f;
        if (runTime >= 0.f && runTime < 500.f)
        {
            for (UINT i = 0; i < 10; i++)
            {
                float startTime = (float)(i * 25);
                float offsetTime = runTime - startTime;
                if (offsetTime >= 0.f && offsetTime <= 250.f)
                {
                    float offsetY = 720.f - (720.f *
                        sinf(offsetTime / 250.f * DirectX::XM_PIDIV2));
                    DirectX::XMFLOAT3 pos = g_DeadFadesUtc[i]->getPosition();
                    pos.y = offsetY * dest;
                    g_DeadFadesUtc[i]->setPosition(pos);
                }
            }
        }
        else
        {
            if (runTime >= 1000.f)
            {
                _uitc->getUiOwner()->
                    getComponent<UAudioComponent>()->
                    playSe("reborn", 0.15f);
                runTime = 0.f;
                g_DeadFadeRun = false;
                g_DeadFadeDestDown = !g_DeadFadeDestDown;
                return;
            }
            else
            {
                for (UINT i = 0; i < 10; i++)
                {
                    ResetDeadPlayerToGround();
                    float startTime = (float)(i * 25);
                    float offsetTime = runTime - 500.f - startTime;
                    if (offsetTime >= 0.f && offsetTime <= 250.f)
                    {
                        float offsetY = 0.f - (720.f *
                            sinf(offsetTime / 250.f * DirectX::XM_PIDIV2));
                        DirectX::XMFLOAT3 pos = g_DeadFadesUtc[i]->getPosition();
                        pos.y = offsetY * dest;;
                        g_DeadFadesUtc[i]->setPosition(pos);
                    }
                }
            }
        }
        runTime += _timer.floatDeltaTime();
    }
}

void DeadFadeDestory(UInteractComponent* _uitc)
{
    g_DeadFadeRun = false;
}

bool GetDeadFadeRunningFlg()
{
    return g_DeadFadeRun;
}

void SetDeadFadeRunningFlg(bool _flag)
{
    g_DeadFadeRun = _flag;
}

static bool g_SceneInFlg = true;
static bool g_SceneOutFlg = false;
static bool g_SceneOutTrigger = false;

static UTransformComponent* g_SceneInUtc[2] = { nullptr };
static UTransformComponent* g_SceneOutUtc[2] = { nullptr };

static float g_SceneInOutTimer = 0.f;

bool SceneFadeInit(UInteractComponent* _uitc)
{
    g_SceneInFlg = true;
    g_SceneOutFlg = false;
    g_SceneOutTrigger = false;
    g_SceneInOutTimer = 0.f;

    auto compContainer = _uitc->getUiOwner()->getSceneNode().
        getComponentContainer();
    for (UINT i = 0; i < 2; i++)
    {
        std::string name = "";
        name = "scene-in-" + std::to_string(i) + "-ui-transform";
        g_SceneInUtc[i] = (UTransformComponent*)(compContainer->
            getComponent(name));
        name = "scene-out-" + std::to_string(i) + "-ui-transform";
        g_SceneOutUtc[i] = (UTransformComponent*)(compContainer->
            getComponent(name));
    }

    return true;
}

void SceneFadeUpdate(UInteractComponent* _uitc, const Timer& _timer)
{
    if (g_SceneInFlg)
    {
        if (g_SceneInOutTimer > 500.f)
        {
            g_SceneInFlg = false;
            g_SceneInOutTimer = 0.f;
            return;
        }

        float sinVal = sinf(g_SceneInOutTimer / 500.f * DirectX::XM_PIDIV2);
        float absX = (sinVal) * 640.f + 320.f;
        DirectX::XMFLOAT3 pos = {};
        pos = g_SceneInUtc[0]->getPosition();
        pos.x = -1.f * absX;
        g_SceneInUtc[0]->setPosition(pos);
        pos = g_SceneInUtc[1]->getPosition();
        pos.x = 1.f * absX;
        g_SceneInUtc[1]->setPosition(pos);

        g_SceneInOutTimer += _timer.floatDeltaTime();
    }

    if (g_SceneOutFlg)
    {
        if (g_SceneInOutTimer > 500.f)
        {
            g_SceneOutFlg = false;
            g_SceneInOutTimer = 0.f;
            return;
        }

        float sinVal = sinf(g_SceneInOutTimer / 500.f * DirectX::XM_PIDIV2);
        float absY = (1.f - sinVal) * 360.f + 180.f;
        DirectX::XMFLOAT3 pos = {};
        pos = g_SceneOutUtc[0]->getPosition();
        pos.y = -1.f * absY;
        g_SceneOutUtc[0]->setPosition(pos);
        pos = g_SceneOutUtc[1]->getPosition();
        pos.y = 1.f * absY;
        g_SceneOutUtc[1]->setPosition(pos);

        g_SceneInOutTimer += _timer.floatDeltaTime();
    }
}

void SceneFadeDestory(UInteractComponent* _uitc)
{
    g_SceneInFlg = true;
    g_SceneOutFlg = false;
    g_SceneOutTrigger = false;
}

bool GetSceneInFlg()
{
    return g_SceneInFlg;
}

bool GetSceneOutFlg()
{
    return g_SceneOutFlg;
}

static UINT g_SceneOutFilter = (UINT)-1;

bool GetSceneOutFinish(UINT _filter)
{
    bool filter = (g_SceneOutFilter == (UINT)-1) ? true :
        ((g_SceneOutFilter == _filter) ? true : false);
    return g_SceneOutTrigger && !g_SceneOutFlg && filter;
}

void SetSceneOutFlg(bool _flag, UINT _filter)
{
    if (g_SceneOutTrigger || g_SceneOutFlg) { return; }
    g_SceneOutFlg = _flag;
    g_SceneOutTrigger = true;
    g_SceneInOutTimer = 0.f;
    g_SceneOutFilter = _filter;
}
