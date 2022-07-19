#include "SleepCrystal.h"

#include "BulletProcess.h"
#include "PlayerProcess.h"

#include <RSUtilityFunctions.h>

#include <unordered_map>

static const dx::XMFLOAT3 SCRYSTAL_BEFIRE_COLOR = {1.f, 0.f, 1.f};
static const dx::XMFLOAT3 SCRYSTAL_AFTER_COLOR = {0.f, 0.f, 1.f};
static const dx::XMFLOAT3 SCRYSTAL_SLEEP_COLOR = {1.f, 0.f, 0.f};
static std::unordered_map<AInteractComponent *, float> G_SCrystalTimerMap = {};
static std::unordered_map<AInteractComponent *, BOOL> G_SCrystalActiveMap = {};

void registerSleepCrystal(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInitMapPtr().insert(
      {FUNC_NAME(sleepCrystalInit), sleepCrystalInit});
  Factory->getAUpdateMapPtr().insert(
      {FUNC_NAME(sleepCrystalUpdate), sleepCrystalUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(sleepCrystalDestory), sleepCrystalDestory});
}

bool sleepCrystalInit(AInteractComponent *Aitc) {
  G_SCrystalTimerMap.insert({Aitc, 5.f});
  G_SCrystalActiveMap.insert({Aitc, FALSE});

  return true;
}

void sleepCrystalUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  auto Found = G_SCrystalTimerMap.find(Aitc);
  BOOL IsActive = G_SCrystalActiveMap[Aitc];
  auto Alc = Aitc->getActorOwner()->getComponent<ALightComponent>();

  if (IsActive && Found->second > 0.f) {
    float Ratio = 1.f - ((5.f - Found->second) / 5.f);
    Ratio =
        (Ratio > 0.2f) ? 1.f : rs_tool::lerp(1.f, 0.f, (0.2f - Ratio) / 0.2f);
    Found->second -= Timer.floatDeltaTime() / 1000.f;
    dx::XMFLOAT3 Color = {0.f, 0.f, 0.f};
    Color.x =
        rs_tool::lerp(SCRYSTAL_SLEEP_COLOR.x, SCRYSTAL_AFTER_COLOR.x, Ratio);
    Color.z =
        rs_tool::lerp(SCRYSTAL_SLEEP_COLOR.z, SCRYSTAL_AFTER_COLOR.z, Ratio);
    Alc->getLightInfo()->setRSLightAlbedo(Color);
    Alc->getLightInfo()->setRSLightIntensity(900.f);
    Alc->getLightInfo()->updateBloomColor();
  } else if (IsActive && Found->second <= 0.f) {
    G_SCrystalActiveMap[Aitc] = FALSE;
    Found->second = 0.f;
    Alc->getLightInfo()->setRSLightAlbedo(SCRYSTAL_SLEEP_COLOR);
    Alc->getLightInfo()->setRSLightIntensity(900.f);
    Alc->getLightInfo()->updateBloomColor();
  } else if (Found->second < 5.f) {
    float Ratio = Found->second / 5.f;
    Ratio =
        (Ratio < 0.5f) ? 0.f : rs_tool::lerp(0.f, 1.f, (Ratio - 0.5f) / 0.5f);
    Found->second += Timer.floatDeltaTime() / 1000.f;
    dx::XMFLOAT3 Color = {1.f, 0.f, 0.f};
    Color.z =
        rs_tool::lerp(SCRYSTAL_SLEEP_COLOR.z, SCRYSTAL_BEFIRE_COLOR.z, Ratio);
    Alc->getLightInfo()->setRSLightAlbedo(Color);
    Alc->getLightInfo()->setRSLightIntensity(
        rs_tool ::lerp(900.f, 600.f, Ratio));
    Alc->getLightInfo()->updateBloomColor();
  } else {
    Found->second = 5.f;
    Alc->getLightInfo()->setRSLightAlbedo(SCRYSTAL_BEFIRE_COLOR);
    Alc->getLightInfo()->setRSLightIntensity(600.f);
    Alc->getLightInfo()->updateBloomColor();
  }

  IsActive = G_SCrystalActiveMap[Aitc];
  auto Acc = Aitc->getActorOwner()->getComponent<ACollisionComponent>();
  if (!IsActive && Found->second >= 5.f) {
    if (checkCollisionWithBullet(Acc)) {
      Aitc->getActorOwner()->getComponent<AAudioComponent>()->playSe(
          "wake-crystal", 0.5f);
      G_SCrystalActiveMap[Aitc] = TRUE;
      Found->second = 5.f;
    }
  }

  if (G_SCrystalActiveMap[Aitc] && !getPlayerDashFlg() &&
      Acc->checkCollisionWith(PLAYER_NAME)) {
    Aitc->getActorOwner()->getComponent<AAudioComponent>()->playSe(
        "crystal-hit", 0.7f);
    setPlayerDashFlg(true);
    G_SCrystalActiveMap[Aitc] = FALSE;
    Found->second = 0.f;
  }
}

void sleepCrystalDestory(AInteractComponent *Aitc) {
  G_SCrystalTimerMap.erase(Aitc);
  G_SCrystalActiveMap.erase(Aitc);
}
