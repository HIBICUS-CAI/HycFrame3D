#include "FadeProcess.h"

#include "PlayerProcess.h"

void registerFadeProcess(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getUInitMapPtr().insert({FUNC_NAME(deadFadeInit), deadFadeInit});
  Factory->getUUpdateMapPtr().insert(
      {FUNC_NAME(deadFadeUpdate), deadFadeUpdate});
  Factory->getUDestoryMapPtr().insert(
      {FUNC_NAME(deadFadeDestory), deadFadeDestory});
  Factory->getUInitMapPtr().insert({FUNC_NAME(sceneFadeInit), sceneFadeInit});
  Factory->getUUpdateMapPtr().insert(
      {FUNC_NAME(sceneFadeUpdate), sceneFadeUpdate});
  Factory->getUDestoryMapPtr().insert(
      {FUNC_NAME(sceneFadeDestory), sceneFadeDestory});
}

static UTransformComponent *G_DeadFadesUtc[10] = {nullptr};

static bool G_DeadFadeRun = false;
static bool G_DeadFadeDestDown = true;

bool deadFadeInit(UInteractComponent *Uitc) {
  using namespace hyc::str;
  auto CompContainer =
      Uitc->getUiOwner()->getSceneNode().getComponentContainer();
  for (UINT I = 0; I < ARRAYSIZE(G_DeadFadesUtc); I++) {
    G_DeadFadesUtc[I] = static_cast<UTransformComponent *>(
        CompContainer->getComponent(sFormat("dead-black-{}-ui-transform", I)));
    if (!G_DeadFadesUtc[I]) {
      return false;
    }
  }

  G_DeadFadeRun = false;

  return true;
}

void deadFadeUpdate(UInteractComponent *Uitc, const Timer &Timer) {
  static float RunTime = 0.f;
  if (G_DeadFadeRun) {
    float Dest = G_DeadFadeDestDown ? 1.f : -1.f;
    if (RunTime >= 0.f && RunTime < 500.f) {
      for (UINT I = 0; I < 10; I++) {
        float StartTime = static_cast<float>(I * 25);
        float OffsetTime = RunTime - StartTime;
        if (OffsetTime >= 0.f && OffsetTime <= 250.f) {
          float OffsetY =
              720.f - (720.f * sinf(OffsetTime / 250.f * dx::XM_PIDIV2));
          dx::XMFLOAT3 Pos = G_DeadFadesUtc[I]->getPosition();
          Pos.y = OffsetY * Dest;
          G_DeadFadesUtc[I]->setPosition(Pos);
        }
      }
    } else {
      if (RunTime >= 1000.f) {
        Uitc->getUiOwner()->getComponent<UAudioComponent>()->playSe("reborn",
                                                                    0.15f);
        RunTime = 0.f;
        G_DeadFadeRun = false;
        G_DeadFadeDestDown = !G_DeadFadeDestDown;
        return;
      } else {
        for (UINT I = 0; I < 10; I++) {
          resetDeadPlayerToGround();
          float StartTime = static_cast<float>(I * 25);
          float OffsetTime = RunTime - 500.f - StartTime;
          if (OffsetTime >= 0.f && OffsetTime <= 250.f) {
            float OffsetY =
                0.f - (720.f * sinf(OffsetTime / 250.f * dx::XM_PIDIV2));
            dx::XMFLOAT3 Pos = G_DeadFadesUtc[I]->getPosition();
            Pos.y = OffsetY * Dest;
            G_DeadFadesUtc[I]->setPosition(Pos);
          }
        }
      }
    }
    RunTime += Timer.floatDeltaTime();
  }
}

void deadFadeDestory(UInteractComponent *Uitc) { G_DeadFadeRun = false; }

bool getDeadFadeRunningFlg() { return G_DeadFadeRun; }

void setDeadFadeRunningFlg(bool Flag) { G_DeadFadeRun = Flag; }

static bool G_SceneInFlg = true;
static bool G_SceneOutFlg = false;
static bool G_SceneOutTrigger = false;

static UTransformComponent *G_SceneInUtc[2] = {nullptr};
static UTransformComponent *G_SceneOutUtc[2] = {nullptr};

static float G_SceneInOutTimer = 0.f;

bool sceneFadeInit(UInteractComponent *Uitc) {
  using namespace hyc::str;
  G_SceneInFlg = true;
  G_SceneOutFlg = false;
  G_SceneOutTrigger = false;
  G_SceneInOutTimer = 0.f;

  auto CompContainer = Uitc->getSceneNode().getComponentContainer();
  for (UINT I = 0; I < 2; I++) {
    G_SceneInUtc[I] = static_cast<UTransformComponent *>(
        CompContainer->getComponent(sFormat("scene-in-{}-ui-transform", I)));
    G_SceneOutUtc[I] = static_cast<UTransformComponent *>(
        CompContainer->getComponent(sFormat("scene-out-{}-ui-transform", I)));
  }

  return true;
}

void sceneFadeUpdate(UInteractComponent *Uitc, const Timer &Timer) {
  if (G_SceneInFlg) {
    if (G_SceneInOutTimer > 500.f) {
      G_SceneInFlg = false;
      G_SceneInOutTimer = 0.f;
      return;
    }

    float SinVal = sinf(G_SceneInOutTimer / 500.f * dx::XM_PIDIV2);
    float AbsX = SinVal * 640.f + 320.f;
    dx::XMFLOAT3 Pos = {};
    Pos = G_SceneInUtc[0]->getPosition();
    Pos.x = -1.f * AbsX;
    G_SceneInUtc[0]->setPosition(Pos);
    Pos = G_SceneInUtc[1]->getPosition();
    Pos.x = 1.f * AbsX;
    G_SceneInUtc[1]->setPosition(Pos);

    G_SceneInOutTimer += Timer.floatDeltaTime();
  }

  if (G_SceneOutFlg) {
    if (G_SceneInOutTimer > 500.f) {
      G_SceneOutFlg = false;
      G_SceneInOutTimer = 0.f;
      return;
    }

    float SinVal = sinf(G_SceneInOutTimer / 500.f * dx::XM_PIDIV2);
    float AbsY = (1.f - SinVal) * 360.f + 180.f;
    dx::XMFLOAT3 Pos = {};
    Pos = G_SceneOutUtc[0]->getPosition();
    Pos.y = -1.f * AbsY;
    G_SceneOutUtc[0]->setPosition(Pos);
    Pos = G_SceneOutUtc[1]->getPosition();
    Pos.y = 1.f * AbsY;
    G_SceneOutUtc[1]->setPosition(Pos);

    G_SceneInOutTimer += Timer.floatDeltaTime();
  }
}

void sceneFadeDestory(UInteractComponent *Uitc) {
  G_SceneInFlg = true;
  G_SceneOutFlg = false;
  G_SceneOutTrigger = false;
}

bool getSceneInFlg() { return G_SceneInFlg; }

bool getSceneOutFlg() { return G_SceneOutFlg; }

static UINT G_SceneOutFilter = static_cast<UINT>(-1);

bool getSceneOutFinish(UINT Filter) {
  bool FilterFlag = (G_SceneOutFilter == static_cast<UINT>(-1))
                        ? true
                        : ((G_SceneOutFilter == Filter) ? true : false);
  return G_SceneOutTrigger && !G_SceneOutFlg && FilterFlag;
}

void setSceneOutFlg(bool Flag, UINT Filter) {
  if (G_SceneOutTrigger || G_SceneOutFlg) {
    return;
  }
  G_SceneOutFlg = Flag;
  G_SceneOutTrigger = true;
  G_SceneInOutTimer = 0.f;
  G_SceneOutFilter = Filter;
}
