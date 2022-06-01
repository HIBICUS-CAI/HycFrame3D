#include "GM02.h"
#include "RSCamera.h"

void RegisterGM02(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(PlayerInput),PlayerInput });
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(PlayerInit),PlayerInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(PlayerUpdate),PlayerUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(PlayerDestory),PlayerDestory });
}

static ATransformComponent* g_PlayerAtc = nullptr;
static RSCamera* g_Cam = nullptr;

void PlayerInput(AInputComponent* _aic, Timer& _timer)
{
    float deltatime = _timer.FloatDeltaTime();

    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        g_PlayerAtc->TranslateXAsix(-0.2f * deltatime);
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        g_PlayerAtc->TranslateXAsix(0.2f * deltatime);
    }

    if (fabsf(g_PlayerAtc->GetProcessingPosition().x) > 80.f)
    {
        g_PlayerAtc->RollBackPositionX();
    }
}

bool PlayerInit(AInteractComponent* _aitc)
{
    g_PlayerAtc = _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    if (!g_PlayerAtc) { return false; }
    g_Cam = _aitc->GetActorOwner()->GetSceneNode().GetMainCamera();
    if (!g_Cam) { return false; }

    return true;
}

void PlayerUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    using namespace DirectX;
    XMVECTOR processing = DirectX::XMLoadFloat3(
        &g_PlayerAtc->GetProcessingPosition());
    XMVECTOR origin = DirectX::XMLoadFloat3(
        &g_PlayerAtc->GetPosition());
    XMFLOAT3 deltaPos = {};
    XMStoreFloat3(&deltaPos, origin - processing);
    g_Cam->TranslateRSCamera(deltaPos);
}

void PlayerDestory(AInteractComponent* _aitc)
{
    g_PlayerAtc = nullptr;
    g_Cam = nullptr;
}
