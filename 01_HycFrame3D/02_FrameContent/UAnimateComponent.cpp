#include "UAnimateComponent.h"

#include "AssetsPool.h"
#include "SceneNode.h"
#include "USpriteComponent.h"
#include "UiObject.h"

#include <DDSTextureLoader11.h>
#include <RSCommon.h>
#include <RSDevices.h>
#include <RSResourceManager.h>
#include <RSRoot_DX11.h>
#include <WICTextureLoader11.h>

#include <vector>

UAnimateComponent::UAnimateComponent(const std::string &CompName,
                                     UiObject *UiOwner)
    : UiComponent(CompName, UiOwner), AnimateMap({}), CurrentAnimateCut(0),
      CurrentAnimate(nullptr), AnimateChangedFlg(false), TimeCounter(0.f) {}

UAnimateComponent::~UAnimateComponent() {}

bool UAnimateComponent::init() { return true; }

void UAnimateComponent::update(const Timer &Timer) {
  if (AnimateChangedFlg) {
    AnimateChangedFlg = false;
    resetCurrentAnimate();
    syncAniInfoToSprite();
  }

  if (CurrentAnimate) {
    if (TimeCounter > CurrentAnimate->SwitchTime) {
      TimeCounter = 0.f;
      ++CurrentAnimateCut;
      if (CurrentAnimateCut >= CurrentAnimate->MaxCut) {
        if (CurrentAnimate->RepeatFlg) {
          CurrentAnimateCut = 0;
        } else {
          --CurrentAnimateCut;
        }
      }
      syncAniInfoToSprite();
    }
    TimeCounter += Timer.floatDeltaTime() / 1000.f;
  }
}

void UAnimateComponent::destory() {
  CurrentAnimate = nullptr;
  for (const auto &Ani : AnimateMap) {
    delete Ani.second;
  }

  AnimateMap.clear();
}

bool UAnimateComponent::loadAnimate(const std::string &AniName,
                                    const std::string &AniPath,
                                    const DirectX::XMFLOAT2 &Stride,
                                    UINT MaxCount,
                                    bool RepeatFlg,
                                    float SwitchTime) {
  std::wstring TexPathWStr = L"";
  HRESULT Hr = S_OK;
  ID3D11ShaderResourceView *Srv = nullptr;
  TexPathWStr = std::wstring(AniPath.begin(), AniPath.end());
  TexPathWStr = L".\\Assets\\Textures\\" + TexPathWStr;

  auto ResourceManager = getRSDX11RootInstance()->getResourceManager();
  auto IfExist = ResourceManager->getMeshSrv(AniPath);

  if (IfExist) {
  } else if (AniPath.find(".dds") != std::string::npos ||
             AniPath.find(".DDS") != std::string::npos) {
    Hr = DirectX::CreateDDSTextureFromFile(
        getRSDX11RootInstance()->getDevices()->getDevice(), TexPathWStr.c_str(),
        nullptr, &Srv);
    if (SUCCEEDED(Hr)) {
      getRSDX11RootInstance()->getResourceManager()->addMeshSrv(AniPath, Srv);
    } else {
      P_LOG(LOG_ERROR, "fail to load texture : {}", AniPath);
      return false;
    }
  } else {
    Hr = DirectX::CreateWICTextureFromFile(
        getRSDX11RootInstance()->getDevices()->getDevice(), TexPathWStr.c_str(),
        nullptr, &Srv);
    if (SUCCEEDED(Hr)) {
      getRSDX11RootInstance()->getResourceManager()->addMeshSrv(AniPath, Srv);
    } else {
      P_LOG(LOG_ERROR, "fail to load texture : {}", AniPath);
      return false;
    }
  }

  ANIMATE_INFO *Ani = new ANIMATE_INFO();
  Ani->TexName = AniPath;
  Ani->MaxCut = MaxCount;
  Ani->Stride = Stride;
  Ani->RepeatFlg = RepeatFlg;
  Ani->SwitchTime = SwitchTime;

  AnimateMap.insert({AniName, Ani});

  return true;
}

void UAnimateComponent::deleteAnimate(const std::string &AniName) {
  auto Found = AnimateMap.find(AniName);
  if (Found != AnimateMap.end()) {
    if (CurrentAnimate == Found->second) {
      CurrentAnimate = nullptr;
    }
    delete Found->second;
  } else {
    P_LOG(LOG_WARNING, "this animate doesnt exist : {}", AniName);
  }
}

void UAnimateComponent::resetCurrentAnimate() {
  CurrentAnimateCut = 0;
  TimeCounter = 0.f;
}

void UAnimateComponent::clearCurrentAnimate() {
  CurrentAnimate = nullptr;
  CurrentAnimateCut = 0;
  TimeCounter = 0.f;
}

void UAnimateComponent::changeAnimateTo(const std::string &AniName) {
  if (AnimateMap.find(AniName) == AnimateMap.end()) {
    P_LOG(LOG_WARNING, "cannot find this animation : {}", AniName);
    return;
  }

  CurrentAnimate = AnimateMap[AniName];
  AnimateChangedFlg = true;
}

void UAnimateComponent::syncAniInfoToSprite() {
  auto MeshPtr =
      getUiOwner()->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
          getUiOwner()->getComponent<USpriteComponent>()->getCompName());

  MeshPtr->MeshData.Textures[0] = CurrentAnimate->TexName;

  float StartX = 0.f;
  float StartY = 0.f;
  unsigned int MaxX = (int)(1.f / CurrentAnimate->Stride.x);
  MaxX = (((1.f / CurrentAnimate->Stride.x) - MaxX) > 0.5f) ? (MaxX + 1) : MaxX;
  StartX = (float)(CurrentAnimateCut % MaxX) * CurrentAnimate->Stride.x;
  StartY = (float)(CurrentAnimateCut / MaxX) * CurrentAnimate->Stride.y;
  DirectX::XMFLOAT4 UV = {StartX, StartY, StartX + CurrentAnimate->Stride.x,
                          StartY + CurrentAnimate->Stride.y};

  for (auto &Ins : MeshPtr->InstanceMap) {
    Ins.second.CustomizedData2 = UV;
    break;
  }
}
