#include "BulletProcess.h"

#include "PauseMenu.h"
#include "PlayerProcess.h"

#include <vector>

using namespace dx;

static std::vector<ATransformComponent *> G_UsableBulletAtcVec = {};
static std::vector<std::pair<ATransformComponent *, dx::XMFLOAT3>>
    G_UsingBulletAtcVec = {};

static ATransformComponent *G_PlayerAtc = nullptr;

static bool G_PlayerCanAim = true;

static float G_AimingRestTimer = 15.f;

static USpriteComponent *G_BulletIconUsc[3] = {nullptr};

void registerBulletProcess(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInitMapPtr().insert(
      {FUNC_NAME(bulletManagerInit), bulletManagerInit});
  Factory->getAUpdateMapPtr().insert(
      {FUNC_NAME(bulletManagerUpdate), bulletManagerUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(bulletManagerDestory), bulletManagerDestory});
}

bool bulletManagerInit(AInteractComponent *Aitc) {
  G_UsableBulletAtcVec.clear();
  G_UsingBulletAtcVec.clear();

  for (UINT I = 0; I < 3; I++) {
    std::string AtcName = "bullet-" + std::to_string(I) + "-actor-transform";
    auto Atc = static_cast<ATransformComponent *>(
        Aitc->getSceneNode().getComponentContainer()->getComponent(AtcName));
    if (!Atc) {
      return false;
    }
    G_UsableBulletAtcVec.push_back(Atc);
  }

  G_PlayerAtc = static_cast<ATransformComponent *>(
      Aitc->getSceneNode().getComponentContainer()->getComponent(
          "player-actor-transform"));
  if (!G_PlayerAtc) {
    return false;
  }

  G_PlayerCanAim = true;

  for (UINT I = 0; I < 3; I++) {
    std::string CompName = "bullet-icon" + std::to_string(I) + "-ui-sprite";
    G_BulletIconUsc[I] = static_cast<USpriteComponent *>(
        Aitc->getSceneNode().getComponentContainer()->getComponent(CompName));
  }

  return true;
}

void bulletManagerUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  if (getGamePauseFlg()) {
    return;
  }

  if (getPlayerAimingFlg()) {
    G_AimingRestTimer -= Timer.floatDeltaTime() / 1000.f;
    if (G_AimingRestTimer < -3.f) {
      G_AimingRestTimer = -3.f;
    }
  } else {
    G_AimingRestTimer += Timer.floatDeltaTime() / 1000.f / 3.f * 5.f;
    if (G_AimingRestTimer > 15.f) {
      G_AimingRestTimer = 15.f;
    }
  }

  if (G_AimingRestTimer <= 0.05f) {
    G_PlayerCanAim = false;
  } else {
    G_PlayerCanAim = true;
  }

  float Blt0IconAlpha = (G_AimingRestTimer - 10.f) / 5.f;
  float Blt1IconAlpha = (G_AimingRestTimer - 5.f) / 5.f;
  float Blt2IconAlpha = (G_AimingRestTimer) / 5.f;
  dx::XMFLOAT4 Offset0 = {1.f, 1.f, 1.f, Blt0IconAlpha};
  dx::XMFLOAT4 Offset1 = {1.f, 1.f, 1.f, Blt1IconAlpha};
  dx::XMFLOAT4 Offset2 = {1.f, 1.f, 1.f, Blt2IconAlpha};
  G_BulletIconUsc[0]->setOffsetColor(Offset0);
  G_BulletIconUsc[1]->setOffsetColor(Offset1);
  G_BulletIconUsc[2]->setOffsetColor(Offset2);

  float SlowRatio = (getPlayerAimingFlg() && G_PlayerCanAim) ? 0.1f : 1.f;
  float Deltatime = Timer.floatDeltaTime() * SlowRatio;
  for (auto I = G_UsingBulletAtcVec.begin(); I != G_UsingBulletAtcVec.end();) {
    auto &Blt = *I;
    auto BltAtc = Blt.first;
    auto &Direction = Blt.second;
    BltAtc->translateXAsix(Direction.x * Deltatime);
    BltAtc->translateYAsix(Direction.y * Deltatime);
    BltAtc->translateZAsix(Direction.z * Deltatime);

    dx::XMVECTOR Vec = dx::XMLoadFloat3(&BltAtc->getProcessingPosition());
    dx::XMVECTOR PlayerPos =
        dx::XMLoadFloat3(&G_PlayerAtc->getProcessingPosition());
    Vec -= PlayerPos;
    Vec = dx::XMVector3Length(Vec);
    if (dx::XMVectorGetX(Vec) > 150.f) {
      I = G_UsingBulletAtcVec.erase(I);
      G_UsableBulletAtcVec.push_back(BltAtc);
    } else {
      ++I;
    }
  }

  for (auto &Blt : G_UsableBulletAtcVec) {
    Blt->setPosition(G_PlayerAtc->getPosition());
  }
}

void bulletManagerDestory(AInteractComponent *Aitc) {
  G_UsableBulletAtcVec.clear();
  G_UsingBulletAtcVec.clear();
  G_PlayerAtc = nullptr;

  for (UINT I = 0; I < 3; I++) {
    G_BulletIconUsc[I] = nullptr;
  }
}

void setBulletShoot(dx::XMFLOAT3 &Pos, dx::XMFLOAT3 &Vec) {
  if (G_UsableBulletAtcVec.size()) {
    auto Atc = G_UsableBulletAtcVec.back();
    G_UsableBulletAtcVec.pop_back();
    Atc->setPosition(Pos);
    dx::XMVECTOR NorVec = dx::XMLoadFloat3(&Vec);
    NorVec = dx::XMVector3Normalize(NorVec);
    dx::XMFLOAT3 Vec = {0.f, 0.f, 0.f};
    dx::XMStoreFloat3(&Vec, NorVec);
    G_UsingBulletAtcVec.push_back({Atc, Vec});

    if (G_AimingRestTimer > 10.f) {
      G_AimingRestTimer = 10.f;
    } else if (G_AimingRestTimer > 5.f) {
      G_AimingRestTimer = 5.f;
    } else {
      G_AimingRestTimer = 0.f;
    }
  }
}

bool getPlayerCanAimFlg() { return G_PlayerCanAim; }

bool checkCollisionWithBullet(ACollisionComponent *Acc) {
  static std::string ObjName = "";
  for (auto &Blt : G_UsingBulletAtcVec) {
    ObjName = Blt.first->getActorOwner()->getObjectName();
    if (Acc->checkCollisionWith(ObjName)) {
      return true;
    }
  }

  return false;
}
