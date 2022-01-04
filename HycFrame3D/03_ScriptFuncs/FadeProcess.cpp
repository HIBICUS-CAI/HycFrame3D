#include "FadeProcess.h"
#include "PlayerProcess.h"

void RegisterFadeProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetUInitMapPtr()->insert(
        { FUNC_NAME(DeadFadeInit),DeadFadeInit });
    _factory->GetUUpdateMapPtr()->insert(
        { FUNC_NAME(DeadFadeUpdate),DeadFadeUpdate });
    _factory->GetUDestoryMapPtr()->insert(
        { FUNC_NAME(DeadFadeDestory),DeadFadeDestory });
}

static UTransformComponent* g_DeadFadesUtc[10] = { nullptr };

static bool g_DeadFadeRun = false;
static bool g_DeadFadeDestDown = true;

bool DeadFadeInit(UInteractComponent* _uitc)
{
    auto compContainer = _uitc->GetUiOwner()->GetSceneNode().
        GetComponentContainer();
    for (UINT i = 0; i < ARRAYSIZE(g_DeadFadesUtc); i++)
    {
        std::string compName =
            "dead-black-" + std::to_string(i) + "-ui-transform";
        g_DeadFadesUtc[i] = (UTransformComponent*)(compContainer->
            GetComponent(compName));
        if (!g_DeadFadesUtc[i]) { return false; }
    }

    g_DeadFadeRun = false;

    return true;
}

void DeadFadeUpdate(UInteractComponent* _uitc, Timer& _timer)
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
                    DirectX::XMFLOAT3 pos = g_DeadFadesUtc[i]->GetPosition();
                    pos.y = offsetY * dest;
                    g_DeadFadesUtc[i]->SetPosition(pos);
                }
            }
        }
        else
        {
            if (runTime >= 1000.f)
            {
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
                        DirectX::XMFLOAT3 pos = g_DeadFadesUtc[i]->GetPosition();
                        pos.y = offsetY * dest;;
                        g_DeadFadesUtc[i]->SetPosition(pos);
                    }
                }
            }
        }
        runTime += _timer.FloatDeltaTime();
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
