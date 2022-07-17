#include "BulletProcess.h"
#include <vector>
#include "PlayerProcess.h"
#include "PauseMenu.h"

using namespace DirectX;

static std::vector<ATransformComponent*> g_UsableBulletAtcVec = {};
static std::vector<std::pair<ATransformComponent*, DirectX::XMFLOAT3>>
g_UsingBulletAtcVec = {};

static ATransformComponent* g_PlayerAtc = nullptr;

static bool g_PlayerCanAim = true;

static float g_AimingRestTimer = 15.f;

static USpriteComponent* g_BulletIconUsc[3] = { nullptr };

void RegisterBulletProcess(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->getAInitMapPtr().insert(
        { FUNC_NAME(BulletManagerInit),BulletManagerInit });
    _factory->getAUpdateMapPtr().insert(
        { FUNC_NAME(BulletManagerUpdate),BulletManagerUpdate });
    _factory->getADestoryMapPtr().insert(
        { FUNC_NAME(BulletManagerDestory),BulletManagerDestory });
}

bool BulletManagerInit(AInteractComponent* _aitc)
{
    g_UsableBulletAtcVec.clear();
    g_UsingBulletAtcVec.clear();

    for (UINT i = 0; i < 3; i++)
    {
        std::string atcName = "bullet-" +
            std::to_string(i) + "-actor-transform";
        auto atc = (ATransformComponent*)(_aitc->getActorOwner()->getSceneNode().
            getComponentContainer()->getComponent(atcName));
        if (!atc) { return false; }
        g_UsableBulletAtcVec.push_back(atc);
    }

    g_PlayerAtc = (ATransformComponent*)(_aitc->getActorOwner()->getSceneNode().
        getComponentContainer()->getComponent("player-actor-transform"));
    if (!g_PlayerAtc) { return false; }

    g_PlayerCanAim = true;

    for (UINT i = 0; i < 3; i++)
    {
        std::string compName = "bullet-icon" + std::to_string(i) + "-ui-sprite";
        g_BulletIconUsc[i] = (USpriteComponent*)(_aitc->getActorOwner()->
            getSceneNode().getComponentContainer()->getComponent(compName));
    }

    return true;
}

void BulletManagerUpdate(AInteractComponent* _aitc,const Timer& _timer)
{
    if (GetGamePauseFlg()) { return; }

    if (GetPlayerAimingFlg())
    {
        g_AimingRestTimer -= _timer.floatDeltaTime() / 1000.f;
        if (g_AimingRestTimer < -3.f) { g_AimingRestTimer = -3.f; }
    }
    else
    {
        g_AimingRestTimer += _timer.floatDeltaTime() / 1000.f / 3.f * 5.f;
        if (g_AimingRestTimer > 15.f) { g_AimingRestTimer = 15.f; }
    }

    if (g_AimingRestTimer <= 0.05f) { g_PlayerCanAim = false; }
    else { g_PlayerCanAim = true; }

    float blt0IconAlpha = (g_AimingRestTimer - 10.f) / 5.f;
    float blt1IconAlpha = (g_AimingRestTimer - 5.f) / 5.f;
    float blt2IconAlpha = (g_AimingRestTimer) / 5.f;
    DirectX::XMFLOAT4 offset0 = { 1.f,1.f,1.f,blt0IconAlpha };
    DirectX::XMFLOAT4 offset1 = { 1.f,1.f,1.f,blt1IconAlpha };
    DirectX::XMFLOAT4 offset2 = { 1.f,1.f,1.f,blt2IconAlpha };
    g_BulletIconUsc[0]->setOffsetColor(offset0);
    g_BulletIconUsc[1]->setOffsetColor(offset1);
    g_BulletIconUsc[2]->setOffsetColor(offset2);

    float slowRatio = (GetPlayerAimingFlg() && g_PlayerCanAim) ? 0.1f : 1.f;
    float deltatime = _timer.floatDeltaTime() * slowRatio;
    //UINT usingSize = (UINT)g_UsingBulletAtcVec.size();
    for (auto i = g_UsingBulletAtcVec.begin(); i != g_UsingBulletAtcVec.end();)
    {
        auto& blt = *i;
        auto bltAtc = blt.first;
        auto& direction = blt.second;
        bltAtc->translateXAsix(direction.x * deltatime);
        bltAtc->translateYAsix(direction.y * deltatime);
        bltAtc->translateZAsix(direction.z * deltatime);

        DirectX::XMVECTOR vec = DirectX::XMLoadFloat3(
            &bltAtc->getProcessingPosition());
        DirectX::XMVECTOR playerPos = DirectX::XMLoadFloat3(
            &g_PlayerAtc->getProcessingPosition());
        vec -= playerPos;
        vec = DirectX::XMVector3Length(vec);
        if (DirectX::XMVectorGetX(vec) > 150.f)
        {
            i = g_UsingBulletAtcVec.erase(i);
            g_UsableBulletAtcVec.push_back(bltAtc);
        }
        else { ++i; }
    }

    for (auto& blt : g_UsableBulletAtcVec)
    {
        blt->setPosition(g_PlayerAtc->getPosition());
    }
}

void BulletManagerDestory(AInteractComponent* _aitc)
{
    g_UsableBulletAtcVec.clear();
    g_UsingBulletAtcVec.clear();
    g_PlayerAtc = nullptr;

    for (UINT i = 0; i < 3; i++)
    {
        g_BulletIconUsc[i] = nullptr;
    }
}

void SetBulletShoot(DirectX::XMFLOAT3& _pos, DirectX::XMFLOAT3& _vec)
{
    if (g_UsableBulletAtcVec.size())
    {
        auto atc = g_UsableBulletAtcVec.back();
        g_UsableBulletAtcVec.pop_back();
        atc->setPosition(_pos);
        DirectX::XMVECTOR norVec = DirectX::XMLoadFloat3(&_vec);
        norVec = DirectX::XMVector3Normalize(norVec);
        DirectX::XMFLOAT3 vec = { 0.f,0.f,0.f };
        DirectX::XMStoreFloat3(&vec, norVec);
        g_UsingBulletAtcVec.push_back({ atc,vec });

        if (g_AimingRestTimer > 10.f) { g_AimingRestTimer = 10.f; }
        else if (g_AimingRestTimer > 5.f) { g_AimingRestTimer = 5.f; }
        else { g_AimingRestTimer = 0.f; }
    }
}

bool GetPlayerCanAimFlg()
{
    return g_PlayerCanAim;
}

bool CheckCollisionWithBullet(ACollisionComponent* _acc)
{
    static std::string objName = "";
    for (auto& blt : g_UsingBulletAtcVec)
    {
        objName = blt.first->getActorOwner()->getObjectName();
        if (_acc->checkCollisionWith(objName)) { return true; }
    }

    return false;
}
