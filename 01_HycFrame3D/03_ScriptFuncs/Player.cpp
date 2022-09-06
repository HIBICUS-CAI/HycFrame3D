#include "Player.h"

#include <RSMeshHelper.h>
#include <RSRoot_DX11.h>
#include <RSStaticResources.h>

void registerPlayer(ObjectFactory *Factory) {
  assert(Factory);
  Factory->getAInputMapPtr().insert({FUNC_NAME(playerInput), playerInput});
  Factory->getAInitMapPtr().insert({FUNC_NAME(playerInit), playerInit});
  Factory->getAUpdateMapPtr().insert({FUNC_NAME(playerUpdate), playerUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(playerDestory), playerDestory});
}

bool createTerrain(SceneNode *ScenePtr);

static ATransformComponent *G_PlayerAtc = nullptr;
static RSCamera *G_Cam = nullptr;
static const float X_LENGTH = 200.f, Z_LENGTH = 200.f;
static const float X_START = -100.f, Z_START = -100.f;
static const int VERTEX_SIZE_IN_SIDE = 1000;
static std::vector<float> G_TerrainHeight = {};

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

  auto &ProcessingPos = G_PlayerAtc->getProcessingPosition();
  if (fabsf(ProcessingPos.x) + 5.f >
          X_LENGTH *
              Aic->getActorObject("terrain-actor")
                  ->getComponent<ATransformComponent>()
                  ->getScaling()
                  .x /
              2.f ||
      fabsf(ProcessingPos.z) + 5.f >
          Z_LENGTH *
              Aic->getActorObject("terrain-actor")
                  ->getComponent<ATransformComponent>()
                  ->getScaling()
                  .z /
              2.f) {
    G_PlayerAtc->rollBackPosition();
  }
}

bool playerInit(AInteractComponent *Aitc) {
  if (!createTerrain(&Aitc->getSceneNode())) {
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
  float ProcessingX = G_PlayerAtc->getProcessingPosition().x /
                      Aitc->getActorObject("terrain-actor")
                          ->getComponent<ATransformComponent>()
                          ->getScaling()
                          .x;
  float ProcessingZ = G_PlayerAtc->getProcessingPosition().z /
                      Aitc->getActorObject("terrain-actor")
                          ->getComponent<ATransformComponent>()
                          ->getScaling()
                          .z;
  float ProcessingY = G_PlayerAtc->getProcessingPosition().y;
  UINT XIndex =
      static_cast<UINT>((ProcessingX - X_START) *
                        static_cast<float>(VERTEX_SIZE_IN_SIDE) / X_LENGTH);
  UINT ZIndex =
      static_cast<UINT>((ProcessingZ - Z_START) *
                        static_cast<float>(VERTEX_SIZE_IN_SIDE) / Z_LENGTH);
  UINT HeightIndex = XIndex * VERTEX_SIZE_IN_SIDE + ZIndex;
  float DeltaY = G_TerrainHeight[HeightIndex] - ProcessingY;
  G_PlayerAtc->translateYAsix(DeltaY);
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
  Now.y += 15.f;
  Now.z += newZ;
  G_Cam->changeRSCameraPosition(Now);
}

void playerDestory(AInteractComponent *Aitc) {
  G_PlayerAtc = nullptr;
  G_Cam = nullptr;
}

bool createTerrain(SceneNode *ScenePtr) {
  assert(ScenePtr);
  std::vector<vertex_type::TangentVertex> VerticesVec = {};
  std::vector<UINT> IndicesVec = {};
  G_TerrainHeight.clear();
  G_TerrainHeight.reserve((size_t)VERTEX_SIZE_IN_SIDE *
                          (size_t)VERTEX_SIZE_IN_SIDE);
  VerticesVec.reserve((size_t)VERTEX_SIZE_IN_SIDE *
                      (size_t)VERTEX_SIZE_IN_SIDE);
  IndicesVec.reserve(6 * (size_t)VERTEX_SIZE_IN_SIDE *
                     (size_t)VERTEX_SIZE_IN_SIDE);

  for (int XIndex = 0; XIndex < VERTEX_SIZE_IN_SIDE; XIndex++) {
    for (int ZIndex = 0; ZIndex < VERTEX_SIZE_IN_SIDE; ZIndex++) {
      using namespace dx;

      float X = X_START + X_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                              static_cast<float>(XIndex);
      float Z = Z_START + Z_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                              static_cast<float>(ZIndex);
      float Y = 10.f * sinf(X / 10.f);
      XMVECTOR PositionVec = XMVectorSet(X, Y, Z, 0);
      XMFLOAT3 Position;
      XMStoreFloat3(&Position, PositionVec);

      float X0 = X_START + X_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(XIndex + 1);
      float Z0 = Z_START + Z_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(ZIndex);
      float Y0 = 10.f * sinf(X0 / 10.f);
      XMVECTOR V0 = XMVectorSet(X0, Y0, Z0, 0) - PositionVec;

      float X1 = X_START + X_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(XIndex);
      float Z1 = Z_START + Z_LENGTH / static_cast<float>(VERTEX_SIZE_IN_SIDE) *
                               static_cast<float>(ZIndex + 1);
      float Y1 = 10.f * sinf(X1 / 10.f);
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
                               static_cast<float>(ZIndex)};

      vertex_type::TangentVertex TVert = {};
      TVert.Position = Position;
      TVert.Normal = Normal;
      TVert.Tangent = Tangent;
      TVert.TexCoord = TexCoord;
      VerticesVec.push_back(TVert);
      G_TerrainHeight.push_back(Position.y);
    }
  }

  for (int I = 0; I < VERTEX_SIZE_IN_SIDE - 1; I++) {
    for (int J = 0; J < VERTEX_SIZE_IN_SIDE - 1; J++) {
      IndicesVec.push_back(I * VERTEX_SIZE_IN_SIDE + J);
      IndicesVec.push_back(I * VERTEX_SIZE_IN_SIDE + J + 1);
      IndicesVec.push_back((I + 1) * VERTEX_SIZE_IN_SIDE + J);

      IndicesVec.push_back((I + 1) * VERTEX_SIZE_IN_SIDE + J);
      IndicesVec.push_back(I * VERTEX_SIZE_IN_SIDE + J + 1);
      IndicesVec.push_back((I + 1) * VERTEX_SIZE_IN_SIDE + J + 1);
    }
  }

  MATERIAL_INFO TerrainMatInfo = {};
  SUBMESH_INFO TerrainInfo = {};
  std::vector<std::string> Textures = {"ground_diffuse.jpg"};
  TerrainMatInfo.InterpolateFactor = 0.f;
  TerrainMatInfo.MajorMaterialID =
      getRSDX11RootInstance()->getStaticResources()->getStaticMaterialIndex(
          "rough-soil");
  TerrainInfo.AnimationFlag = false;
  TerrainInfo.IndeicesPtr = &IndicesVec;
  TerrainInfo.MaterialPtr = &TerrainMatInfo;
  TerrainInfo.TexturesPtr = &Textures;
  TerrainInfo.TopologyType = TOPOLOGY_TYPE::TRIANGLELIST;
  TerrainInfo.VerteicesPtr = &VerticesVec;

  RS_SUBMESH_DATA TerrainMeshData = {};
  getRSDX11RootInstance()->getMeshHelper()->processSubMesh(
      &TerrainMeshData, &TerrainInfo, LAYOUT_TYPE::NORMAL_TANGENT_TEX);
  addTextureToSubMesh(&TerrainMeshData, "ground_normal.jpg",
                      MESH_TEXTURE_TYPE::NORMAL);
  addTextureToSubMesh(&TerrainMeshData, "ground_rough.jpg",
                      MESH_TEXTURE_TYPE::ROUGHNESS);
  ScenePtr->getAssetsPool()->insertNewIndexedMesh("terrain", TerrainMeshData,
                                                  MESH_TYPE::OPACITY, 0);

  return true;
}
