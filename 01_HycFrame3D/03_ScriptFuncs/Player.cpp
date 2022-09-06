#include "Player.h"

void registerPlayer(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInputMapPtr().insert({FUNC_NAME(playerInput), playerInput});
  Factory->getAInitMapPtr().insert({FUNC_NAME(playerInit), playerInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(playerUpdate), playerUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(playerDestory), playerDestory});
}

static ATransformComponent *G_PlayerAtc = nullptr;
static RSCamera *G_Cam = nullptr;

void playerInput(AInputComponent *Aic, const Timer &Timer) {
  float DeltaTime = Timer.floatDeltaTime();
  Aic->getActorOwner()->getComponent<AAnimateComponent>()->changeAnimationTo(
      "idle");

  if (input::isKeyDownInSingle(KB_ESCAPE)) {
    PostQuitMessage(0);
  }

  if (input::isKeyDownInSingle(KB_Q)) {
    G_PlayerAtc->rotateYAsix(-0.005f * DeltaTime);
  }
  if (input::isKeyDownInSingle(KB_E)) {
    G_PlayerAtc->rotateYAsix(0.005f * DeltaTime);
  }
  float Angle = G_PlayerAtc->getProcessingRotation().y - 3.14f;
  if (input::isKeyDownInSingle(KB_A)) {
    G_PlayerAtc->translateXAsix(-0.2f * DeltaTime * cosf(Angle));
    G_PlayerAtc->translateZAsix(0.2f * DeltaTime * sinf(Angle));
    Aic->getActorOwner()->getComponent<AAnimateComponent>()->changeAnimationTo(
        "run");
  }
  if (input::isKeyDownInSingle(KB_D)) {
    G_PlayerAtc->translateXAsix(0.2f * DeltaTime * cosf(Angle));
    G_PlayerAtc->translateZAsix(-0.2f * DeltaTime * sinf(Angle));
    Aic->getActorOwner()->getComponent<AAnimateComponent>()->changeAnimationTo(
        "run");
  }
  if (input::isKeyDownInSingle(KB_W)) {
    G_PlayerAtc->translateXAsix(0.2f * DeltaTime * sinf(Angle));
    G_PlayerAtc->translateZAsix(0.2f * DeltaTime * cosf(Angle));
    Aic->getActorOwner()->getComponent<AAnimateComponent>()->changeAnimationTo(
        "run");
  }
  if (input::isKeyDownInSingle(KB_S)) {
    G_PlayerAtc->translateXAsix(-0.2f * DeltaTime * sinf(Angle));
    G_PlayerAtc->translateZAsix(-0.2f * DeltaTime * cosf(Angle));
    Aic->getActorOwner()->getComponent<AAnimateComponent>()->changeAnimationTo(
        "run");
  }
}

bool playerInit(AInteractComponent *Aitc) {
  G_PlayerAtc = Aitc->getActorOwner()->getComponent<ATransformComponent>();
  if (!G_PlayerAtc) {
    return false;
  }
  G_Cam = Aitc->getSceneNode().getMainCamera();
  if (!G_Cam) {
    return false;
  }
  G_Cam->changeRSCameraPosition({0.f, 0.f, 0.f});
  G_Cam->resetRSCameraRotation({0.f, 0.f, 1.f}, {0.f, 1.f, 0.f});
  return true;
}

void playerUpdate(AInteractComponent *Aitc, const Timer &Timer) {
  using namespace dx;
  XMVECTOR Processing = dx::XMLoadFloat3(&G_PlayerAtc->getProcessingPosition());
  XMVECTOR Origin = DirectX::XMLoadFloat3(&G_PlayerAtc->getPosition());
  XMFLOAT3 DeltaPos = {};
  XMStoreFloat3(&DeltaPos, Origin - Processing);
  DeltaPos.z *= -1.f;
  G_Cam->translateRSCamera(DeltaPos);

  float Theta = G_PlayerAtc->getProcessingRotation().y - 3.14f;
  G_Cam->resetRSCameraRotation({sinf(Theta), 0.f, cosf(Theta)},
                               {0.f, 1.f, 0.f});
  float NewX = -50.f * sinf(Theta), newZ = -50.f * cosf(Theta);
  dx::XMFLOAT3 Now = G_PlayerAtc->getProcessingPosition();
  Now.x += NewX;
  Now.y = 0.f;
  Now.z += newZ;
  G_Cam->changeRSCameraPosition(Now);
}

void playerDestory(AInteractComponent *Aitc) {
  G_PlayerAtc = nullptr;
  G_Cam = nullptr;
}
