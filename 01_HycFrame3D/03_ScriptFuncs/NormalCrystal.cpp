#include "NormalCrystal.h"

#include "PlayerProcess.h"

#include <RSUtilityFunctions.h>

#include <unordered_map>

static std::unordered_map<AInteractComponent *, float> G_NCrystalTimerMap = {};

void registerNormalCrystal(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInitMapPtr().insert(
      {FUNC_NAME(normalCrystalInit), normalCrystalInit});
  Factory->getAUpdateMapPtr().insert(
      {FUNC_NAME(normalCrystalUpdate), normalCrystalUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(normalCrystalDestory), normalCrystalDestory});
}

bool normalCrystalInit(AInteractComponent *Aitc) {
  G_NCrystalTimerMap.insert({Aitc, 5.f});

  return true;
}

void normalCrystalUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  auto Acc = Aitc->getActorOwner()->getComponent<ACollisionComponent>();
  auto Found = G_NCrystalTimerMap.find(Aitc);
  bool Active = Found->second >= 5.f;

  if (!getPlayerDashFlg() && Active && Acc->checkCollisionWith(PLAYER_NAME)) {
    Aitc->getActorOwner()->getComponent<AAudioComponent>()->playSe(
        "crystal-hit", 0.7f);
    setPlayerDashFlg(true);
    Found->second = 0.f;
  }

  float LightRatio = Found->second / 5.f;
  if (Active) {
    LightRatio = 1.f;
  } else {
    LightRatio = (LightRatio < 0.5f)
                     ? 0.f
                     : rs_tool::lerp(0.f, 1.f, (LightRatio - 0.5f) / 0.5f);
  }
  auto Alc = Aitc->getActorOwner()->getComponent<ALightComponent>();
  dx::XMFLOAT3 Color = {};
  Color.x = rs_tool::lerp(1.f, 0.f, LightRatio);
  Color.y = rs_tool::lerp(0.f, 1.f, LightRatio);
  Alc->getLightInfo()->setRSLightAlbedo(Color);
  Alc->getLightInfo()->setRSLightIntensity(
      rs_tool::lerp(800.f, 300.f, LightRatio));
  Alc->getLightInfo()->updateBloomColor();

  Found->second += Timer.floatDeltaTime() / 1000.f;
  if (Found->second >= 5.f) {
    Found->second = 5.f;
  }
}

void normalCrystalDestory(AInteractComponent *Aitc) {
  G_NCrystalTimerMap.erase(Aitc);
}
