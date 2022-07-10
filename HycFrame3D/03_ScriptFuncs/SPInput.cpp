#include "SPInput.h"

#include <RSMeshHelper.h>
#include <RSPipelinesManager.h>
#include <RSRoot_DX11.h>

void registerSPInput(ObjectFactory *Factory) {
#ifdef _DEBUG
  assert(Factory);
#endif // _DEBUG
  Factory->getAInputMapPtr().insert({FUNC_NAME(testASpInput), testASpInput});
  Factory->getAInputMapPtr().insert({FUNC_NAME(tempToTitle), tempToTitle});
  Factory->getAInputMapPtr().insert({FUNC_NAME(tempToSelect), tempToSelect});
  Factory->getAInputMapPtr().insert({FUNC_NAME(tempToRun), tempToRun});
  Factory->getAInputMapPtr().insert({FUNC_NAME(tempToResult), tempToResult});
  Factory->getAInitMapPtr().insert({FUNC_NAME(testASpInit), testASpInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(testASpUpdate), testASpUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(testASpDestory), testASpDestory});
  Factory->getUInputMapPtr().insert({FUNC_NAME(testUSpInput), testUSpInput});
  Factory->getUInputMapPtr().insert(
      {FUNC_NAME(testUSpBtnInput), testUSpBtnInput});
  Factory->getUInitMapPtr().insert({FUNC_NAME(testUSpInit), testUSpInit});
  Factory->getUUpdateMapPtr().insert({FUNC_NAME(testUSpUpdate), testUSpUpdate});
  Factory->getUDestoryMapPtr().insert(
      {FUNC_NAME(testUSpDestory), testUSpDestory});

  Factory->getAInitMapPtr().insert({FUNC_NAME(aniInit), aniInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(aniUpdate), aniUpdate});
  Factory->getADestoryMapPtr().insert({FUNC_NAME(aniDestory), aniDestory});
  Factory->getAInitMapPtr().insert({FUNC_NAME(bbInit), bbInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(bbUpdate), bbUpdate});
  Factory->getADestoryMapPtr().insert({FUNC_NAME(bbDestory), bbDestory});
}

void testASpInput(AInputComponent *Aic, const Timer &Timer) {
  if (input::isKeyDownInSingle(M_LEFTBTN)) {
    auto MouseOffset = input::getMouseOffset();
    float HoriR = -MouseOffset.x * Timer.floatDeltaTime() / 800.f;
    Aic->getSceneNode().getMainCamera()->rotateRSCamera(0.f, HoriR);
  }

  if (input::isKeyPushedInSingle(KB_RETURN)) {
    P_LOG(LOG_DEBUG, "to test2");
    Aic->getSceneNode().getSceneManager()->loadSceneNode("sample2-scene",
                                                         "sample2-scene.json");
  }

  if (input::isKeyDownInSingle(KB_W)) {
    Aic->getActorObject("sp-point-light-actor")
        ->getComponent<ATransformComponent>()
        ->translateZAsix(0.1f * Timer.floatDeltaTime());
  }
  if (input::isKeyDownInSingle(KB_A)) {
    Aic->getActorObject("sp-point-light-actor")
        ->getComponent<ATransformComponent>()
        ->translateXAsix(-0.1f * Timer.floatDeltaTime());
  }
  if (input::isKeyDownInSingle(KB_S)) {
    Aic->getActorObject("sp-point-light-actor")
        ->getComponent<ATransformComponent>()
        ->translateZAsix(-0.1f * Timer.floatDeltaTime());
  }
  if (input::isKeyDownInSingle(KB_D)) {
    Aic->getActorObject("sp-point-light-actor")
        ->getComponent<ATransformComponent>()
        ->translateXAsix(0.1f * Timer.floatDeltaTime());
  }
  if (input::isKeyPushedInSingle(KB_P)) {
    static bool Simp = true;
    if (Simp) {
      getRSDX11RootInstance()->getPipelinesManager()->setPipeline(
          "light-pipeline");
    } else {
      getRSDX11RootInstance()->getPipelinesManager()->setPipeline(
          "simple-pipeline");
    }
    Simp = !Simp;
  }
}

bool testASpInit(AInteractComponent *Aitc) {
  P_LOG(LOG_DEBUG, "a sp init");

  Aitc->getActorOwner()->getComponent<ATimerComponent>()->startTimer("timer1");

  return true;
}
void testASpUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  CONTACT_PONT_PAIR Contact = {};
  if (Aitc->getActorObject("sp-point-light-actor")
          ->getComponent<ACollisionComponent>()
          ->checkCollisionWith("sp-actor", &Contact)) {
    Aitc->getActorObject("sp-point-light-actor")
        ->getComponent<ATransformComponent>()
        ->rollBackPosition();
    P_LOG(LOG_DEBUG, "a : {:+f}, {:+f}, {:+f} ; b : {:+f}, {:+f}, {:+f}",
          Contact.first.x, Contact.first.y, Contact.first.z, Contact.second.x,
          Contact.second.y, Contact.second.z);
    auto Center = ACollisionComponent::calcCenterOfContact(Contact);
    P_LOG(LOG_DEBUG, "center of contact : {:+f}, {:+f}, {:+f}", Center.x,
          Center.y, Center.z);
  }

  Aitc->getActorObject("sp-particle-actor")
      ->getComponent<ATransformComponent>()
      ->setPosition(Aitc->getActorObject("sp-point-light-actor")
                        ->getComponent<ATransformComponent>()
                        ->getProcessingPosition());
  Aitc->getActorObject("sp-particle-actor")
      ->getComponent<ATransformComponent>()
      ->translateYAsix(5.f);
}

void testASpDestory(AInteractComponent *Aitc) {
  P_LOG(LOG_DEBUG, "a sp destory");
}

void testUSpInput(UInputComponent *Uic, const Timer &Timer) {
  float Delta = Timer.floatDeltaTime();
  auto Utc = Uic->getUiOwner()->getComponent<UTransformComponent>();

  if (input::isKeyDownInSingle(KB_W)) {
    Utc->translateYAsix(0.1f * Delta);
  }
  if (input::isKeyDownInSingle(KB_A)) {
    Utc->translateXAsix(-0.1f * Delta);
  }
  if (input::isKeyDownInSingle(KB_S)) {
    Utc->translateYAsix(-0.1f * Delta);
  }
  if (input::isKeyDownInSingle(KB_D)) {
    Utc->translateXAsix(0.1f * Delta);
  }

  if (input::isKeyPushedInSingle(KB_Z)) {
    Uic->getUiOwner()->getComponent<USpriteComponent>()->resetTexture();
  }
  if (input::isKeyPushedInSingle(KB_X)) {
    Uic->getUiOwner()->getComponent<UAnimateComponent>()->changeAnimateTo(
        "number");
  }
  if (input::isKeyPushedInSingle(KB_C)) {
    Uic->getUiOwner()->getComponent<UAnimateComponent>()->changeAnimateTo(
        "runman");
  }

  if (input::isKeyPushedInSingle(KB_N)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playBgm("test", 0.8f);
  }
  if (input::isKeyPushedInSingle(KB_M)) {
    Uic->getUiOwner()->getComponent<UAudioComponent>()->playBgm("test", 0.4f);
  }

  if (input::isKeyPushedInSingle(KB_RETURN)) {
    P_LOG(LOG_DEBUG, "to test1");
    Uic->getSceneNode().getSceneManager()->loadSceneNode("sample1-scene",
                                                         "sample1-scene.json");
  }
}

void testUSpBtnInput(UInputComponent *Uic, const Timer &Timer) {
  auto Ubc = Uic->getUiOwner()->getComponent<UButtonComponent>();
  if (!Ubc) {
    return;
  }

  if (input::isKeyPushedInSingle(KB_UP)) {
    Ubc->selectUpBtn();
  }
  if (input::isKeyPushedInSingle(KB_LEFT)) {
    Ubc->selectLeftBtn();
  }
  if (input::isKeyPushedInSingle(KB_DOWN)) {
    Ubc->selectDownBtn();
  }
  if (input::isKeyPushedInSingle(KB_RIGHT)) {
    Ubc->selectRightBtn();
  }

  if (Ubc->isCursorOnBtn() && input::isKeyPushedInSingle(M_LEFTBTN)) {
    P_LOG(LOG_DEBUG, "this btn has been click : {}", Ubc->getCompName());
  }
}

bool testUSpInit(UInteractComponent *Uitc) {
  P_LOG(LOG_DEBUG, "u sp init");
  return true;
}

void testUSpUpdate(UInteractComponent *Uitc, const Timer &Timer) {}

void testUSpDestory(UInteractComponent *_uitc) {
  P_LOG(LOG_DEBUG, "u sp destory");
}

void tempToTitle(AInputComponent *Aic, const Timer &Timer) {
  if (input::isKeyPushedInSingle(KB_RCONTROL)) {
    P_LOG(LOG_DEBUG, "to title");
    Aic->getSceneNode().getSceneManager()->loadSceneNode("title-scene",
                                                         "title-scene.json");
  }
}

void tempToSelect(AInputComponent *Aic, const Timer &Timer) {
  if (input::isKeyPushedInSingle(KB_RCONTROL)) {
    P_LOG(LOG_DEBUG, "to select");
    Aic->getSceneNode().getSceneManager()->loadSceneNode("select-scene",
                                                         "select-scene.json");
  }
}

void tempToRun(AInputComponent *Aic, const Timer &Timer) {
  if (input::isKeyPushedInSingle(KB_RCONTROL)) {
    P_LOG(LOG_DEBUG, "to run");
    Aic->getSceneNode().getSceneManager()->loadSceneNode("run-scene",
                                                         "run-scene.json");
  }
}

void tempToResult(AInputComponent *Aic, const Timer &Timer) {
  if (input::isKeyPushedInSingle(KB_RCONTROL)) {
    P_LOG(LOG_DEBUG, "to result");
    Aic->getSceneNode().getSceneManager()->loadSceneNode("result-scene",
                                                         "result-scene.json");
  }
}

static float G_AniSpdFactor = 50.f;
static AAnimateComponent *G_Aanc = nullptr;

bool aniInit(AInteractComponent *Aitc) {
  P_LOG(LOG_DEBUG, "animate init");

  G_Aanc = Aitc->getActorOwner()->getComponent<AAnimateComponent>();
  if (!G_Aanc) {
    return false;
  }

  return true;
}

void aniUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  Aitc->getActorOwner()->getComponent<ATransformComponent>()->rotateYAsix(
      Timer.floatDeltaTime() / 1000.f);

  if (input::isKeyPushedInSingle(KB_1)) {
    G_Aanc->changeAnimationTo("run");
  } else if (input::isKeyPushedInSingle(KB_2)) {
    G_Aanc->changeAnimationTo("bite");
  } else if (input::isKeyPushedInSingle(KB_3)) {
    G_Aanc->changeAnimationTo("roar");
  } else if (input::isKeyPushedInSingle(KB_4)) {
    G_Aanc->changeAnimationTo("attack_tail");
  } else if (input::isKeyPushedInSingle(KB_5)) {
    G_Aanc->changeAnimationTo("idle");
  } else if (input::isKeyPushedInSingle(KB_UP)) {
    G_AniSpdFactor += 10.f;
    G_Aanc->SetSpeedFactor(G_AniSpdFactor);
  } else if (input::isKeyPushedInSingle(KB_DOWN)) {
    G_AniSpdFactor -= 10.f;
    G_Aanc->SetSpeedFactor(G_AniSpdFactor);
  }
}

void aniDestory(AInteractComponent *Aitc) {
  P_LOG(LOG_DEBUG, "animate destory");
  G_Aanc = nullptr;
}

static ATransformComponent *G_BBAtc = nullptr;
static float G_XFactor = 0.f;

bool bbInit(AInteractComponent *Aitc) {
  G_XFactor = -1.f;
  G_BBAtc = Aitc->getActorOwner()->getComponent<ATransformComponent>();
  if (!G_BBAtc) {
    return false;
  }

  return true;
}

void bbUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  G_BBAtc->translateXAsix(Timer.floatDeltaTime() * G_XFactor * 0.01f);
  if (fabsf(G_BBAtc->getProcessingPosition().x) > 18.f) {
    G_BBAtc->rollBackPositionX();
    G_XFactor *= -1.f;
  }
}

void bbDestory(AInteractComponent *Aitc) { G_BBAtc = nullptr; }
