#define BT_NO_SIMD_OPERATOR_OVERLOADS

#include "PlayerProcess.h"

#include "BulletProcess.h"
#include "FadeProcess.h"
#include "PauseMenu.h"

#include <TomlUtility.h>
#include <WM_Interface.h>

#include <vector>

using namespace dx;

void registerPlayerProcess(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInputMapPtr().insert({FUNC_NAME(playerInput), playerInput});
  Factory->getAInitMapPtr().insert({FUNC_NAME(playerInit), playerInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(playerUpdate), playerUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(playerDestory), playerDestory});
}

static dx::XMFLOAT3 G_PlayerMoveVec = {0.f, 0.f, 0.f};

static float G_PlayerYAsixSpeed = 0.f;
static bool G_PlayerCanJumpFlg = false;
static std::vector<std::string> G_GroundObjNameVec = {};

static ATransformComponent *G_PlayerAngleAtc = nullptr;

static bool G_PlayerCanDashFlg = false;
static bool G_PlayerIsDashing = false;
static float G_DashTimer = 0.f;

static ATransformComponent *G_PlayerGunAtc = nullptr;
static float G_GunYAngleOffset = dx::XM_PIDIV4;
static bool G_PlayerIsAming = false;
static bool G_PlayerCanShoot = false;

static ATransformComponent *G_LastReachGroundAtc = nullptr;

static bool G_ResetDeadPlayerToGround = false;

struct MOVE_SPEED_CONFIG {
  const float SpeedFactor = 1.f;
  MOVE_SPEED_CONFIG() {
    using namespace hyc::text;
    std::string ErrorMessage = "";
    TomlNode RootNode = {};
    if (!loadTomlAndParse(RootNode,
                          ".\\Assets\\Configs\\game-control-config.toml",
                          ErrorMessage)) {
      assert(false && "check error message in source code");
      exit(-1);
    } else {
      float &SpdFactorRef = const_cast<float &>(SpeedFactor);
      SpdFactorRef = getAs<float>(RootNode["input"]["camera-speed-factor"]);
    }
  }
};

static MOVE_SPEED_CONFIG G_SpeedFactorConfig = {};

void playerInput(AInputComponent *Aic, const Timer &Timer) {
  if (getGamePauseFlg() || getSceneInFlg()) {
    return;
  }

  float Deltatime = Timer.floatDeltaTime();

  auto MouseOffset = input::getMouseOffset();
  if (!MouseOffset.x && !MouseOffset.y) {
    MouseOffset = input::rightStickOffset();
    MouseOffset.x /= 200;
    MouseOffset.y /= 200;
  }
  float HoriR = MouseOffset.x * Deltatime / 2000.f;
  float VertR = -MouseOffset.y * Deltatime / 2000.f;
  HoriR *= G_SpeedFactorConfig.SpeedFactor;
  VertR *= G_SpeedFactorConfig.SpeedFactor;

  G_PlayerAngleAtc->rotate({VertR, HoriR, 0.f});
  if (fabsf(G_PlayerAngleAtc->getProcessingRotation().x) > XM_PIDIV2) {
    G_PlayerAngleAtc->rollBackRotationX();
    VertR = 0.f;
  }
  Aic->getActorOwner()->getSceneNode().getMainCamera()->rotateRSCamera(VertR,
                                                                       HoriR);
  auto Atc = Aic->getActorOwner()->getComponent<ATransformComponent>();
  Atc->rotateYAsix(HoriR);

  static const dx::XMFLOAT3 IDENT = {0.f, 0.f, 1.f};
  static const dx::XMVECTOR IDENT_VEC = dx::XMLoadFloat3(&IDENT);
  static dx::XMFLOAT3 LookAt = {0.f, 0.f, 0.f};

  if (input::isKeyDownInSingle(M_RIGHTBTN) ||
      input::isKeyDownInSingle(GP_LEFTBACKSHDBTN)) {
    G_PlayerIsAming = true;
    if (getPlayerCanAimFlg()) {
      G_GunYAngleOffset -= Deltatime / 150.f;
    } else {
      G_GunYAngleOffset += Deltatime / 150.f;
    }
  } else {
    G_GunYAngleOffset += Deltatime / 150.f;
    G_PlayerIsAming = false;
  }
  if (G_GunYAngleOffset >= dx::XM_PIDIV4) {
    G_GunYAngleOffset = dx::XM_PIDIV4;
    G_PlayerCanShoot = false;
  } else if (G_GunYAngleOffset <= 0.f) {
    G_GunYAngleOffset = 0.f;
    G_PlayerCanShoot = true;
  } else {
    G_PlayerCanShoot = false;
  }
  float AimSlow = (G_PlayerIsAming && getPlayerCanAimFlg()) ? 0.1f : 1.f;
  Deltatime *= AimSlow;

  dx::XMFLOAT3 Angle = G_PlayerAngleAtc->getProcessingRotation();
  Angle.x *= -1.f;
  Angle.y += G_GunYAngleOffset;
  G_PlayerGunAtc->setRotation(Angle);

  if (G_PlayerCanDashFlg && (input::isKeyPushedInSingle(KB_LALT) ||
                             input::isKeyPushedInSingle(GP_RIGHTFORESHDBTN))) {
    Aic->getActorOwner()->getComponent<AAudioComponent>()->playSe("dash", 0.7f);
    G_PlayerCanDashFlg = false;
    G_PlayerIsDashing = true;
    G_PlayerCanJumpFlg = false;
    G_DashTimer = 0.f;
    G_PlayerYAsixSpeed = 0.f;
    LookAt = G_PlayerAngleAtc->getProcessingRotation();
    dx::XMMATRIX Matrix =
        dx::XMMatrixRotationX(LookAt.x) * dx::XMMatrixRotationY(LookAt.y);
    dx::XMVECTOR LookAtVec = {};
    LookAtVec = dx::XMVector3TransformNormal(IDENT_VEC, Matrix);
    LookAtVec = dx::XMVector3Normalize(LookAtVec);
    dx::XMStoreFloat3(&LookAt, LookAtVec);
  }

  if (G_PlayerCanShoot && (input::isKeyPushedInSingle(M_LEFTBTN) ||
                           input::isKeyPushedInSingle(GP_RIGHTBACKSHDBTN))) {
    Aic->getActorOwner()->getComponent<AAudioComponent>()->playSe("shoot",
                                                                  0.3f);
    LookAt = G_PlayerAngleAtc->getProcessingRotation();
    dx::XMMATRIX Matrix =
        dx::XMMatrixRotationX(LookAt.x) * dx::XMMatrixRotationY(LookAt.y);
    dx::XMVECTOR LookAtVec = {};
    LookAtVec = dx::XMVector3TransformNormal(IDENT_VEC, Matrix);
    LookAtVec = dx::XMVector3Normalize(LookAtVec);
    dx::XMStoreFloat3(&LookAt, LookAtVec);
    dx::XMFLOAT3 ShootPos = Atc->getProcessingPosition();
    dx::XMFLOAT3 ShootAt = LookAt;
    ShootAt.y *= -1.f;
    ShootPos.x += ShootAt.x * 2.f;
    ShootPos.y += ShootAt.y * 2.f;
    ShootPos.z += ShootAt.z * 2.f;
    setBulletShoot(ShootPos, ShootAt);
  }

  if (!G_PlayerIsDashing) {
    float AngleY = Atc->getProcessingRotation().y;
    dx::XMVECTOR FrontVec =
        dx::XMVector3TransformNormal(IDENT_VEC, dx::XMMatrixRotationY(AngleY));
    dx::XMVECTOR RightVec = dx::XMVector3TransformNormal(
        IDENT_VEC, dx::XMMatrixRotationY(AngleY + dx::XM_PIDIV2));
    dx::XMVECTOR MoveVec = dx::XMVectorZero();

    if (input::isKeyDownInSingle(KB_W)) {
      MoveVec += FrontVec;
    }
    if (input::isKeyDownInSingle(KB_A)) {
      MoveVec -= RightVec;
    }
    if (input::isKeyDownInSingle(KB_S)) {
      MoveVec -= FrontVec;
    }
    if (input::isKeyDownInSingle(KB_D)) {
      MoveVec += RightVec;
    }
    auto LeftStick = input::leftStickOffset();
    if (LeftStick.x > 0) {
      MoveVec += RightVec;
    }
    if (LeftStick.x < 0) {
      MoveVec -= RightVec;
    }
    if (LeftStick.y > 0) {
      MoveVec -= FrontVec;
    }
    if (LeftStick.y < 0) {
      MoveVec += FrontVec;
    }
    MoveVec = dx::XMVector3Normalize(MoveVec);
    dx::XMStoreFloat3(&G_PlayerMoveVec, MoveVec);
    Atc->translateZAsix(0.02f * Deltatime * G_PlayerMoveVec.z);
    Atc->translateXAsix(0.02f * Deltatime * G_PlayerMoveVec.x);

    if (G_PlayerCanJumpFlg && (input::isKeyPushedInSingle(KB_SPACE) ||
                               input::isKeyPushedInSingle(GP_LEFTFORESHDBTN) ||
                               input::isKeyPushedInSingle(GP_BOTTOMBTN))) {
      G_PlayerCanJumpFlg = false;
      G_PlayerYAsixSpeed = 50.f;
    }

    G_PlayerYAsixSpeed -= 120.f * (Deltatime / 1000.f);
    Atc->translateYAsix(G_PlayerYAsixSpeed * (Deltatime / 1000.f));
  } else {
    Atc->translateZAsix(0.5f * Deltatime * LookAt.z);
    Atc->translateYAsix(0.5f * Deltatime * -LookAt.y);
    Atc->translateXAsix(0.5f * Deltatime * LookAt.x);
  }
}

bool playerInit(AInteractComponent *Aitc) {
  using namespace hyc::str;
  G_PlayerCanJumpFlg = false;

  Aitc->getSceneNode().getMainCamera()->rotateRSCamera(
      0.f, Aitc->getActorOwner()
               ->getComponent<ATransformComponent>()
               ->getRotation()
               .y);

  G_PlayerYAsixSpeed = 0.f;

  G_GroundObjNameVec.clear();
  int GroundSize = 1;
  for (int I = 0; I < GroundSize; I++) {
    G_GroundObjNameVec.push_back(sFormat("ground-{}-actor", I));
  }

  ShowCursor(FALSE);

  G_PlayerAngleAtc = static_cast<ATransformComponent *>(
      Aitc->getSceneNode().getComponentContainer()->getComponent(
          "player-angle-actor-transform"));

  G_PlayerGunAtc = static_cast<ATransformComponent *>(
      Aitc->getSceneNode().getComponentContainer()->getComponent(
          "player-gun-actor-transform"));
  G_GunYAngleOffset = dx::XM_PIDIV4;
  G_PlayerIsAming = false;
  G_PlayerCanShoot = false;

  G_PlayerCanDashFlg = true;
  G_PlayerIsDashing = false;
  G_DashTimer = 0.f;

  G_LastReachGroundAtc = nullptr;

  return true;
}

void playerUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  if (getGamePauseFlg()) {
    return;
  }

  auto Atc = Aitc->getActorOwner()->getComponent<ATransformComponent>();
  dx::XMFLOAT3 CamOffset = Atc->getProcessingPosition();
  Aitc->getSceneNode().getMainCamera()->changeRSCameraPosition(CamOffset);

  G_PlayerGunAtc->setPosition(Atc->getProcessingPosition());

  float AimSlow = (G_PlayerIsAming && getPlayerCanAimFlg()) ? 0.1f : 1.f;

  if (G_PlayerIsDashing) {
    G_DashTimer += Timer.floatDeltaTime() * AimSlow;
    if (G_DashTimer > 200.f) {
      G_PlayerIsDashing = false;
      G_DashTimer = 0.f;
    }
  }

  if (Atc->getProcessingPosition().y < -50.f) {
    setDeadFadeRunningFlg(true);
  }

  if (G_ResetDeadPlayerToGround) {
    if (!G_LastReachGroundAtc) {
      G_LastReachGroundAtc = Aitc->getActorObject("ground-0-actor")
                                 ->getComponent<ATransformComponent>();
    }
    dx::XMFLOAT3 Rebirth = G_LastReachGroundAtc->getPosition();
    Rebirth.y += 10.f;
    Atc->setPosition(Rebirth);
    G_PlayerYAsixSpeed = 0.f;
    G_PlayerCanJumpFlg = false;
    G_GunYAngleOffset = dx::XM_PIDIV4;
    G_PlayerIsAming = false;
    G_PlayerCanShoot = false;
    G_PlayerCanDashFlg = true;
    G_PlayerIsDashing = false;
    G_DashTimer = 0.f;
    G_ResetDeadPlayerToGround = false;
  }
}

void playerDestory(AInteractComponent *Aitc) {
  Aitc->getActorOwner()->getSceneNode().getMainCamera()->resetRSCameraRotation(
      {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
  ShowCursor(TRUE);
  RECT WndRect = {};
  GetClientRect(window::getWindowPtr()->getWndHandle(), &WndRect);
  SetCursorPos((WndRect.right - WndRect.left) / 2,
               (WndRect.bottom - WndRect.top) / 2);
  G_PlayerAngleAtc = nullptr;
  G_LastReachGroundAtc = nullptr;
}

void setPlayerDashFlg(bool CanDashFlag) { G_PlayerCanDashFlg = CanDashFlag; }

bool getPlayerDashFlg() { return G_PlayerCanDashFlg; }

bool getPlayerIsDashingFlg() { return G_PlayerIsDashing; }

bool getPlayerAimingFlg() { return G_PlayerIsAming; }

void setPlayerContactGround() {
  G_PlayerYAsixSpeed = 0.f;
  G_PlayerCanJumpFlg = true;
  G_PlayerCanDashFlg = true;
  G_PlayerIsDashing = false;
  G_DashTimer = 0.f;
}

void setPlayerBrokeHead() {
  G_PlayerYAsixSpeed = 0.f;
  G_PlayerIsDashing = false;
  G_DashTimer = 0.f;
}

void setPlayerDashToObstacle() {
  G_PlayerYAsixSpeed = 0.f;
  G_PlayerIsDashing = false;
  G_DashTimer = 0.f;
}

const dx::XMFLOAT3 &getPlayerMoveDirection() { return G_PlayerMoveVec; }

void setPlayerMoveDirection(const dx::XMFLOAT3 &Direction) {
  G_PlayerMoveVec = Direction;
}

void setPlayerLastReachGround(ATransformComponent *GroundAtc) {
  G_LastReachGroundAtc = GroundAtc;
}

void resetDeadPlayerToGround() { G_ResetDeadPlayerToGround = true; }
