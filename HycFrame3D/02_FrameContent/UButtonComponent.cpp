#include "UButtonComponent.h"

#include "AssetsPool.h"
#include "SceneNode.h"
#include "UTransformComponent.h"
#include "UiObject.h"

#include <RSMeshHelper.h>
#include <RSRoot_DX11.h>
#include <TextUtility.h>
#include <WM_Interface.h>

static UButtonComponent *G_SelectedBtnCompPtr = nullptr;

static bool G_CanThisFrameProcessSelect = true;
static bool G_ShouleUseMouseSelect = false;

static SUBMESH_DATA *G_SelectTexMeshPtr = nullptr;

static dx::XMFLOAT2 G_ScreenSpaceCursorPosition = {0.f, 0.f};

static std::string G_SelectFlagTexture = "";

UButtonComponent::UButtonComponent(const std::string &CompName,
                                   UiObject *UiOwner)
    : UiComponent(CompName, UiOwner),
      SurroundBtnObjectNames({NULL_BTN, NULL_BTN, NULL_BTN, NULL_BTN}),
      SelectedFlag(false) {}

UButtonComponent::~UButtonComponent() {}

bool UButtonComponent::init() {
  if (G_SelectedBtnCompPtr) {
    G_SelectedBtnCompPtr = nullptr;
  }
  if (G_SelectTexMeshPtr) {
    G_SelectTexMeshPtr = nullptr;
  }
  G_CanThisFrameProcessSelect = true;
  G_ShouleUseMouseSelect = false;

  if (SelectedFlag) {
    G_SelectedBtnCompPtr = this;
  }

  SUBMESH_DATA *MeshPtr =
      getUiOwner()->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
          SELECTED_BTN_SPRITE_NAME);
  if (!MeshPtr) {
    if (G_SelectFlagTexture == "") {
      using namespace hyc::text;
      TomlNode BtnFlgConfig = {};
      std::string ErrorMess = "";
      if (!loadTomlAndParse(BtnFlgConfig,
                            ".\\Assets\\Configs\\selected-flag-config.toml",
                            ErrorMess)) {
        P_LOG(LOG_ERROR, "failed to parse btn flag config : %s\n",
              ErrorMess.c_str());
        return false;
      }
      G_SelectFlagTexture =
          getAs<std::string>(BtnFlgConfig["flag-texture"]["file"]);
    }

    RS_SUBMESH_DATA BtnSelect =
        getRSDX11RootInstance()
            ->getMeshHelper()
            ->getGeoGenerator()
            ->createSpriteRect(LAYOUT_TYPE::NORMAL_TANGENT_TEX,
                               G_SelectFlagTexture);
    getUiOwner()->getSceneNode().getAssetsPool()->insertNewSubMesh(
        SELECTED_BTN_SPRITE_NAME, BtnSelect, MESH_TYPE::UI_SPRITE);
    MeshPtr = getUiOwner()->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
        SELECTED_BTN_SPRITE_NAME);

    RS_INSTANCE_DATA InsData = {};
    InsData.CustomizedData1 = {1.f, 1.f, 1.f, 1.f};
    InsData.CustomizedData2 = {0.f, 0.f, 1.f, 1.f};
    MeshPtr->InstanceMap.insert({SELECTED_BTN_SPRITE_NAME, InsData});

    G_SelectTexMeshPtr = MeshPtr;
  }

  return true;
}

void UButtonComponent::update(const Timer &Timer) {
  if (!G_CanThisFrameProcessSelect && !G_ShouleUseMouseSelect) {
    G_CanThisFrameProcessSelect = true;
  }

  if (G_ShouleUseMouseSelect) {
    auto Utc = getUiOwner()->getComponent<UTransformComponent>();
#ifdef _DEBUG
    assert(Utc);
#endif // _DEBUG
    static dx::XMFLOAT2 Cur = {};
    static dx::XMFLOAT3 Pos = {};
    static dx::XMFLOAT3 Size = {};
    Cur = G_ScreenSpaceCursorPosition;
    Pos = Utc->getPosition();
    Size = Utc->getScaling();
    if (fabsf(Cur.x - Pos.x) < Size.x / 2.f &&
        fabsf(Cur.y - Pos.y) < Size.y / 2.f) {
      if (G_SelectedBtnCompPtr) {
        G_SelectedBtnCompPtr->setIsBeingSelected(false);
      }
      setIsBeingSelected(true);
      G_SelectedBtnCompPtr = this;
    }
  }

  if (SelectedFlag) {
    G_SelectedBtnCompPtr = this;
    syncDataFromTransform();
  }
}

void UButtonComponent::destory() {
  if (G_SelectedBtnCompPtr) {
    G_SelectedBtnCompPtr = nullptr;
  }
  if (G_SelectTexMeshPtr) {
    G_SelectTexMeshPtr = nullptr;
  }
  G_CanThisFrameProcessSelect = true;
  G_ShouleUseMouseSelect = false;
}

void UButtonComponent::setIsBeingSelected(bool BeingSelected) {
  SelectedFlag = BeingSelected;
}

bool UButtonComponent::isBeingSelected() const { return SelectedFlag; }

bool UButtonComponent::isCursorOnBtn() {
  if (G_ShouleUseMouseSelect && SelectedFlag) {
    auto Utc = getUiOwner()->getComponent<UTransformComponent>();
#ifdef _DEBUG
    assert(Utc);
#endif // _DEBUG
    static dx::XMFLOAT2 Cur = {};
    static dx::XMFLOAT3 Pos = {};
    static dx::XMFLOAT3 Size = {};
    Cur = G_ScreenSpaceCursorPosition;
    Pos = Utc->getPosition();
    Size = Utc->getScaling();
    if (fabsf(Cur.x - Pos.x) < Size.x / 2.f &&
        fabsf(Cur.y - Pos.y) < Size.y / 2.f) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void UButtonComponent::selectUpBtn() {
  if (SelectedFlag && G_CanThisFrameProcessSelect && !G_ShouleUseMouseSelect &&
      SurroundBtnObjectNames[BTN_UP] != NULL_BTN) {
    auto UpUi = getUiOwner()->getSceneNode().getUiObject(
        SurroundBtnObjectNames[BTN_UP]);
    if (!UpUi) {
      return;
    }

    auto UpUbc = UpUi->getComponent<UButtonComponent>();
    if (!UpUbc) {
      return;
    }

    setIsBeingSelected(false);
    UpUbc->setIsBeingSelected(true);
    G_CanThisFrameProcessSelect = false;
  }
}

void UButtonComponent::selectDownBtn() {
  if (SelectedFlag && G_CanThisFrameProcessSelect && !G_ShouleUseMouseSelect &&
      SurroundBtnObjectNames[BTN_DOWN] != NULL_BTN) {
    auto DownUi = getUiOwner()->getSceneNode().getUiObject(
        SurroundBtnObjectNames[BTN_DOWN]);
    if (!DownUi) {
      return;
    }

    auto DownUbc = DownUi->getComponent<UButtonComponent>();
    if (!DownUbc) {
      return;
    }

    setIsBeingSelected(false);
    DownUbc->setIsBeingSelected(true);
    G_CanThisFrameProcessSelect = false;
  }
}

void UButtonComponent::selectLeftBtn() {
  if (SelectedFlag && G_CanThisFrameProcessSelect && !G_ShouleUseMouseSelect &&
      SurroundBtnObjectNames[BTN_LEFT] != NULL_BTN) {
    auto LeftUi = getUiOwner()->getSceneNode().getUiObject(
        SurroundBtnObjectNames[BTN_LEFT]);
    if (!LeftUi) {
      return;
    }

    auto LeftUbc = LeftUi->getComponent<UButtonComponent>();
    if (!LeftUbc) {
      return;
    }

    setIsBeingSelected(false);
    LeftUbc->setIsBeingSelected(true);
    G_CanThisFrameProcessSelect = false;
  }
}

void UButtonComponent::selectRightBtn() {
  if (SelectedFlag && G_CanThisFrameProcessSelect && !G_ShouleUseMouseSelect &&
      SurroundBtnObjectNames[BTN_RIGHT] != NULL_BTN) {
    auto RightUi = getUiOwner()->getSceneNode().getUiObject(
        SurroundBtnObjectNames[BTN_RIGHT]);
    if (!RightUi) {
      return;
    }

    auto RightUbc = RightUi->getComponent<UButtonComponent>();
    if (!RightUbc) {
      return;
    }

    setIsBeingSelected(false);
    RightUbc->setIsBeingSelected(true);
    G_CanThisFrameProcessSelect = false;
  }
}

UButtonComponent *UButtonComponent::getUpBtn() {
  auto Ui =
      getUiOwner()->getSceneNode().getUiObject(SurroundBtnObjectNames[BTN_UP]);
  if (!Ui) {
    P_LOG(LOG_WARNING, "this ui obj doesnt exist : %s\n",
          SurroundBtnObjectNames[BTN_UP].c_str());
    return nullptr;
  }

  auto Ubc = Ui->getComponent<UButtonComponent>();
  if (!Ubc) {
    P_LOG(LOG_WARNING, "this ui obj doesnt have a btn comp : %s\n",
          SurroundBtnObjectNames[BTN_UP].c_str());
    return nullptr;
  } else {
    return Ubc;
  }
}

UButtonComponent *UButtonComponent::getDownBtn() {
  auto Ui = getUiOwner()->getSceneNode().getUiObject(
      SurroundBtnObjectNames[BTN_DOWN]);
  if (!Ui) {
    P_LOG(LOG_WARNING, "this ui obj doesnt exist : %s\n",
          SurroundBtnObjectNames[BTN_DOWN].c_str());
    return nullptr;
  }

  auto Ubc = Ui->getComponent<UButtonComponent>();
  if (!Ubc) {
    P_LOG(LOG_WARNING, "this ui obj doesnt have a btn comp : %s\n",
          SurroundBtnObjectNames[BTN_DOWN].c_str());
    return nullptr;
  } else {
    return Ubc;
  }
}

UButtonComponent *UButtonComponent::getLeftBtn() {
  auto Ui = getUiOwner()->getSceneNode().getUiObject(
      SurroundBtnObjectNames[BTN_LEFT]);
  if (!Ui) {
    P_LOG(LOG_WARNING, "this ui obj doesnt exist : %s\n",
          SurroundBtnObjectNames[BTN_LEFT].c_str());
    return nullptr;
  }

  auto Ubc = Ui->getComponent<UButtonComponent>();
  if (!Ubc) {
    P_LOG(LOG_WARNING, "this ui obj doesnt have a btn comp : %s\n",
          SurroundBtnObjectNames[BTN_LEFT].c_str());
    return nullptr;
  } else {
    return Ubc;
  }
}

UButtonComponent *UButtonComponent::getRightBtn() {
  auto Ui = getUiOwner()->getSceneNode().getUiObject(
      SurroundBtnObjectNames[BTN_RIGHT]);
  if (!Ui) {
    P_LOG(LOG_WARNING, "this ui obj doesnt exist : %s\n",
          SurroundBtnObjectNames[BTN_RIGHT].c_str());
    return nullptr;
  }

  auto Ubc = Ui->getComponent<UButtonComponent>();
  if (!Ubc) {
    P_LOG(LOG_WARNING, "this ui obj doesnt have a btn comp : %s\n",
          SurroundBtnObjectNames[BTN_RIGHT].c_str());
    return nullptr;
  } else {
    return Ubc;
  }
}

void UButtonComponent::setUpBtnObjName(const std::string &UpBtn) {
  SurroundBtnObjectNames[BTN_UP] = UpBtn;
}

void UButtonComponent::setDownBtnObjName(const std::string &DownBtn) {
  SurroundBtnObjectNames[BTN_DOWN] = DownBtn;
}

void UButtonComponent::setLeftBtnObjName(const std::string &LeftBtn) {
  SurroundBtnObjectNames[BTN_LEFT] = LeftBtn;
}

void UButtonComponent::setRightBtnObjName(const std::string &RightBtn) {
  SurroundBtnObjectNames[BTN_RIGHT] = RightBtn;
}

void UButtonComponent::syncDataFromTransform() {
  UTransformComponent *Utc = getUiOwner()->getComponent<UTransformComponent>();
#ifdef _DEBUG
  assert(Utc);
#endif // _DEBUG

  dx::XMFLOAT3 World = Utc->getProcessingPosition();
  dx::XMFLOAT3 Angle = Utc->getProcessingRotation();
  dx::XMFLOAT3 Scale = Utc->getProcessingScaling();

  auto &InsMap = getUiOwner()
                     ->getSceneNode()
                     .getAssetsPool()
                     ->getSubMeshIfExisted(SELECTED_BTN_SPRITE_NAME)
                     ->InstanceMap;

  for (auto &Ins : InsMap) {
    auto &InsData = Ins.second;

    dx::XMMATRIX Matrix = {};
    Matrix = dx::XMMatrixMultiply(
        dx::XMMatrixScaling(Scale.x, Scale.y, Scale.z),
        dx::XMMatrixRotationRollPitchYaw(Angle.x, Angle.y, Angle.z));
    Matrix = dx::XMMatrixMultiply(
        Matrix, dx::XMMatrixTranslation(World.x, World.y, World.z));
    dx::XMStoreFloat4x4(&(InsData.WorldMatrix), Matrix);

    break;
  }
}

void UButtonComponent::setScreenSpaceCursorPos(float InputX, float InputY) {
  G_ScreenSpaceCursorPosition = {InputX, InputY};
}

void UButtonComponent::setShouldUseMouse(bool ShouldMouse) {
  G_ShouleUseMouseSelect = ShouldMouse;
}
