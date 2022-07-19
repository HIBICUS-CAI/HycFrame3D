#include "MiscProcess.h"

void registerMiscProcess(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInitMapPtr().insert({FUNC_NAME(dragonInit), dragonInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(dragonUpdate), dragonUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(dragonDestory), dragonDestory});
  Factory->getUInitMapPtr().insert({FUNC_NAME(hillInfoInit), hillInfoInit});
  Factory->getUUpdateMapPtr().insert(
      {FUNC_NAME(hillInfoUpdate), hillInfoUpdate});
  Factory->getUDestoryMapPtr().insert(
      {FUNC_NAME(hillInfoDestory), hillInfoDestory});
  Factory->getUInitMapPtr().insert({FUNC_NAME(resultInit), resultInit});
  Factory->getUUpdateMapPtr().insert({FUNC_NAME(resultUpdate), resultUpdate});
  Factory->getUDestoryMapPtr().insert(
      {FUNC_NAME(resultDestory), resultDestory});
  Factory->getUInitMapPtr().insert({FUNC_NAME(logoFadeInit), logoFadeInit});
  Factory->getUUpdateMapPtr().insert(
      {FUNC_NAME(logoFadeUpdate), logoFadeUpdate});
  Factory->getUDestoryMapPtr().insert(
      {FUNC_NAME(logoFadeDestory), logoFadeDestory});
}

static ATransformComponent *G_DragonAtc = nullptr;

bool dragonInit(AInteractComponent *Aitc) {
  G_DragonAtc = Aitc->getActorOwner()->getComponent<ATransformComponent>();
  if (!G_DragonAtc) {
    return false;
  }
  return true;
}

void dragonUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  G_DragonAtc->rotateYAsix(Timer.floatDeltaTime() / 8000.f);
}

void dragonDestory(AInteractComponent *Aitc) { G_DragonAtc = nullptr; }

static UButtonComponent *G_TutorialUbc = nullptr;
static UButtonComponent *G_Route1Ubc = nullptr;
static UButtonComponent *G_Route2Ubc = nullptr;
static USpriteComponent *G_TutorialInfoUsc = nullptr;
static USpriteComponent *G_Route1InfoUsc = nullptr;
static USpriteComponent *G_Route2InfoUsc = nullptr;

bool hillInfoInit(UInteractComponent *Uitc) {
  G_TutorialUbc = nullptr;
  G_Route1Ubc = nullptr;
  G_Route2Ubc = nullptr;
  G_TutorialInfoUsc = nullptr;
  G_Route1InfoUsc = nullptr;
  G_Route2InfoUsc = nullptr;
  return true;
}

void hillInfoUpdate(UInteractComponent *Uitc, const Timer &Timer) {
  if (!G_TutorialUbc) {
    G_TutorialUbc =
        Uitc->getUiObject("tutorial-btn-ui")->getComponent<UButtonComponent>();
  }
  if (!G_Route1Ubc) {
    G_Route1Ubc =
        Uitc->getUiObject("route1-btn-ui")->getComponent<UButtonComponent>();
  }
  if (!G_Route2Ubc) {
    G_Route2Ubc =
        Uitc->getUiObject("route2-btn-ui")->getComponent<UButtonComponent>();
  }
  if (!G_TutorialInfoUsc) {
    G_TutorialInfoUsc =
        Uitc->getUiObject("text-tutorial-ui")->getComponent<USpriteComponent>();
  }
  if (!G_Route1InfoUsc) {
    G_Route1InfoUsc =
        Uitc->getUiObject("text-route1-ui")->getComponent<USpriteComponent>();
  }
  if (!G_Route2InfoUsc) {
    G_Route2InfoUsc =
        Uitc->getUiObject("text-route2-ui")->getComponent<USpriteComponent>();
  }

  G_TutorialInfoUsc->setOffsetColor({1.f, 1.f, 1.f, 0.f});
  G_Route1InfoUsc->setOffsetColor({1.f, 1.f, 1.f, 0.f});
  G_Route2InfoUsc->setOffsetColor({1.f, 1.f, 1.f, 0.f});
  if (G_TutorialUbc->isBeingSelected()) {
    G_TutorialInfoUsc->setOffsetColor({1.f, 1.f, 1.f, 1.f});
  }
  if (G_Route1Ubc->isBeingSelected()) {
    G_Route1InfoUsc->setOffsetColor({1.f, 1.f, 1.f, 1.f});
  }
  if (G_Route2Ubc->isBeingSelected()) {
    G_Route2InfoUsc->setOffsetColor({1.f, 1.f, 1.f, 1.f});
  }
}

void hillInfoDestory(UInteractComponent *_uitc) {
  G_TutorialUbc = nullptr;
  G_Route1Ubc = nullptr;
  G_Route2Ubc = nullptr;
  G_TutorialInfoUsc = nullptr;
  G_Route1InfoUsc = nullptr;
  G_Route2InfoUsc = nullptr;
}

bool resultInit(UInteractComponent *Uitc) {
  Uitc->getUiOwner()->getComponent<UAnimateComponent>()->changeAnimateTo(
      "success");
  return true;
}

void resultUpdate(UInteractComponent *Uitc, const Timer &Timer) {}

void resultDestory(UInteractComponent *Uitc) {}

static USpriteComponent *G_LogoFadeUsc = nullptr;
static float G_LogoTimer = 0.f;

bool logoFadeInit(UInteractComponent *Uitc) {
  G_LogoTimer = 0.f;
  G_LogoFadeUsc = Uitc->getUiOwner()->getComponent<USpriteComponent>();
  if (!G_LogoFadeUsc) {
    return false;
  }
  return true;
}

void logoFadeUpdate(UInteractComponent *Uitc, const Timer &Timer) {
  if (G_LogoTimer < 1000.f) {
    G_LogoFadeUsc->setOffsetColor(
        {1.f, 1.f, 1.f, 1.f - (G_LogoTimer / 1000.f)});
  } else if (G_LogoTimer > 2500.f && G_LogoTimer < 3500.f) {
    G_LogoFadeUsc->setOffsetColor(
        {1.f, 1.f, 1.f, (G_LogoTimer - 2500.f) / 1000.f});
  } else if (G_LogoTimer > 3500.f) {
    Uitc->getSceneNode().getSceneManager()->loadSceneNode("title-scene",
                                                          "title-scene.json");
    stopBGM();
    setVolume("title", 0.2f);
    playBGM("title");
  }

  G_LogoTimer += Timer.floatDeltaTime();
}

void logoFadeDestory(UInteractComponent *Uitc) {
  G_LogoTimer = 0.f;
  G_LogoFadeUsc = nullptr;
}
