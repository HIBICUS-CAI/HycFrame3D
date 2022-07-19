#include "DestProcess.h"

#include "FadeProcess.h"
#include "PlayerProcess.h"

void registerDestProcess(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInitMapPtr().insert({FUNC_NAME(destInit), destInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(destUpdate), destUpdate});
  Factory->getADestoryMapPtr().insert({FUNC_NAME(destDestory), destDestory});
  Factory->getAInitMapPtr().insert({FUNC_NAME(destPtcInit), destPtcInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(destPtcUpdate), destPtcUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(destPtcDestory), destPtcDestory});
}

static ACollisionComponent *G_DestAcc = nullptr;
static ATransformComponent *G_DestAtc = nullptr;
static ACollisionComponent *G_DestPtcAcc = nullptr;
static AParticleComponent *G_DestPtcApc = nullptr;

static const UINT DEST_REACH = 0;

bool destInit(AInteractComponent *Aitc) {
  G_DestAcc = Aitc->getActorOwner()->getComponent<ACollisionComponent>();
  if (!G_DestAcc) {
    return false;
  }

  G_DestAtc = Aitc->getActorOwner()->getComponent<ATransformComponent>();
  if (!G_DestAtc) {
    return false;
  }

  return true;
}

void destUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  G_DestAtc->rotateYAsix(Timer.floatDeltaTime() / 1500.f);

  if (G_DestAcc->checkCollisionWith(PLAYER_NAME)) {
    stopBGM();
    Aitc->getActorOwner()->getComponent<AAudioComponent>()->playSe("goal",
                                                                   0.3f);
    setSceneOutFlg(true, DEST_REACH);
  }

  if (getSceneOutFinish(DEST_REACH)) {
    P_LOG(LOG_DEBUG, "to result");
    Aitc->getSceneNode().getSceneManager()->loadSceneNode("result-scene",
                                                          "result-scene.json");
    setVolume("result", 0.2f);
    playBGM("result");
  }
}

void destDestory(AInteractComponent *Aitc) {
  G_DestAcc = nullptr;
  G_DestAtc = nullptr;
}

bool destPtcInit(AInteractComponent *Aitc) {
  G_DestPtcApc = Aitc->getActorOwner()->getComponent<AParticleComponent>();
  if (!G_DestPtcApc) {
    return false;
  }

  G_DestPtcAcc = Aitc->getActorOwner()->getComponent<ACollisionComponent>();
  if (!G_DestPtcAcc) {
    return false;
  }

  return true;
}

void destPtcUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  if (G_DestPtcAcc->checkCollisionWith(PLAYER_NAME)) {
    G_DestPtcApc->getEmitterInfo().EmitNumPerSecond = 240.f;
  } else {
    G_DestPtcApc->getEmitterInfo().EmitNumPerSecond = 80.f;
  }
}

void destPtcDestory(AInteractComponent *Aitc) {
  G_DestAcc = nullptr;
  G_DestPtcAcc = nullptr;
  G_DestPtcApc = nullptr;
}
