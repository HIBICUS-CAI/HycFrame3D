#include "01_MaterialEditor.h"

#include <RSPipelinesManager.h>
#include <RSRoot_DX11.h>
#include <RSStaticResources.h>

void
registerMaterialEditor(ObjectFactory *Factory) {
#ifdef _DEBUG
  assert(Factory);
#endif // _DEBUG
  Factory->getAInputMapPtr().insert(
      {FUNC_NAME(matEditorInput), matEditorInput});
  Factory->getAInitMapPtr().insert({FUNC_NAME(matEditorInit), matEditorInit});
  Factory->getAUpdateMapPtr().insert(
      {FUNC_NAME(matEditorUpdate), matEditorUpdate});
  Factory->getADestoryMapPtr().insert(
      {FUNC_NAME(matEditorDestory), matEditorDestory});
}

enum class MAT_TYPE {
  SUBSURFACE = KB_1,
  METALLIC = KB_2,
  SPECULAR = KB_3,
  SPECULARTINT = KB_4,
  ROUGHNESS = KB_5,
  ANISOTROPIC = KB_6,
  SHEEN = KB_7,
  SHEENTINT = KB_8,
  CLEARCOAT = KB_9,
  CLEARCOATGLOSS = KB_0
};

static ATransformComponent *G_PointLightAtc = nullptr;
static ATransformComponent *G_MaterialBallAtc = nullptr;
static RS_MATERIAL_DATA *G_Material = nullptr;
static MAT_TYPE G_EditingMatTerm = MAT_TYPE::ROUGHNESS;
static SUBMESH_DATA *G_MatBallMesh = nullptr;

void
OutputThisMaterialInfo() {
  std::string MatInfo = "Material Info\n";
  MatInfo += "===============================\n";
  MatInfo += "FresnelR0 :\t\t" + std::to_string(G_Material->FresnelR0.x);
  MatInfo += ", " + std::to_string(G_Material->FresnelR0.y);
  MatInfo += ", " + std::to_string(G_Material->FresnelR0.z);
  MatInfo += "\n";
  MatInfo += "SubSurface :\t" + std::to_string(G_Material->SubSurface) + ", \t";
  MatInfo += "Metallic :\t\t\t" + std::to_string(G_Material->Metallic) + "\n";
  MatInfo += "Specular :\t\t" + std::to_string(G_Material->Specular) + ", \t";
  MatInfo +=
      "SpecularTint :\t\t" + std::to_string(G_Material->SpecularTint) + "\n";
  MatInfo += "Roughness :\t\t" + std::to_string(G_Material->Roughness) + ", \t";
  MatInfo +=
      "Anisotropic :\t\t" + std::to_string(G_Material->Anisotropic) + "\n";
  MatInfo += "Sheen :\t\t\t" + std::to_string(G_Material->Sheen) + ", \t";
  MatInfo += "SheenTint :\t\t\t" + std::to_string(G_Material->SheenTint) + "\n";
  MatInfo += "Clearcoat :\t\t" + std::to_string(G_Material->Clearcoat) + ", \t";
  MatInfo +=
      "ClearcoatGloss :\t" + std::to_string(G_Material->ClearcoatGloss) + "\n";
  MatInfo += "===============================\n";
  P_LOG(LOG_DEBUG, "%s", MatInfo.c_str());
}

void
matEditorInput(AInputComponent *Aic, Timer &Timer) {
  if (input::isKeyPushedInSingle(KB_F5)) {
    Aic->getSceneNode().getSceneManager()->loadSceneNode("material-scene",
                                                         "material-scene.json");
  }

  if (input::isKeyPushedInSingle(KB_P)) {
    static bool Simp = false;
    if (Simp) {
      getRSDX11RootInstance()->getPipelinesManager()->setPipeline(
          "light-pipeline");
    } else {
      getRSDX11RootInstance()->getPipelinesManager()->setPipeline(
          "simple-pipeline");
    }
    Simp = !Simp;
  }

  const float Deltatime = Timer.floatDeltaTime();

  if (input::isKeyDownInSingle(M_LEFTBTN)) {
    auto MouseOffset = input::getMouseOffset();
    float HoriR = -MouseOffset.x * Deltatime / 500.f;
    float VertR = -MouseOffset.y * Deltatime / 500.f;
    G_MaterialBallAtc->rotate({VertR, HoriR, 0.f});
  }

  if (input::isKeyDownInSingle(KB_W)) {
    G_PointLightAtc->translateZAsix(0.1f * Deltatime);
  }
  if (input::isKeyDownInSingle(KB_S)) {
    G_PointLightAtc->translateZAsix(-0.1f * Deltatime);
  }
  if (input::isKeyDownInSingle(KB_D)) {
    G_PointLightAtc->translateXAsix(0.1f * Deltatime);
  }
  if (input::isKeyDownInSingle(KB_A)) {
    G_PointLightAtc->translateXAsix(-0.1f * Deltatime);
  }
  if (input::isKeyDownInSingle(KB_E)) {
    G_PointLightAtc->translateYAsix(0.1f * Deltatime);
  }
  if (input::isKeyDownInSingle(KB_Q)) {
    G_PointLightAtc->translateYAsix(-0.1f * Deltatime);
  }

  UINT Check = 0;
  switch (MAT_TYPE::SUBSURFACE) {
  case MAT_TYPE::SUBSURFACE:
    Check = static_cast<UINT>(MAT_TYPE::SUBSURFACE);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing subsurface\n");
      break;
    }
  case MAT_TYPE::METALLIC:
    Check = static_cast<UINT>(MAT_TYPE::METALLIC);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing metallic\n");
      break;
    }
  case MAT_TYPE::SPECULAR:
    Check = static_cast<UINT>(MAT_TYPE::SPECULAR);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing specular\n");
      break;
    }
  case MAT_TYPE::SPECULARTINT:
    Check = static_cast<UINT>(MAT_TYPE::SPECULARTINT);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing specular tint\n");
      break;
    }
  case MAT_TYPE::ROUGHNESS:
    Check = static_cast<UINT>(MAT_TYPE::ROUGHNESS);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing roughness\n");
      break;
    }
  case MAT_TYPE::ANISOTROPIC:
    Check = static_cast<UINT>(MAT_TYPE::ANISOTROPIC);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing anisotropic\n");
      break;
    }
  case MAT_TYPE::SHEEN:
    Check = static_cast<UINT>(MAT_TYPE::SHEEN);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing sheen\n");
      break;
    }
  case MAT_TYPE::SHEENTINT:
    Check = static_cast<UINT>(MAT_TYPE::SHEENTINT);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing sheen tint\n");
      break;
    }
  case MAT_TYPE::CLEARCOAT:
    Check = static_cast<UINT>(MAT_TYPE::CLEARCOAT);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing clearcoat\n");
      break;
    }
  case MAT_TYPE::CLEARCOATGLOSS:
    Check = static_cast<UINT>(MAT_TYPE::CLEARCOATGLOSS);
    if (input::isKeyPushedInSingle(Check)) {
      G_EditingMatTerm = static_cast<MAT_TYPE>(Check);
      P_LOG(LOG_DEBUG, "editing clearcoat gloss\n");
      break;
    }
  }

  float *EditValue = nullptr;
  switch (G_EditingMatTerm) {
  case MAT_TYPE::SUBSURFACE:
    EditValue = &G_Material->SubSurface;
    break;
  case MAT_TYPE::METALLIC:
    EditValue = &G_Material->Metallic;
    break;
  case MAT_TYPE::SPECULAR:
    EditValue = &G_Material->Specular;
    break;
  case MAT_TYPE::SPECULARTINT:
    EditValue = &G_Material->SpecularTint;
    break;
  case MAT_TYPE::ROUGHNESS:
    EditValue = &G_Material->Roughness;
    break;
  case MAT_TYPE::ANISOTROPIC:
    EditValue = &G_Material->Anisotropic;
    break;
  case MAT_TYPE::SHEEN:
    EditValue = &G_Material->Sheen;
    break;
  case MAT_TYPE::SHEENTINT:
    EditValue = &G_Material->SheenTint;
    break;
  case MAT_TYPE::CLEARCOAT:
    EditValue = &G_Material->Clearcoat;
    break;
  case MAT_TYPE::CLEARCOATGLOSS:
    EditValue = &G_Material->ClearcoatGloss;
    break;
  }
  assert(EditValue);
  if (input::isKeyDownInSingle(KB_RIGHT)) {
    *EditValue += Deltatime / 1000.f;
    if (*EditValue > 1.f) {
      *EditValue = 1.f;
    }
    getRSDX11RootInstance()->getStaticResources()->remapMaterialData();
  }
  if (input::isKeyDownInSingle(KB_LEFT)) {
    *EditValue -= Deltatime / 1000.f;
    if (*EditValue < 0.f) {
      *EditValue = 0.f;
    }
    getRSDX11RootInstance()->getStaticResources()->remapMaterialData();
  }

  float &Factor =
      G_MatBallMesh->InstanceMap.begin()->second.MaterialData.InterpolateFactor;
  if (input::isKeyDownInSingle(KB_COMMA)) {
    Factor -= Deltatime / 1000.f;
    if (Factor < 0.f) {
      Factor = 0.f;
    }
  }
  if (input::isKeyDownInSingle(KB_PERIOD)) {
    Factor += Deltatime / 1000.f;
    if (Factor > 1.f) {
      Factor = 1.f;
    }
  }
  if (input::isKeyPushedInSingle(KB_SLASH)) {
    P_LOG(LOG_DEBUG, "current material factor : %f\n", Factor);
  }

  if (input::isKeyPushedInSingle(KB_RETURN)) {
    OutputThisMaterialInfo();
  }
}

bool
matEditorInit(AInteractComponent *Aitc) {
  std::string Basic = "light-pipeline";
  getRSDX11RootInstance()->getPipelinesManager()->setPipeline(Basic);

  G_PointLightAtc = Aitc->getActorObject("point-light-actor")
                        ->getComponent<ATransformComponent>();
  if (!G_PointLightAtc) {
    return false;
  }

  G_MaterialBallAtc = Aitc->getActorObject("mat-ball-actor")
                          ->getComponent<ATransformComponent>();
  if (!G_MaterialBallAtc) {
    return false;
  }

  G_MatBallMesh = &(
      *Aitc->getSceneNode().getAssetsPool()->getSubMeshIfExisted("mat-ball0"));
  if (!G_MatBallMesh) {
    return false;
  }

  G_Material = getRSDX11RootInstance()
                   ->getStaticResources()
                   ->getMaterialDataPtrForTest();
  if (!G_Material) {
    return false;
  }

  G_EditingMatTerm = MAT_TYPE::ROUGHNESS;

  return true;
}

void
matEditorUpdate(AInteractComponent *Aitc, Timer &Timer) {}

void
matEditorDestory(AInteractComponent *Aitc) {
  G_PointLightAtc = nullptr;
  G_MaterialBallAtc = nullptr;
  G_Material = nullptr;
  G_MatBallMesh = nullptr;
}
