#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "PlayerProcess.h"
#include "WM_Interface.h"
#include <vector>
#include "BulletProcess.h"
#include "FadeProcess.h"
#include "PauseMenu.h"

using namespace DirectX;

void RegisterPlayerProcess(ObjectFactory* _factory)
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

static DirectX::XMFLOAT3 g_PlayerMoveVec = { 0.f,0.f,0.f };

static float g_PlayerYAsixSpeed = 0.f;
static bool g_PlayerCanJumpFlg = false;
static std::vector<std::string> g_GroundObjNameVec = {};

static ATransformComponent* g_PlayerAngleAtc = nullptr;

static bool g_PlayerCanDashFlg = false;
static bool g_PlayerIsDashing = false;
static float g_DashTimer = 0.f;

static ATransformComponent* g_PlayerGunAtc = nullptr;
static float g_GunYAngleOffset = DirectX::XM_PIDIV4;
static bool g_PlayerIsAming = false;
static bool g_PlayerCanShoot = false;

static ATransformComponent* g_LastReachGroundAtc = nullptr;

static bool g_ResetDeadPlayerToGround = false;

void PlayerInput(AInputComponent* _aic, Timer& _timer)
{
    if (GetGamePauseFlg() || GetSceneInFlg()) { return; }
    if (InputInterface::IsKeyPushedInSingle(KB_BACKSPACE))
    {
        SetSceneOutFlg(true);
    }
    if (GetSceneOutFinish())
    {
        P_LOG(LOG_DEBUG, "to result\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("result-scene", "result-scene.json");
    }

    float deltatime = _timer.FloatDeltaTime();

    auto mouseOffset = InputInterface::GetMouseOffset();
    float horiR = mouseOffset.x * deltatime / 2000.f;
    float vertR = -mouseOffset.y * deltatime / 2000.f;
    _aic->GetActorOwner()->GetSceneNode().GetMainCamera()->
        RotateRSCamera(vertR, horiR);

    DirectX::XMFLOAT3 camOffset = { 0.f,0.f,0.f };
    auto atc = _aic->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    atc->RotateYAsix(horiR);

    g_PlayerAngleAtc->Rotate({ vertR,horiR,0.f });

    static const DirectX::XMFLOAT3 ident = { 0.f,0.f,1.f };
    static const DirectX::XMVECTOR identVec = DirectX::XMLoadFloat3(&ident);
    static DirectX::XMFLOAT3 lookAt = { 0.f,0.f,0.f };

    if (InputInterface::IsKeyDownInSingle(M_RIGHTBTN))
    {
        g_PlayerIsAming = true;
        if (GetPlayerCanAimFlg()) { g_GunYAngleOffset -= deltatime / 150.f; }
        else { g_GunYAngleOffset += deltatime / 150.f; }
    }
    else
    {
        g_GunYAngleOffset += deltatime / 150.f;
        g_PlayerIsAming = false;
    }
    if (g_GunYAngleOffset >= DirectX::XM_PIDIV4)
    {
        g_GunYAngleOffset = DirectX::XM_PIDIV4;
        g_PlayerCanShoot = false;
    }
    else if (g_GunYAngleOffset <= 0.f)
    {
        g_GunYAngleOffset = 0.f;
        g_PlayerCanShoot = true;
    }
    else
    {
        g_PlayerCanShoot = false;
    }
    float aimSlow = (g_PlayerIsAming && GetPlayerCanAimFlg()) ? 0.1f : 1.f;
    deltatime *= aimSlow;

    DirectX::XMFLOAT3 angle = g_PlayerAngleAtc->GetProcessingRotation();
    angle.x *= -1.f;
    angle.y += g_GunYAngleOffset;
    g_PlayerGunAtc->SetRotation(angle);

    if (g_PlayerCanDashFlg && InputInterface::IsKeyPushedInSingle(KB_LALT))
    {
        g_PlayerCanDashFlg = false;
        g_PlayerIsDashing = true;
        g_PlayerCanJumpFlg = false;
        g_DashTimer = 0.f;
        g_PlayerYAsixSpeed = 0.f;
        lookAt = g_PlayerAngleAtc->GetProcessingRotation();
        DirectX::XMMATRIX mat = DirectX::XMMatrixRotationX(lookAt.x) *
            DirectX::XMMatrixRotationY(lookAt.y);
        DirectX::XMVECTOR lookAtVec = {};
        lookAtVec = DirectX::XMVector3TransformNormal(identVec, mat);
        lookAtVec = DirectX::XMVector3Normalize(lookAtVec);
        DirectX::XMStoreFloat3(&lookAt, lookAtVec);
    }

    if (g_PlayerCanShoot && InputInterface::IsKeyPushedInSingle(M_LEFTBTN))
    {
        lookAt = g_PlayerAngleAtc->GetProcessingRotation();
        DirectX::XMMATRIX mat = DirectX::XMMatrixRotationX(lookAt.x) *
            DirectX::XMMatrixRotationY(lookAt.y);
        DirectX::XMVECTOR lookAtVec = {};
        lookAtVec = DirectX::XMVector3TransformNormal(identVec, mat);
        lookAtVec = DirectX::XMVector3Normalize(lookAtVec);
        DirectX::XMStoreFloat3(&lookAt, lookAtVec);
        DirectX::XMFLOAT3 shootPos = atc->GetProcessingPosition();
        DirectX::XMFLOAT3 shootAt = lookAt;
        shootPos.x += shootAt.x * 2.f;
        shootPos.y += shootAt.y * 2.f;
        shootPos.z += shootAt.z * 2.f;
        shootAt.y *= -1.f;
        SetBulletShoot(shootPos, shootAt);
    }

    if (!g_PlayerIsDashing)
    {
        float angleY = atc->GetProcessingRotation().y;
        DirectX::XMVECTOR frontVec = DirectX::XMVector3TransformNormal(
            identVec, DirectX::XMMatrixRotationY(angleY));
        DirectX::XMVECTOR rightVec = DirectX::XMVector3TransformNormal(
            identVec, DirectX::XMMatrixRotationY(angleY + DirectX::XM_PIDIV2));
        DirectX::XMVECTOR moveVec = DirectX::XMVectorZero();

        if (InputInterface::IsKeyDownInSingle(KB_W)) { moveVec += frontVec; }
        if (InputInterface::IsKeyDownInSingle(KB_A)) { moveVec -= rightVec; }
        if (InputInterface::IsKeyDownInSingle(KB_S)) { moveVec -= frontVec; }
        if (InputInterface::IsKeyDownInSingle(KB_D)) { moveVec += rightVec; }
        moveVec = DirectX::XMVector3Normalize(moveVec);
        DirectX::XMStoreFloat3(&g_PlayerMoveVec, moveVec);
        atc->TranslateZAsix(0.02f * deltatime * g_PlayerMoveVec.z);
        atc->TranslateXAsix(0.02f * deltatime * g_PlayerMoveVec.x);

        if (g_PlayerCanJumpFlg && InputInterface::IsKeyPushedInSingle(KB_SPACE))
        {
            g_PlayerCanJumpFlg = false;
            g_PlayerYAsixSpeed = 50.f;
        }

        g_PlayerYAsixSpeed -= 120.f * (deltatime / 1000.f);
        atc->TranslateYAsix(g_PlayerYAsixSpeed * (deltatime / 1000.f));
    }
    else
    {
        atc->TranslateZAsix(0.5f * deltatime * lookAt.z);
        atc->TranslateYAsix(0.5f * deltatime * -lookAt.y);
        atc->TranslateXAsix(0.5f * deltatime * lookAt.x);
    }
}

bool PlayerInit(AInteractComponent* _aitc)
{
    g_PlayerCanJumpFlg = false;

    _aitc->GetActorOwner()->GetSceneNode().GetMainCamera()->
        RotateRSCamera(0.f, _aitc->GetActorOwner()->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            GetRotation().y);

    g_PlayerYAsixSpeed = 0.f;

    g_GroundObjNameVec.clear();
    int groundSize = 1;
    for (int i = 0; i < groundSize; i++)
    {
        g_GroundObjNameVec.push_back("ground-" + std::to_string(i) + "-actor");
    }

    ShowCursor(FALSE);

    g_PlayerAngleAtc = (ATransformComponent*)(_aitc->
        GetActorOwner()->GetSceneNode().
        GetComponentContainer()->GetComponent("player-angle-actor-transform"));

    g_PlayerGunAtc = (ATransformComponent*)(_aitc->
        GetActorOwner()->GetSceneNode().
        GetComponentContainer()->GetComponent("player-gun-actor-transform"));
    g_GunYAngleOffset = DirectX::XM_PIDIV4;
    g_PlayerIsAming = false;
    g_PlayerCanShoot = false;

    g_PlayerCanDashFlg = true;
    g_PlayerIsDashing = false;
    g_DashTimer = 0.f;

    g_LastReachGroundAtc = nullptr;

    return true;
}

void PlayerUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    if (GetGamePauseFlg()) { return; }

    auto atc = _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    DirectX::XMFLOAT3 camOffset = atc->GetProcessingPosition();
    _aitc->GetActorOwner()->GetSceneNode().GetMainCamera()->
        ChangeRSCameraPosition(camOffset);

    g_PlayerGunAtc->SetPosition(atc->GetProcessingPosition());

    float aimSlow = (g_PlayerIsAming && GetPlayerCanAimFlg()) ? 0.1f : 1.f;

    if (g_PlayerIsDashing)
    {
        g_DashTimer += _timer.FloatDeltaTime() * aimSlow;
        if (g_DashTimer > 200.f)
        {
            g_PlayerIsDashing = false;
            g_DashTimer = 0.f;
        }
    }

    if (atc->GetProcessingPosition().y < -50.f)
    {
        SetDeadFadeRunningFlg(true);
    }

    if (g_ResetDeadPlayerToGround)
    {
        if (!g_LastReachGroundAtc)
        {
            g_LastReachGroundAtc = _aitc->GetActorOwner()->GetSceneNode().
                GetActorObject("ground-0-actor")->
                GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
        }
        DirectX::XMFLOAT3 rebirth = g_LastReachGroundAtc->GetPosition();
        rebirth.y += 10.f;
        atc->SetPosition(rebirth);
        g_PlayerYAsixSpeed = 0.f;
        g_PlayerCanJumpFlg = false;
        g_GunYAngleOffset = DirectX::XM_PIDIV4;
        g_PlayerIsAming = false;
        g_PlayerCanShoot = false;
        g_PlayerCanDashFlg = true;
        g_PlayerIsDashing = false;
        g_DashTimer = 0.f;
        g_ResetDeadPlayerToGround = false;
    }
}

void PlayerDestory(AInteractComponent* _aitc)
{
    _aitc->GetActorOwner()->GetSceneNode().GetMainCamera()->
        ResetRSCameraRotation({ 0.f,0.f,1.f }, { 0.f,1.f,0.f });
    ShowCursor(TRUE);
    RECT wndRect = {};
    GetClientRect(WindowInterface::GetWindowPtr()->GetWndHandle(), &wndRect);
    SetCursorPos((wndRect.right - wndRect.left) / 2,
        (wndRect.bottom - wndRect.top) / 2);
    g_PlayerAngleAtc = nullptr;
    g_LastReachGroundAtc = nullptr;
}

void SetPlayerDashFlg(bool _canDashFlg)
{
    g_PlayerCanDashFlg = _canDashFlg;
}

bool GetPlayerDashFlg()
{
    return g_PlayerCanDashFlg;
}

bool GetPlayerIsDashingFlg()
{
    return g_PlayerIsDashing;
}

bool GetPlayerAimingFlg()
{
    return g_PlayerIsAming;
}

void SetPlayerContactGround()
{
    g_PlayerYAsixSpeed = 0.f;
    g_PlayerCanJumpFlg = true;
    g_PlayerCanDashFlg = true;
    g_PlayerIsDashing = false;
    g_DashTimer = 0.f;
}

void SetPlayerBrokeHead()
{
    g_PlayerYAsixSpeed = 0.f;
    g_PlayerIsDashing = false;
    g_DashTimer = 0.f;
}

void SetPlayerDashToObstacle()
{
    g_PlayerYAsixSpeed = 0.f;
    g_PlayerIsDashing = false;
    g_DashTimer = 0.f;
}

DirectX::XMFLOAT3& GetPlayerMoveDirection()
{
    return g_PlayerMoveVec;
}

void SetPlayerMoveDirection(DirectX::XMFLOAT3 _dir)
{
    g_PlayerMoveVec = _dir;
}

void SetPlayerLastReachGround(ATransformComponent* _groundAtc)
{
    g_LastReachGroundAtc = _groundAtc;
}

void ResetDeadPlayerToGround()
{
    g_ResetDeadPlayerToGround = true;
}
