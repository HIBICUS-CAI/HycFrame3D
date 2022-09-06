#include "Player.h"

void registerPlayer(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInputMapPtr().insert({FUNC_NAME(playerInput), playerInput});
  Factory->getAInitMapPtr().insert({FUNC_NAME(playerInit), playerInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(playerUpdate), playerUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(playerDestory), playerDestory});
}

bool createTerrain();

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
  if (!createTerrain()) {
    return false;
  }

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

bool createTerrain() {
  const float X_LENGTH = 200.f, Y_LENGTH = 200.f;
  const float X_START = -100.f, Y_START = -100.f;
  const int VERTEX_SIZE_IN_SIDE = 1000;
  std::vector<vertex_type::TangentVertex> VerticesVec = {};
  VerticesVec.reserve((size_t)VERTEX_SIZE_IN_SIDE *
                      (size_t)VERTEX_SIZE_IN_SIDE);

  for (int XIndex = 0; XIndex < VERTEX_SIZE_IN_SIDE; XIndex++) {
    for (int YIndex = 0; YIndex < VERTEX_SIZE_IN_SIDE; YIndex++) {
      using namespace dx;

      float X = X_START + X_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                              static_cast<float>(XIndex);
      float Y = Y_START + Y_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                              static_cast<float>(YIndex);
      float Z = 10.f * sinf(X) - 20.f;
      XMVECTOR PositionVec = XMVectorSet(X, Y, Z, 0);
      XMFLOAT3 Position;
      XMStoreFloat3(&Position, PositionVec);

      float X0 = X_START + X_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(XIndex + 1);
      float Y0 = Y_START + Y_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(YIndex);
      float Z0 = 10.f * sinf(X0) - 20.f;
      XMVECTOR V0 = XMVectorSet(X0, Y0, Z0, 0) - PositionVec;

      float X1 = X_START + X_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(XIndex);
      float Y1 = Y_START + Y_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(YIndex + 1);
      float Z1 = 10.f * sinf(X1) - 20.f;
      XMVECTOR V1 = XMVectorSet(X1, Y1, Z1, 0) - PositionVec;

      XMVECTOR N = XMVector3Normalize(XMVector3Cross(V0, V1));
      XMFLOAT3 Normal;
      XMStoreFloat3(&Normal, N);
      if (Normal.z < 0.f) {
        N *= -1.f;
        XMStoreFloat3(&Normal, N);
      }

      XMVECTOR T = XMVector3Normalize(XMVector3Cross(N, V0));
      XMFLOAT3 Tangent;
      XMStoreFloat3(&Tangent, T);

      XMFLOAT2 TexCoord = {500.f / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(XIndex),
                           500.f / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(YIndex)};

      vertex_type::TangentVertex TVert = {};
      TVert.Position = Position;
      TVert.Normal = Normal;
      TVert.Tangent = Tangent;
      TVert.TexCoord = TexCoord;
      VerticesVec.push_back(TVert);
    }
  }

  return true;
}
