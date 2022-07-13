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
  Factory->getAInitMapPtr().insert({FUNC_NAME(grInit), grInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(grUpdate), grUpdate});
  Factory->getADestoryMapPtr().insert({FUNC_NAME(grDestory), grDestory});
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

dx::XMFLOAT3 calcFerguson(dx::XMFLOAT3 XStart,
                          dx::XMFLOAT3 XEnd,
                          dx::XMFLOAT3 VStart,
                          dx::XMFLOAT3 VEnd,
                          float T) {
  using namespace dx;
  XMVECTOR X0 = XMLoadFloat3(&XStart);
  XMVECTOR X1 = XMLoadFloat3(&XEnd);
  XMVECTOR V0 = XMLoadFloat3(&VStart);
  XMVECTOR V1 = XMLoadFloat3(&VEnd);
  float H0 = (T - 1.f) * (T - 1.f) * (2.f * T + 1.f);
  float H1 = (1.f - T) * (1.f - T) * T;
  float H2 = (T - 1.f) * T * T;
  float H3 = T * T * (3.f - 2.f * T);

  XMFLOAT3 Result = {};
  XMStoreFloat3(&Result, X0 * H0 + V0 * H1 + V1 * H2 + X1 * H3);
  return Result;
}

static float Time = 0.f;

bool grInit(AInteractComponent *Aitc) {
  Time = 0.f;
  return true;
}

void grUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  static const int SIZE = 5;
  static const float TIMEMAX = static_cast<float>(SIZE - 1);
  static const dx::XMFLOAT3 X[SIZE] = {{-35.f, 0.f, 50.f},
                                       {0.f, 0.f, 85.f},
                                       {35.f, 0.f, 50.f},
                                       {00.f, 0.f, 15.f},
                                       {-35.f, 0.f, 50.f}};
  static const dx::XMFLOAT3 V[SIZE] = {{-30.f, 15.f, -30.f},
                                       {-30.f, -15.f, 30.f},
                                       {30.f, 15.f, 30.f},
                                       {30.f, -15.f, -30.f},
                                       {-30.f, 15.f, -30.f}};
  float Speed = Timer.floatDeltaTime() / 1500.f;

  if ((Time += Speed) >= TIMEMAX) {
    Time = 0.f;
  }
  int Index0 = static_cast<int>(Time), Index1 = Index0 + 1;
  float T = Time - static_cast<float>(Index0);

  auto Atc = Aitc->getActorOwner()->getComponent<ATransformComponent>();
  Atc->setPosition(calcFerguson(X[Index0], X[Index1], V[Index0], V[Index1], T));
}

void grDestory(AInteractComponent *Aitc) {}
