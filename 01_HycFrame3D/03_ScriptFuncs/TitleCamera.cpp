#include "TitleCamera.h"

void registerTitleCamera(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInitMapPtr().insert({FUNC_NAME(titleCamInit), titleCamInit});
  Factory->getAUpdateMapPtr().insert(
      {FUNC_NAME(titleCamUpdate), titleCamUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(titleCamDestory), titleCamDestory});
}

static RSCamera *G_MainCam = nullptr;

bool titleCamInit(AInteractComponent *Aitc) {
  G_MainCam = Aitc->getActorOwner()->getSceneNode().getMainCamera();
  if (!G_MainCam) {
    return false;
  }
  G_MainCam->rotateRSCamera(0.25f, 0.f);

  return true;
}

void titleCamUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  float Time = 0.05f * Timer.floatDeltaTime() / 1000.f;
  G_MainCam->rotateRSCamera(0.f, Time);
}

void titleCamDestory(AInteractComponent *Aitc) {
  G_MainCam->resetRSCameraRotation({0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
  G_MainCam = nullptr;
}
