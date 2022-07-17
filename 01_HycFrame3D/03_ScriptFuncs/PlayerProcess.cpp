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
    _factory->getAInputMapPtr().insert(
        { FUNC_NAME(PlayerInput),PlayerInput });
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(PlayerInit),PlayerInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(PlayerUpdate),PlayerUpdate });
    _factory->getADestoryMapPtr().insert(
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

void PlayerInput(AInputComponent* _aic, const Timer& _timer)
{
    if (GetGamePauseFlg() || GetSceneInFlg()) { return; }

    float deltatime = _timer.floatDeltaTime();

    auto mouseOffset = input::getMouseOffset();
    if (!mouseOffset.x && !mouseOffset.y)
    {
        mouseOffset = input::rightStickOffset();
        mouseOffset.x /= 200;
        mouseOffset.y /= 200;
    }
    float horiR = mouseOffset.x * deltatime / 2000.f;
    float vertR = -mouseOffset.y * deltatime / 2000.f;

    g_PlayerAngleAtc->rotate({ vertR,horiR,0.f });
    if (fabsf(g_PlayerAngleAtc->getProcessingRotation().x) > XM_PIDIV2)
    {
        g_PlayerAngleAtc->rollBackRotationX();
        vertR = 0.f;
    }
    _aic->getActorOwner()->getSceneNode().getMainCamera()->
        rotateRSCamera(vertR, horiR);
    auto atc = _aic->getActorOwner()->
        getComponent<ATransformComponent>();
    atc->rotateYAsix(horiR);

    static const DirectX::XMFLOAT3 ident = { 0.f,0.f,1.f };
    static const DirectX::XMVECTOR identVec = DirectX::XMLoadFloat3(&ident);
    static DirectX::XMFLOAT3 lookAt = { 0.f,0.f,0.f };

    if (input::isKeyDownInSingle(M_RIGHTBTN) ||
        input::isKeyDownInSingle(GP_LEFTBACKSHDBTN))
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

    DirectX::XMFLOAT3 angle = g_PlayerAngleAtc->getProcessingRotation();
    angle.x *= -1.f;
    angle.y += g_GunYAngleOffset;
    g_PlayerGunAtc->setRotation(angle);

    if (g_PlayerCanDashFlg && (input::isKeyPushedInSingle(KB_LALT) ||
        input::isKeyPushedInSingle(GP_RIGHTFORESHDBTN)))
    {
        _aic->getActorOwner()->
            getComponent<AAudioComponent>()->
            playSe("dash", 0.7f);
        g_PlayerCanDashFlg = false;
        g_PlayerIsDashing = true;
        g_PlayerCanJumpFlg = false;
        g_DashTimer = 0.f;
        g_PlayerYAsixSpeed = 0.f;
        lookAt = g_PlayerAngleAtc->getProcessingRotation();
        DirectX::XMMATRIX mat = DirectX::XMMatrixRotationX(lookAt.x) *
            DirectX::XMMatrixRotationY(lookAt.y);
        DirectX::XMVECTOR lookAtVec = {};
        lookAtVec = DirectX::XMVector3TransformNormal(identVec, mat);
        lookAtVec = DirectX::XMVector3Normalize(lookAtVec);
        DirectX::XMStoreFloat3(&lookAt, lookAtVec);
    }

    if (g_PlayerCanShoot && (input::isKeyPushedInSingle(M_LEFTBTN) ||
        input::isKeyPushedInSingle(GP_RIGHTBACKSHDBTN)))
    {
        _aic->getActorOwner()->
            getComponent<AAudioComponent>()->
            playSe("shoot", 0.3f);
        lookAt = g_PlayerAngleAtc->getProcessingRotation();
        DirectX::XMMATRIX mat = DirectX::XMMatrixRotationX(lookAt.x) *
            DirectX::XMMatrixRotationY(lookAt.y);
        DirectX::XMVECTOR lookAtVec = {};
        lookAtVec = DirectX::XMVector3TransformNormal(identVec, mat);
        lookAtVec = DirectX::XMVector3Normalize(lookAtVec);
        DirectX::XMStoreFloat3(&lookAt, lookAtVec);
        DirectX::XMFLOAT3 shootPos = atc->getProcessingPosition();
        DirectX::XMFLOAT3 shootAt = lookAt;
        shootAt.y *= -1.f;
        shootPos.x += shootAt.x * 2.f;
        shootPos.y += shootAt.y * 2.f;
        shootPos.z += shootAt.z * 2.f;
        SetBulletShoot(shootPos, shootAt);
    }

    if (!g_PlayerIsDashing)
    {
        float angleY = atc->getProcessingRotation().y;
        DirectX::XMVECTOR frontVec = DirectX::XMVector3TransformNormal(
            identVec, DirectX::XMMatrixRotationY(angleY));
        DirectX::XMVECTOR rightVec = DirectX::XMVector3TransformNormal(
            identVec, DirectX::XMMatrixRotationY(angleY + DirectX::XM_PIDIV2));
        DirectX::XMVECTOR moveVec = DirectX::XMVectorZero();

        if (input::isKeyDownInSingle(KB_W)) { moveVec += frontVec; }
        if (input::isKeyDownInSingle(KB_A)) { moveVec -= rightVec; }
        if (input::isKeyDownInSingle(KB_S)) { moveVec -= frontVec; }
        if (input::isKeyDownInSingle(KB_D)) { moveVec += rightVec; }
        auto leftStick = input::leftStickOffset();
        if (leftStick.x > 0) { moveVec += rightVec; }
        if (leftStick.x < 0) { moveVec -= rightVec; }
        if (leftStick.y > 0) { moveVec -= frontVec; }
        if (leftStick.y < 0) { moveVec += frontVec; }
        moveVec = DirectX::XMVector3Normalize(moveVec);
        DirectX::XMStoreFloat3(&g_PlayerMoveVec, moveVec);
        atc->translateZAsix(0.02f * deltatime * g_PlayerMoveVec.z);
        atc->translateXAsix(0.02f * deltatime * g_PlayerMoveVec.x);

        if (g_PlayerCanJumpFlg && (input::isKeyPushedInSingle(KB_SPACE) ||
            input::isKeyPushedInSingle(GP_LEFTFORESHDBTN) ||
            input::isKeyPushedInSingle(GP_BOTTOMBTN)))
        {
            g_PlayerCanJumpFlg = false;
            g_PlayerYAsixSpeed = 50.f;
        }

        g_PlayerYAsixSpeed -= 120.f * (deltatime / 1000.f);
        atc->translateYAsix(g_PlayerYAsixSpeed * (deltatime / 1000.f));
    }
    else
    {
        atc->translateZAsix(0.5f * deltatime * lookAt.z);
        atc->translateYAsix(0.5f * deltatime * -lookAt.y);
        atc->translateXAsix(0.5f * deltatime * lookAt.x);
    }
}

bool PlayerInit(AInteractComponent* _aitc)
{
    g_PlayerCanJumpFlg = false;

    _aitc->getActorOwner()->getSceneNode().getMainCamera()->
        rotateRSCamera(0.f, _aitc->getActorOwner()->
            getComponent<ATransformComponent>()->
            getRotation().y);

    g_PlayerYAsixSpeed = 0.f;

    g_GroundObjNameVec.clear();
    int groundSize = 1;
    for (int i = 0; i < groundSize; i++)
    {
        g_GroundObjNameVec.push_back("ground-" + std::to_string(i) + "-actor");
    }

    ShowCursor(FALSE);

    g_PlayerAngleAtc = (ATransformComponent*)(_aitc->
        getActorOwner()->getSceneNode().
        getComponentContainer()->getComponent("player-angle-actor-transform"));

    g_PlayerGunAtc = (ATransformComponent*)(_aitc->
        getActorOwner()->getSceneNode().
        getComponentContainer()->getComponent("player-gun-actor-transform"));
    g_GunYAngleOffset = DirectX::XM_PIDIV4;
    g_PlayerIsAming = false;
    g_PlayerCanShoot = false;

    g_PlayerCanDashFlg = true;
    g_PlayerIsDashing = false;
    g_DashTimer = 0.f;

    g_LastReachGroundAtc = nullptr;

    return true;
}

void PlayerUpdate(AInteractComponent* _aitc, const Timer& _timer)
{
    if (GetGamePauseFlg()) { return; }

    auto atc = _aitc->getActorOwner()->
        getComponent<ATransformComponent>();
    DirectX::XMFLOAT3 camOffset = atc->getProcessingPosition();
    _aitc->getActorOwner()->getSceneNode().getMainCamera()->
        changeRSCameraPosition(camOffset);

    g_PlayerGunAtc->setPosition(atc->getProcessingPosition());

    float aimSlow = (g_PlayerIsAming && GetPlayerCanAimFlg()) ? 0.1f : 1.f;

    if (g_PlayerIsDashing)
    {
        g_DashTimer += _timer.floatDeltaTime() * aimSlow;
        if (g_DashTimer > 200.f)
        {
            g_PlayerIsDashing = false;
            g_DashTimer = 0.f;
        }
    }

    if (atc->getProcessingPosition().y < -50.f)
    {
        SetDeadFadeRunningFlg(true);
    }

    if (g_ResetDeadPlayerToGround)
    {
        if (!g_LastReachGroundAtc)
        {
            g_LastReachGroundAtc = _aitc->getActorOwner()->getSceneNode().
                getActorObject("ground-0-actor")->
                getComponent<ATransformComponent>();
        }
        DirectX::XMFLOAT3 rebirth = g_LastReachGroundAtc->getPosition();
        rebirth.y += 10.f;
        atc->setPosition(rebirth);
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
    _aitc->getActorOwner()->getSceneNode().getMainCamera()->
        resetRSCameraRotation({ 0.f,0.f,1.f }, { 0.f,1.f,0.f });
    ShowCursor(TRUE);
    RECT wndRect = {};
    GetClientRect(window::getWindowPtr()->getWndHandle(), &wndRect);
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
