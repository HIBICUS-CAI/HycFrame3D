#include "ObjectFactory.h"

#include "00_FunctionRegister.h"
#include "ActorAll.h"
#include "ModelHelper.h"
#include "SoundHelper.h"
#include "UiAll.h"

#include <FormatUtility.h>
#include <RSMeshHelper.h>
#include <RSRoot_DX11.h>
#include <RSStaticResources.h>
#include <TextUtility.h>

using namespace hyc::text;

ObjectFactory::ObjectFactory()
    : SceneManagerPtr(nullptr), ActorInputFuncPtrMap({}),
      ActorInteractInitFuncPtrMap({}), ActorInteractUpdateFuncPtrMap({}),
      ActorInteractDestoryFuncPtrMap({}), UiInputFuncPtrMap({}),
      UiInteractInitFuncPtrMap({}), UiInteractUpdateFuncPtrMap({}),
      UiInteractDestoryFuncPtrMap({}) {}

ObjectFactory::~ObjectFactory() {}

bool ObjectFactory::startUp(SceneManager *SceneManager) {
  if (!SceneManager) {
    P_LOG(LOG_ERROR, "invalid scene manager pointer");
    return false;
  }

  SceneManagerPtr = SceneManager;

  registerAllFuncPtr(this);

  return true;
}

void ObjectFactory::cleanAndStop() {}

SceneNode *ObjectFactory::createSceneNode(const std::string &Name,
                                          const std::string &Path) {
  SceneNode *NewNode = new SceneNode(Name, SceneManagerPtr);
  if (!NewNode) {
    P_LOG(LOG_ERROR, "failed to alloc a scene node memory name : {} , {}", Name,
          Path);
    return nullptr;
  }

  JsonFile SceneConfig = {};
  if (!loadJsonAndParse(SceneConfig, Path)) {
    P_LOG(LOG_ERROR,
          "failed to parse scene config name {} with error code : {}", Path,
          getJsonParseError(SceneConfig));
    delete NewNode;
    return nullptr;
  }

  {
    std::string SceneName = SceneConfig["scene-name"].GetString();
    if (SceneName != Name) {
      P_LOG(LOG_ERROR, "the scene's name in json file {} \
                doesn't pair with what has passed",
            Path);
      delete NewNode;
      return nullptr;
    }
  }

  if (SceneConfig["ambient-factor"].IsNull() ||
      SceneConfig["ambient-factor"].Size() != 4) {
    P_LOG(LOG_ERROR, "the scene's name in json file {} \
                doesn't has an ambient light data",
          Path);
    delete NewNode;
    return nullptr;
  }
  dx::XMFLOAT4 Ambient = {SceneConfig["ambient-factor"][0].GetFloat(),
                          SceneConfig["ambient-factor"][1].GetFloat(),
                          SceneConfig["ambient-factor"][2].GetFloat(),
                          SceneConfig["ambient-factor"][3].GetFloat()};
  NewNode->setCurrentAmbientFactor(Ambient);

  {
    std::string IblEnvTexName = "";
    std::string IblDiffTexName = "";
    std::string IblSpecTexName = "";

    JsonNode IblEnvNode = getJsonNode(SceneConfig, "/ibl-environment");
    JsonNode IblDiffNode = getJsonNode(SceneConfig, "/ibl-diffuse");
    JsonNode IblSpecNode = getJsonNode(SceneConfig, "/ibl-specular");

    if (IblEnvNode) {
      IblEnvTexName = IblEnvNode->GetString();
    }
    if (IblDiffNode) {
      IblDiffTexName = IblDiffNode->GetString();
    }
    if (IblSpecNode) {
      IblSpecTexName = IblSpecNode->GetString();
    }

    NewNode->loadIBLTexture(IblEnvTexName, IblDiffTexName, IblSpecTexName);
  }

  createSceneAssets(NewNode, SceneConfig);

  if (SceneConfig.HasMember("actor") && !SceneConfig["actor"].IsNull()) {
    for (unsigned int I = 0, E = SceneConfig["actor"].Size(); I < E; I++) {
      createActorObject(NewNode, SceneConfig,
                        "/actor/" + hyc::str::toString(I));
    }
  }
  if (SceneConfig.HasMember("ui") && !SceneConfig["ui"].IsNull()) {
    for (unsigned int I = 0, E = SceneConfig["ui"].Size(); I < E; I++) {
      createUiObject(NewNode, SceneConfig, "/ui/" + hyc::str::toString(I));
    }
  }

  return NewNode;
}

void ObjectFactory::createSceneAssets(SceneNode *Scene, JsonFile &Json) {
  JsonNode ModelRoot = getJsonNode(Json, "/model-assets");
  std::string JsonPath = "";
  if (ModelRoot && ModelRoot->Size()) {
    UINT ModelSize = ModelRoot->Size();
    std::string MeshName = "";
    RS_MATERIAL_INFO MatInfo = {};
    std::string ForceDiffuse = "";
    std::string ForceNormal = "";
    std::string ForceMetal = "";
    std::string ForceRough = "";
    std::string ForceEmiss = "";
    std::string LoadMode = "";
    RS_SUBMESH_DATA MeshData = {};
    static SUBMESH_BONES BonesData = {};
    BonesData.clear();
    MESH_ANIMATION_DATA *AnimationData = nullptr;
    for (UINT I = 0; I < ModelSize; I++) {
      MeshName = "";
      int SubIndex = 0;
      MatInfo = {};
      ForceDiffuse = "";
      ForceNormal = "";
      ForceMetal = "";
      ForceRough = "";
      ForceEmiss = "";
      LoadMode = "";
      MeshData = {};

      JsonPath = "/model-assets/" + hyc::str::toString(I);

      MeshName = getJsonNode(Json, JsonPath + "/mesh-name")->GetString();

      JsonNode MatInfoNode = getJsonNode(Json, JsonPath + "/material-info");
      if (MatInfoNode && !MatInfoNode->IsNull()) {
        JsonNode MajorNode =
            getJsonNode(Json, JsonPath + "/material-info/major-material");
        JsonNode MinorNode =
            getJsonNode(Json, JsonPath + "/material-info/minor-material");
        JsonNode FactorNode =
            getJsonNode(Json, JsonPath + "/material-info/interpolate-factor");
        auto StaticResPtr = getRSDX11RootInstance()->getStaticResources();

        assert(StaticResPtr && MajorNode && MinorNode && FactorNode);

        std::string MajName = MajorNode->GetString();
        std::string MinName = MinorNode->GetString();
        MatInfo.MajorMaterialID = StaticResPtr->getStaticMaterialIndex(MajName);
        MatInfo.MinorMaterialID = StaticResPtr->getStaticMaterialIndex(MinName);
        MatInfo.InterpolateFactor = FactorNode->GetFloat();
      } else {
        P_LOG(LOG_ERROR, "mesh {} doesnt have material info", MeshName);
      }

      JsonNode DiffuseNode = getJsonNode(Json, JsonPath + "/force-diffuse");
      if (DiffuseNode && !DiffuseNode->IsNull()) {
        ForceDiffuse = DiffuseNode->GetString();
      }

      JsonNode NormalNode = getJsonNode(Json, JsonPath + "/force-normal");
      if (NormalNode && !NormalNode->IsNull()) {
        ForceNormal = NormalNode->GetString();
      }

      JsonNode MetalNode = getJsonNode(Json, JsonPath + "/force-metallic");
      if (MetalNode && !MetalNode->IsNull()) {
        ForceMetal = MetalNode->GetString();
      }

      JsonNode RoughNode = getJsonNode(Json, JsonPath + "/force-roughness");
      if (RoughNode && !RoughNode->IsNull()) {
        ForceRough = RoughNode->GetString();
      }

      JsonNode EmissNode = getJsonNode(Json, JsonPath + "/force-emissive");
      if (EmissNode && !EmissNode->IsNull()) {
        ForceEmiss = EmissNode->GetString();
      }

      LoadMode = getJsonNode(Json, JsonPath + "/load-mode")->GetString();

      if (LoadMode == "model-file") {
        std::string FileName =
            getJsonNode(Json, JsonPath + "/load-info/m-file")->GetString();
        std::string FileType =
            getJsonNode(Json, JsonPath + "/load-info/m-file-type")->GetString();
        auto SubIndexNode =
            getJsonNode(Json, JsonPath + "/load-info/m-sub-mesh-index");
        if (SubIndexNode && !SubIndexNode->IsNull()) {
          SubIndex = SubIndexNode->GetInt();
        }
        MODEL_FILE_TYPE Type = MODEL_FILE_TYPE::BIN;
        if (FileType == "binary") {
          Type = MODEL_FILE_TYPE::BIN;
        } else if (FileType == "json") {
          Type = MODEL_FILE_TYPE::JSON;
        } else {
          P_LOG(LOG_ERROR, "invlaid model file type : {}", FileType);
          return;
        }
        loadModelFile(FileName, Type, SubIndex, &MeshData, &BonesData,
                      &AnimationData);
      } else if (LoadMode == "program-box") {
        float Width =
            getJsonNode(Json, JsonPath + "/load-info/b-size/0")->GetFloat();
        float Height =
            getJsonNode(Json, JsonPath + "/load-info/b-size/1")->GetFloat();
        float Depth =
            getJsonNode(Json, JsonPath + "/load-info/b-size/2")->GetFloat();
        UINT Divide =
            getJsonNode(Json, JsonPath + "/load-info/b-divide")->GetUint();
        MeshData = getRSDX11RootInstance()
                       ->getMeshHelper()
                       ->getGeoGenerator()
                       ->createBox(
                           Width, Height, Depth, Divide,
                           LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                           getJsonNode(Json, JsonPath + "/load-info/b-tex-file")
                               ->GetString());
      } else if (LoadMode == "program-sphere") {
        float Radius =
            getJsonNode(Json, JsonPath + "/load-info/s-radius")->GetFloat();
        UINT Slice =
            getJsonNode(Json, JsonPath + "/load-info/s-slice-stack-count/0")
                ->GetUint();
        UINT Stack =
            getJsonNode(Json, JsonPath + "/load-info/s-slice-stack-count/1")
                ->GetUint();
        MeshData = getRSDX11RootInstance()
                       ->getMeshHelper()
                       ->getGeoGenerator()
                       ->createSphere(
                           Radius, Slice, Stack,
                           LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                           getJsonNode(Json, JsonPath + "/load-info/s-tex-file")
                               ->GetString());
      } else if (LoadMode == "program-geo-sphere") {
        float Radius =
            getJsonNode(Json, JsonPath + "/load-info/gs-radius")->GetFloat();
        UINT Divide =
            getJsonNode(Json, JsonPath + "/load-info/gs-divide")->GetUint();
        MeshData =
            getRSDX11RootInstance()
                ->getMeshHelper()
                ->getGeoGenerator()
                ->createGeometrySphere(
                    Radius, Divide, LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                    getJsonNode(Json, JsonPath + "/load-info/gs-tex-file")
                        ->GetString());
      } else if (LoadMode == "program-cylinder") {
        float TopRadius =
            getJsonNode(Json, JsonPath + "/load-info/c-top-btm-het-size/0")
                ->GetFloat();
        float BottomRadius =
            getJsonNode(Json, JsonPath + "/load-info/c-top-btm-het-size/1")
                ->GetFloat();
        float Height =
            getJsonNode(Json, JsonPath + "/load-info/c-top-btm-het-size/2")
                ->GetFloat();
        UINT Slice =
            getJsonNode(Json, JsonPath + "/load-info/c-slice-stack-count/0")
                ->GetUint();
        UINT Stack =
            getJsonNode(Json, JsonPath + "/load-info/c-slice-stack-count/1")
                ->GetUint();
        MeshData = getRSDX11RootInstance()
                       ->getMeshHelper()
                       ->getGeoGenerator()
                       ->createCylinder(
                           BottomRadius, TopRadius, Height, Slice, Stack,
                           LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                           getJsonNode(Json, JsonPath + "/load-info/c-tex-file")
                               ->GetString());
      } else if (LoadMode == "program-grid") {
        float Width =
            getJsonNode(Json, JsonPath + "/load-info/g-size/0")->GetFloat();
        float Depth =
            getJsonNode(Json, JsonPath + "/load-info/g-size/1")->GetFloat();
        UINT Row = getJsonNode(Json, JsonPath + "/load-info/g-row-col-count/0")
                       ->GetUint();
        UINT Col = getJsonNode(Json, JsonPath + "/load-info/g-row-col-count/1")
                       ->GetUint();
        MeshData = getRSDX11RootInstance()
                       ->getMeshHelper()
                       ->getGeoGenerator()
                       ->createGrid(
                           Width, Depth, Row, Col,
                           LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
                           getJsonNode(Json, JsonPath + "/load-info/g-tex-file")
                               ->GetString());
      } else {
        P_LOG(LOG_ERROR, "invlaid model load mode : {}", LoadMode);
        return;
      }

      MeshData.Material = MatInfo;

      if (ForceDiffuse != "") {
        addTextureToSubMesh(&MeshData, ForceDiffuse, MESH_TEXTURE_TYPE::ALBEDO);
      }
      if (ForceNormal != "") {
        addTextureToSubMesh(&MeshData, ForceNormal, MESH_TEXTURE_TYPE::NORMAL);
      }
      if (ForceMetal != "") {
        addTextureToSubMesh(&MeshData, ForceMetal, MESH_TEXTURE_TYPE::METALLIC);
      }
      if (ForceRough != "") {
        addTextureToSubMesh(&MeshData, ForceRough,
                            MESH_TEXTURE_TYPE::ROUGHNESS);
      }
      if (ForceEmiss != "") {
        addTextureToSubMesh(&MeshData, ForceEmiss, MESH_TEXTURE_TYPE::EMISSIVE);
      }

      if (!MeshData.Textures.size()) {
        P_LOG(LOG_ERROR, "invlaid model without diffuse : {}", MeshName);
        getRSDX11RootInstance()->getMeshHelper()->releaseSubMesh(MeshData);
        return;
      }

      Scene->getAssetsPool()->insertNewIndexedMesh(MeshName, MeshData,
                                                   MESH_TYPE::OPACITY, SubIndex,
                                                   &BonesData, AnimationData);
    }
  }

  JsonNode AudioRoot = getJsonNode(Json, "/audio-assets");
  if (AudioRoot && AudioRoot->Size()) {
    for (UINT I = 0, E = AudioRoot->Size(); I < E; I++) {
      JsonNode Audio = getJsonNode(
          Json, "/audio-assets/" + hyc::str::toString(I) + "/audio-name");
      std::string Name = Audio->GetString();
      Audio = getJsonNode(Json, "/audio-assets/" + hyc::str::toString(I) +
                                    "/audio-file");
      std::string File = Audio->GetString();
      loadSound(Name, File);
      Scene->getAssetsPool()->insertNewSound(Name);
    }
  }
}

void ObjectFactory::createActorObject(SceneNode *Scene,
                                      JsonFile &Json,
                                      const std::string &JsonPath) {
  JsonNode ActorRoot = getJsonNode(Json, JsonPath);
  if (!ActorRoot) {
    P_LOG(LOG_ERROR, "failed to get actor root node : {}", JsonPath);
    return;
  }

  std::string ActorName = "";
  {
    JsonNode ActorNameNode = getJsonNode(Json, JsonPath + "/actor-name");
    if (ActorNameNode && ActorNameNode->IsString()) {
      ActorName = ActorNameNode->GetString();
    } else {
      P_LOG(LOG_ERROR, "invalid actor name in : {}", JsonPath);
      return;
    }
  }

  ActorObject Actor(ActorName, *Scene);

  JsonNode CompsRoot = getJsonNode(Json, JsonPath + "/components");
  if (CompsRoot) {
    for (UINT I = 0, E = CompsRoot->Size(); I < E; I++) {
      std::string CompPath = JsonPath + "/components/" + hyc::str::toString(I);
      createActorComp(Scene, &Actor, Json, CompPath);
    }
  }

  Scene->addActorObject(Actor);
}

void ObjectFactory::createUiObject(SceneNode *Scene,
                                   JsonFile &Json,
                                   const std::string &JsonPath) {
  JsonNode UiRoot = getJsonNode(Json, JsonPath);
  if (!UiRoot) {
    P_LOG(LOG_ERROR, "failed to get ui root node : {}", JsonPath);
    return;
  }

  std::string UiName = "";
  {
    JsonNode UiNameNode = getJsonNode(Json, JsonPath + "/ui-name");
    if (UiNameNode && UiNameNode->IsString()) {
      UiName = UiNameNode->GetString();
    } else {
      P_LOG(LOG_ERROR, "invalid ui name in : {}", JsonPath);
      return;
    }
  }

  UiObject Ui(UiName, *Scene);

  JsonNode CompsRoot = getJsonNode(Json, JsonPath + "/components");
  if (CompsRoot) {
    for (UINT I = 0, E = CompsRoot->Size(); I < E; I++) {
      std::string CompPath = JsonPath + "/components/" + hyc::str::toString(I);
      createUiComp(Scene, &Ui, Json, CompPath);
    }
  }

  Scene->addUiObject(Ui);
}

void ObjectFactory::createActorComp(SceneNode *Scene,
                                    ActorObject *Actor,
                                    JsonFile &Json,
                                    const std::string &JsonPath) {
  std::string CompType = getJsonNode(Json, JsonPath + "/type")->GetString();
  std::string CompName = Actor->getObjectName() + "-" + CompType;

  if (CompType == "transform") {
    ATransformComponent Atc(CompName, nullptr);

    float Pos[3] = {0.f};
    float Ang[3] = {0.f};
    float Sca[3] = {0.f};
    for (UINT I = 0, E = ARRAYSIZE(Pos); I < E; I++) {
      Pos[I] = getJsonNode(Json, JsonPath + "/atc-init-position/" +
                                     hyc::str::toString(I))
                   ->GetFloat();
      Ang[I] = getJsonNode(Json, JsonPath + "/atc-init-angle/" +
                                     hyc::str::toString(I))
                   ->GetFloat();
      Sca[I] = getJsonNode(Json, JsonPath + "/atc-init-scale/" +
                                     hyc::str::toString(I))
                   ->GetFloat();
    }
    Atc.forcePosition({Pos[0], Pos[1], Pos[2]});
    Atc.forceRotation({Ang[0], Ang[1], Ang[2]});
    Atc.forceScaling({Sca[0], Sca[1], Sca[2]});

    COMP_TYPE Type = COMP_TYPE::A_TRANSFORM;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Atc);
  } else if (CompType == "input") {
    AInputComponent Aic(CompName, nullptr);

    std::string InputFuncName =
        getJsonNode(Json, JsonPath + "/aic-func-name")->GetString();
    auto Found = ActorInputFuncPtrMap.find(InputFuncName);
    if (Found == ActorInputFuncPtrMap.end()) {
      P_LOG(LOG_ERROR, "invlaid input func name : {}", InputFuncName);
      return;
    }
    Aic.setInputFunction(Found->second);

    COMP_TYPE Type = COMP_TYPE::A_INPUT;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Aic);
  } else if (CompType == "interact") {
    AInteractComponent Aitc(CompName, nullptr);

    std::string InitFuncName =
        getJsonNode(Json, JsonPath + "/aitc-init-func-name")->GetString();
    std::string UpdateFuncName =
        getJsonNode(Json, JsonPath + "/aitc-update-func-name")->GetString();
    std::string DestoryFuncName =
        getJsonNode(Json, JsonPath + "/aitc-destory-func-name")->GetString();
    auto FoundInit = ActorInteractInitFuncPtrMap.find(InitFuncName);
    if (FoundInit == ActorInteractInitFuncPtrMap.end()) {
      P_LOG(LOG_ERROR, "invlaid init func name : {}", InitFuncName);
      return;
    }
    Aitc.setInitFunction(FoundInit->second);
    auto FoundUpdate = ActorInteractUpdateFuncPtrMap.find(UpdateFuncName);
    if (FoundUpdate == ActorInteractUpdateFuncPtrMap.end()) {
      P_LOG(LOG_ERROR, "invlaid update func name : {}", UpdateFuncName);
      return;
    }
    Aitc.setUpdateFunction(FoundUpdate->second);
    auto FoundDestory = ActorInteractDestoryFuncPtrMap.find(DestoryFuncName);
    if (FoundDestory == ActorInteractDestoryFuncPtrMap.end()) {
      P_LOG(LOG_ERROR, "invlaid destory func name : {}", DestoryFuncName);
      return;
    }
    Aitc.setDestoryFunction(FoundDestory->second);

    COMP_TYPE Type = COMP_TYPE::A_INTERACT;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Aitc);
  } else if (CompType == "timer") {
    ATimerComponent Atmc(CompName, nullptr);

    UINT TimerSize = getJsonNode(Json, JsonPath + "/atmc-timers")->Size();
    for (UINT I = 0; I < TimerSize; I++) {
      std::string TimerName =
          getJsonNode(Json, JsonPath + "/atmc-timers/" + hyc::str::toString(I))
              ->GetString();
      Atmc.addTimer(TimerName);
    }

    COMP_TYPE Type = COMP_TYPE::A_TIMER;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Atmc);
  } else if (CompType == "collision") {
    ACollisionComponent Acc(CompName, nullptr);

    std::string ShapeType =
        getJsonNode(Json, JsonPath + "/acc-collision-shape")->GetString();
    UINT ValueSize =
        getJsonNode(Json, JsonPath + "/acc-collision-size")->Size();
    float Value[3] = {0.f};
    for (UINT I = 0; I < ValueSize; I++) {
      Value[I] = getJsonNode(Json, JsonPath + "/acc-collision-size/" +
                                       hyc::str::toString(I))
                     ->GetFloat();
    }
    COLLISION_SHAPE Shape = COLLISION_SHAPE::SIZE;
    if (ShapeType == "sphere") {
      Shape = COLLISION_SHAPE::SPHERE;
    } else if (ShapeType == "box") {
      Shape = COLLISION_SHAPE::BOX;
    } else {
      P_LOG(LOG_ERROR, "invlaid collision shape name : {}", ShapeType);
      return;
    }
    Acc.createCollisionShape(Shape, {Value[0], Value[1], Value[2]});

    COMP_TYPE Type = COMP_TYPE::A_COLLISION;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Acc);
  } else if (CompType == "mesh") {
    AMeshComponent Amc(CompName, nullptr);

    UINT MeshSize = getJsonNode(Json, JsonPath + "/amc-meshes")->Size();
    for (UINT I = 0; I < MeshSize; I++) {
      std::string MeshName =
          getJsonNode(Json, JsonPath + "/amc-meshes/" + hyc::str::toString(I) +
                                "/mesh-name")
              ->GetString();
      JsonNode OffsetNode =
          getJsonNode(Json, JsonPath + "/amc-meshes/" + hyc::str::toString(I) +
                                "/mesh-offset");
      dx::XMFLOAT3 Offset = {0.f, 0.f, 0.f};
      if (OffsetNode && !OffsetNode->IsNull()) {
        Offset = {
            getJsonNode(Json, JsonPath + "/amc-meshes/" +
                                  hyc::str::toString(I) + "/mesh-offset/0")
                ->GetFloat(),
            getJsonNode(Json, JsonPath + "/amc-meshes/" +
                                  hyc::str::toString(I) + "/mesh-offset/1")
                ->GetFloat(),
            getJsonNode(Json, JsonPath + "/amc-meshes/" +
                                  hyc::str::toString(I) + "/mesh-offset/2")
                ->GetFloat()};
      }
      Amc.addMeshInfo(MeshName, Offset);
    }

    JsonNode IntensityNode =
        getJsonNode(Json, JsonPath + "/amc-emissive-intensity");
    if (IntensityNode && IntensityNode->IsFloat()) {
      Amc.setEmissiveIntensity(GetAs<float>(IntensityNode));
    }

    COMP_TYPE Type = COMP_TYPE::A_MESH;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Amc);
  } else if (CompType == "light") {
    ALightComponent Alc(CompName, nullptr);

    std::string LightTypeStr =
        getJsonNode(Json, JsonPath + "/alc-light-type")->GetString();
    LIGHT_TYPE LightType = LIGHT_TYPE::POINT;
    if (LightTypeStr == "direct") {
      LightType = LIGHT_TYPE::DIRECT;
    } else if (LightTypeStr == "point") {
      LightType = LIGHT_TYPE::POINT;
    } else if (LightTypeStr == "spot") {
      LightType = LIGHT_TYPE::SPOT;
    } else {
      P_LOG(LOG_ERROR, "invlaid light type : {}", LightTypeStr);
      return;
    }

    bool BloomFlag = getJsonNode(Json, JsonPath + "/alc-with-bloom")->GetBool();
    bool ShadowFlag = false;
    if (LightType == LIGHT_TYPE::DIRECT) {
      ShadowFlag = getJsonNode(Json, JsonPath + "/alc-with-shadow")->GetBool();
    }

    LIGHT_INFO LI = {};
    CAM_INFO CI = {};
    LI.Type = LightType;
    LI.Position = {0.f, 0.f, 0.f};
    LI.ShadowFlag = ShadowFlag;
    LI.Direction.x =
        getJsonNode(Json, JsonPath + "/alc-direction/0")->GetFloat();
    LI.Direction.y =
        getJsonNode(Json, JsonPath + "/alc-direction/1")->GetFloat();
    LI.Direction.z =
        getJsonNode(Json, JsonPath + "/alc-direction/2")->GetFloat();
    LI.Intensity = getJsonNode(Json, JsonPath + "/alc-intensity")->GetFloat();
    LI.Albedo.x = getJsonNode(Json, JsonPath + "/alc-albedo/0")->GetFloat();
    LI.Albedo.y = getJsonNode(Json, JsonPath + "/alc-albedo/1")->GetFloat();
    LI.Albedo.z = getJsonNode(Json, JsonPath + "/alc-albedo/2")->GetFloat();
    LI.FalloffStart =
        getJsonNode(Json, JsonPath + "/alc-fall-off-start-end/0")->GetFloat();
    LI.FalloffEnd =
        getJsonNode(Json, JsonPath + "/alc-fall-off-start-end/1")->GetFloat();
    LI.SpotPower = getJsonNode(Json, JsonPath + "/alc-spot-power")->GetFloat();
    if (getJsonNode(Json, JsonPath + "/alc-cam-up-vec")) {
      CI.Type = LENS_TYPE::ORTHOGRAPHIC;
      CI.Position = LI.Position;
      CI.LookAtVector = LI.Direction;
      CI.NearFarZ = {0.f, 1000.f};
      CI.PerspFovYRatio = {dx::XM_PIDIV4, 16.f / 9.f};
      CI.OrthoWidthHeight = {128.f * 9.5f, 72.f * 9.5f};
      CI.UpVector.x =
          getJsonNode(Json, JsonPath + "/alc-cam-up-vec/0")->GetFloat();
      CI.UpVector.y =
          getJsonNode(Json, JsonPath + "/alc-cam-up-vec/1")->GetFloat();
      CI.UpVector.z =
          getJsonNode(Json, JsonPath + "/alc-cam-up-vec/2")->GetFloat();
    }

    Alc.addLight(LI, BloomFlag, ShadowFlag, CI);

    COMP_TYPE Type = COMP_TYPE::A_LIGHT;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Alc);
  } else if (CompType == "audio") {
    AAudioComponent Aauc(CompName, nullptr);

    UINT AudioSize = getJsonNode(Json, JsonPath + "/aauc-sounds")->Size();
    for (UINT I = 0; I < AudioSize; I++) {
      std::string SoundName =
          getJsonNode(Json, JsonPath + "/aauc-sounds/" + hyc::str::toString(I))
              ->GetString();
      Aauc.addAudio(SoundName, *Scene);
    }

    COMP_TYPE Type = COMP_TYPE::A_AUDIO;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Aauc);
  } else if (CompType == "particle") {
    AParticleComponent Apc(CompName, nullptr);

    std::string PtcTexName =
        getJsonNode(Json, JsonPath + "/apc-texture-name")->GetString();
    PARTICLE_TEXTURE PtcTex = PARTICLE_TEXTURE::SIZE;
    if (PtcTexName == "circle") {
      PtcTex = PARTICLE_TEXTURE::WHITE_CIRCLE;
    } else if (PtcTexName == "smoke") {
      PtcTex = PARTICLE_TEXTURE::WHITE_SMOKE;
    } else {
      P_LOG(LOG_ERROR, "invlaid particle texture type : {}", PtcTexName);
      return;
    }

    float EmitPreSec =
        getJsonNode(Json, JsonPath + "/apc-emit-per-second")->GetFloat();
    dx::XMFLOAT3 Velocity = {
        getJsonNode(Json, JsonPath + "/apc-velocity/0")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-velocity/1")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-velocity/2")->GetFloat()};
    dx::XMFLOAT3 PosVariance = {
        getJsonNode(Json, JsonPath + "/apc-pos-variance/0")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-pos-variance/1")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-pos-variance/2")->GetFloat()};
    float VelVariance =
        getJsonNode(Json, JsonPath + "/apc-vel-variance")->GetFloat();
    dx::XMFLOAT3 Acceleration = {
        getJsonNode(Json, JsonPath + "/apc-acceleration/0")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-acceleration/1")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-acceleration/2")->GetFloat()};
    float Mass = getJsonNode(Json, JsonPath + "/apc-particle-mass")->GetFloat();
    float LifeSpan = getJsonNode(Json, JsonPath + "/apc-life-span")->GetFloat();
    float StartSize =
        getJsonNode(Json, JsonPath + "/apc-start-end-size/0")->GetFloat();
    float EndSize =
        getJsonNode(Json, JsonPath + "/apc-start-end-size/1")->GetFloat();
    dx::XMFLOAT4 StartColor = {
        getJsonNode(Json, JsonPath + "/apc-start-color/0")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-start-color/1")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-start-color/2")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-start-color/3")->GetFloat()};
    dx::XMFLOAT4 EndColor = {
        getJsonNode(Json, JsonPath + "/apc-end-color/0")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-end-color/1")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-end-color/2")->GetFloat(),
        getJsonNode(Json, JsonPath + "/apc-end-color/3")->GetFloat()};
    bool StreakFlg =
        getJsonNode(Json, JsonPath + "/apc-streak-flag")->GetBool();

    PARTICLE_EMITTER_INFO PEI = {};
    PEI.EmitNumPerSecond = EmitPreSec;
    PEI.Velocity = Velocity;
    PEI.PosVariance = PosVariance;
    PEI.VelVariance = VelVariance;
    PEI.Acceleration = Acceleration;
    PEI.ParticleMass = Mass;
    PEI.LifeSpan = LifeSpan;
    PEI.StartSize = StartSize;
    PEI.EndSize = EndSize;
    PEI.StartColor = StartColor;
    PEI.EndColor = EndColor;
    PEI.StreakFlag = StreakFlg;
    PEI.TextureID = PtcTex;

    Apc.createEmitter(&PEI);

    COMP_TYPE Type = COMP_TYPE::A_PARTICLE;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Apc);
  } else if (CompType == "animate") {
    AAnimateComponent Aac(CompName, nullptr);

    std::string InitAni =
        getJsonNode(Json, JsonPath + "/aac-init-animation")->GetString();
    float SpdFactor = 1.f;
    if (!getJsonNode(Json, JsonPath + "/aac-speed-factor")->IsNull()) {
      SpdFactor = getJsonNode(Json, JsonPath + "/aac-speed-factor")->GetFloat();
    }
    Aac.changeAnimationTo(InitAni);
    Aac.SetSpeedFactor(SpdFactor);

    COMP_TYPE Type = COMP_TYPE::A_ANIMATE;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Aac);
  } else if (CompType == "sprite") {
    ASpriteComponent Asc(CompName, nullptr);

    JsonNode TexNameNode = getJsonNode(Json, JsonPath + "/asc-texture");
    JsonNode BillFlgNode = getJsonNode(Json, JsonPath + "/asc-billboard");
    JsonNode SizeXNode = getJsonNode(Json, JsonPath + "/asc-size/0");
    JsonNode SizeYNode = getJsonNode(Json, JsonPath + "/asc-size/1");
    JsonNode TexCUNode = getJsonNode(Json, JsonPath + "/asc-tex-coord/0");
    JsonNode TexCVNode = getJsonNode(Json, JsonPath + "/asc-tex-coord/1");
    JsonNode TexCULenNode = getJsonNode(Json, JsonPath + "/asc-tex-coord/2");
    JsonNode TexCVLenNode = getJsonNode(Json, JsonPath + "/asc-tex-coord/3");

    assert(TexNameNode && BillFlgNode && SizeXNode && SizeYNode && TexCUNode &&
           TexCVNode && TexCULenNode && TexCVLenNode);

    Asc.setSpriteProperty({SizeXNode->GetFloat(), SizeYNode->GetFloat()},
                          {TexCUNode->GetFloat(), TexCVNode->GetFloat(),
                           TexCULenNode->GetFloat(), TexCVLenNode->GetFloat()},
                          BillFlgNode->GetBool());

    JsonNode AniFlgNode = getJsonNode(Json, JsonPath + "/asc-with-animation");
    if (AniFlgNode && !AniFlgNode->IsNull() && AniFlgNode->GetBool()) {
      JsonNode StrideUNode = getJsonNode(Json, JsonPath + "/asc-stride/0");
      JsonNode StrideVNode = getJsonNode(Json, JsonPath + "/asc-stride/1");
      JsonNode MaxCutNode = getJsonNode(Json, JsonPath + "/asc-max-cut");
      JsonNode SwitchTimeNode =
          getJsonNode(Json, JsonPath + "/asc-switch-time");
      JsonNode RepeatFlgNode = getJsonNode(Json, JsonPath + "/asc-repeat-flag");

      assert(StrideUNode && StrideVNode && MaxCutNode && SwitchTimeNode &&
             RepeatFlgNode);

      Asc.setAnimationProperty(
          {StrideUNode->GetFloat(), StrideVNode->GetFloat()},
          MaxCutNode->GetUint(), RepeatFlgNode->GetBool(),
          SwitchTimeNode->GetFloat());
    }

    Asc.createGeoPointWithTexture(Scene, TexNameNode->GetString());

    COMP_TYPE Type = COMP_TYPE::A_SPRITE;
    Actor->addAComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Asc);
  } else {
    P_LOG(LOG_ERROR, "invlaid comp type : {}", CompType);
    return;
  }
}

void ObjectFactory::createUiComp(SceneNode *Scene,
                                 UiObject *Ui,
                                 JsonFile &Json,
                                 const std::string &JsonPath) {
  std::string CompType = getJsonNode(Json, JsonPath + "/type")->GetString();
  std::string CompName = Ui->getObjectName() + "-" + CompType;

  if (CompType == "transform") {
    UTransformComponent Utc(CompName, nullptr);

    float Pos[3] = {0.f};
    float Ang[3] = {0.f};
    float Sca[3] = {0.f};
    for (UINT I = 0, E = ARRAYSIZE(Pos); I < E; I++) {
      Pos[I] = getJsonNode(Json, JsonPath + "/utc-init-position/" +
                                     hyc::str::toString(I))
                   ->GetFloat();
      Ang[I] = getJsonNode(Json, JsonPath + "/utc-init-angle/" +
                                     hyc::str::toString(I))
                   ->GetFloat();
      Sca[I] = getJsonNode(Json, JsonPath + "/utc-init-scale/" +
                                     hyc::str::toString(I))
                   ->GetFloat();
    }
    Utc.forcePosition({Pos[0], Pos[1], Pos[2]});
    Utc.forceRotation({Ang[0], Ang[1], Ang[2]});
    Utc.forceScaling({Sca[0], Sca[1], Sca[2]});

    COMP_TYPE Type = COMP_TYPE::U_TRANSFORM;
    Ui->addUComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Utc);
  } else if (CompType == "input") {
    UInputComponent Uic(CompName, nullptr);

    std::string InputFuncName =
        getJsonNode(Json, JsonPath + "/uic-func-name")->GetString();
    auto Found = UiInputFuncPtrMap.find(InputFuncName);
    if (Found == UiInputFuncPtrMap.end()) {
      P_LOG(LOG_ERROR, "invlaid input func name : {}", InputFuncName);
      return;
    }
    Uic.setInputFunction(Found->second);

    COMP_TYPE Type = COMP_TYPE::U_INPUT;
    Ui->addUComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Uic);
  } else if (CompType == "interact") {
    UInteractComponent Uitc(CompName, nullptr);

    std::string InitFuncName =
        getJsonNode(Json, JsonPath + "/uitc-init-func-name")->GetString();
    std::string UpdateFuncName =
        getJsonNode(Json, JsonPath + "/uitc-update-func-name")->GetString();
    std::string DestoryFuncName =
        getJsonNode(Json, JsonPath + "/uitc-destory-func-name")->GetString();
    auto FoundInit = UiInteractInitFuncPtrMap.find(InitFuncName);
    if (FoundInit == UiInteractInitFuncPtrMap.end()) {
      P_LOG(LOG_ERROR, "invlaid init func name : {}", InitFuncName);
      return;
    }
    Uitc.setInitFunction(FoundInit->second);
    auto FoundUpdate = UiInteractUpdateFuncPtrMap.find(UpdateFuncName);
    if (FoundUpdate == UiInteractUpdateFuncPtrMap.end()) {
      P_LOG(LOG_ERROR, "invlaid update func name : {}", UpdateFuncName);
      return;
    }
    Uitc.setUpdateFunction(FoundUpdate->second);
    auto FoundDestory = UiInteractDestoryFuncPtrMap.find(DestoryFuncName);
    if (FoundDestory == UiInteractDestoryFuncPtrMap.end()) {
      P_LOG(LOG_ERROR, "invlaid destory func name : {}", DestoryFuncName);
      return;
    }
    Uitc.setDestoryFunction(FoundDestory->second);

    COMP_TYPE Type = COMP_TYPE::U_INTERACT;
    Ui->addUComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Uitc);
  } else if (CompType == "timer") {
    UTimerComponent Utmc(CompName, nullptr);

    UINT TimerSize = getJsonNode(Json, JsonPath + "/utmc-timers")->Size();
    for (UINT I = 0; I < TimerSize; I++) {
      std::string TimerName =
          getJsonNode(Json, JsonPath + "/utmc-timers/" + hyc::str::toString(I))
              ->GetString();
      Utmc.addTimer(TimerName);
    }

    COMP_TYPE Type = COMP_TYPE::U_TIMER;
    Ui->addUComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Utmc);
  } else if (CompType == "audio") {
    UAudioComponent Uauc(CompName, nullptr);

    UINT AudioSize = getJsonNode(Json, JsonPath + "/uauc-sounds")->Size();
    for (UINT i = 0; i < AudioSize; i++) {
      std::string SoundName =
          getJsonNode(Json, JsonPath + "/uauc-sounds/" + hyc::str::toString(i))
              ->GetString();
      Uauc.addAudio(SoundName, *Scene);
    }

    COMP_TYPE Type = COMP_TYPE::U_AUDIO;
    Ui->addUComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Uauc);
  } else if (CompType == "sprite") {
    USpriteComponent Usc(CompName, nullptr);

    dx::XMFLOAT4 OffsetColor = {
        getJsonNode(Json, JsonPath + "/usc-offset-color/0")->GetFloat(),
        getJsonNode(Json, JsonPath + "/usc-offset-color/1")->GetFloat(),
        getJsonNode(Json, JsonPath + "/usc-offset-color/2")->GetFloat(),
        getJsonNode(Json, JsonPath + "/usc-offset-color/3")->GetFloat()};
    std::string TexFile =
        getJsonNode(Json, JsonPath + "/usc-tex-file")->GetString();

    Usc.createSpriteMesh(Scene, OffsetColor, TexFile);

    COMP_TYPE Type = COMP_TYPE::U_SPRITE;
    Ui->addUComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Usc);
  } else if (CompType == "animate") {
    UAnimateComponent Uac(CompName, nullptr);

    UINT AniSize = getJsonNode(Json, JsonPath + "/uac-animates")->Size();
    std::string AniArrayPath = JsonPath + "/uac-animates/";
    for (UINT I = 0; I < AniSize; I++) {
      AniArrayPath = JsonPath + "/uac-animates/" + hyc::str::toString(I);

      std::string AniName =
          getJsonNode(Json, AniArrayPath + "/name")->GetString();
      std::string AniFile =
          getJsonNode(Json, AniArrayPath + "/tex-file")->GetString();
      dx::XMFLOAT2 Stride = {
          getJsonNode(Json, AniArrayPath + "/u-v-stride/0")->GetFloat(),
          getJsonNode(Json, AniArrayPath + "/u-v-stride/1")->GetFloat()};
      UINT MaxCount = getJsonNode(Json, AniArrayPath + "/max-count")->GetUint();
      float SwitchTime =
          getJsonNode(Json, AniArrayPath + "/switch-time")->GetFloat();
      bool RepeatFlg =
          getJsonNode(Json, AniArrayPath + "/repeat-flag")->GetBool();

      Uac.loadAnimate(AniName, AniFile, Stride, MaxCount, RepeatFlg,
                      SwitchTime);
    }

    JsonNode InitNode = getJsonNode(Json, JsonPath + "/uac-init-ani");
    if (InitNode && !InitNode->IsNull()) {
      Uac.changeAnimateTo(InitNode->GetString());
    }

    COMP_TYPE Type = COMP_TYPE::U_ANIMATE;
    Ui->addUComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Uac);
  } else if (CompType == "button") {
    UButtonComponent Ubc(CompName, nullptr);

    bool Selected =
        getJsonNode(Json, JsonPath + "/ubc-init-selected")->GetBool();
    Ubc.setIsBeingSelected(Selected);
    JsonNode SurdBtn = getJsonNode(Json, JsonPath + "/ubc-up-btn");
    if (SurdBtn && !SurdBtn->IsNull()) {
      Ubc.setUpBtnObjName(SurdBtn->GetString());
    }
    SurdBtn = getJsonNode(Json, JsonPath + "/ubc-down-btn");
    if (SurdBtn && !SurdBtn->IsNull()) {
      Ubc.setDownBtnObjName(SurdBtn->GetString());
    }
    SurdBtn = getJsonNode(Json, JsonPath + "/ubc-left-btn");
    if (SurdBtn && !SurdBtn->IsNull()) {
      Ubc.setLeftBtnObjName(SurdBtn->GetString());
    }
    SurdBtn = getJsonNode(Json, JsonPath + "/ubc-right-btn");
    if (SurdBtn && !SurdBtn->IsNull()) {
      Ubc.setRightBtnObjName(SurdBtn->GetString());
    }

    COMP_TYPE Type = COMP_TYPE::U_BUTTON;
    Ui->addUComponent(Type);
    Scene->getComponentContainer()->addComponent(Type, Ubc);
  } else {
    P_LOG(LOG_ERROR, "invlaid comp type : {}", CompType);
    return;
  }
}
