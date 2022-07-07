#include "AMeshComponent.h"

#include "ATransformComponent.h"
#include "ActorComponent.h"
#include "ActorObject.h"
#include "AssetsPool.h"
#include "SceneNode.h"

#include <set>

AMeshComponent::AMeshComponent(const std::string &CompName,
                               ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), MeshesNameArray({}),
      SubMeshesNameArray({}), OffsetPositionArray({}), EmissiveIntensity(0.f) {}

AMeshComponent::~AMeshComponent() {}

bool AMeshComponent::init() {
  int MeshIndex = 0;
  std::vector<DirectX::XMFLOAT3> *TempOffset =
      new std::vector<DirectX::XMFLOAT3>;
  if (!TempOffset) {
    return false;
  }
  TempOffset->reserve(256);
  for (const auto &MeshName : MeshesNameArray) {
    auto SubArray =
        getActorOwner()->getSceneNode().getAssetsPool()->getMeshIfExisted(
            MeshName);
#ifdef _DEBUG
    assert(SubArray);
#endif // _DEBUG
    auto SubSize = SubArray->size();
    for (size_t I = 0; I < SubSize; I++) {
      SubMeshesNameArray.push_back((*SubArray)[I]);
      TempOffset->push_back(OffsetPositionArray[MeshIndex]);
    }
    ++MeshIndex;
  }
  auto AllOffset = TempOffset->size();
  OffsetPositionArray.clear();
  OffsetPositionArray.resize(AllOffset);
  for (size_t I = 0; I < AllOffset; I++) {
    OffsetPositionArray[I] = TempOffset->at(I);
  }
  TempOffset->clear();
  delete TempOffset;

  for (const auto &MeshName : SubMeshesNameArray) {
    if (!bindInstanceToAssetsPool(MeshName)) {
      return false;
    }
  }

  return true;
}

void AMeshComponent::update(const Timer &Timer) { syncTransformDataToInstance(); }

void AMeshComponent::destory() {
  for (const auto &MeshName : SubMeshesNameArray) {
    SUBMESH_DATA *MeshPtr =
        getActorOwner()->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
            MeshName);
    if (MeshPtr) {
      MeshPtr->InstanceMap.erase(getCompName());
    }
  }
}

void AMeshComponent::addMeshInfo(const std::string &MeshName,
                                 DirectX::XMFLOAT3 Offset) {
  MeshesNameArray.push_back(MeshName);
  OffsetPositionArray.push_back(Offset);
}

void AMeshComponent::setEmissiveIntensity(float Intensity) {
  EmissiveIntensity = Intensity;
  if (EmissiveIntensity > 255.f) {
    EmissiveIntensity = 255.f;
  }
}

float AMeshComponent::getEmissiveIntensity() { return EmissiveIntensity; }

bool AMeshComponent::bindInstanceToAssetsPool(const std::string &MeshName) {
  SUBMESH_DATA *MeshPtr =
      getActorOwner()->getSceneNode().getAssetsPool()->getSubMeshIfExisted(
          MeshName);
  if (!MeshPtr) {
    return false;
  }

  RS_INSTANCE_DATA InsData = {};
  InsData.MaterialData = MeshPtr->MeshData.Material;
  if (MeshPtr->MeshData.Textures[1] != "") {
    InsData.CustomizedData1.x = 1.f;
  } else {
    InsData.CustomizedData1.x = -1.f;
  }
  if (MeshPtr->MeshData.Textures[2] != "") {
    InsData.CustomizedData1.y = 1.f;
  } else {
    InsData.CustomizedData1.y = -1.f;
  }
  if (MeshPtr->MeshData.Textures[3] != "") {
    InsData.CustomizedData1.z = 1.f;
  } else {
    InsData.CustomizedData1.z = -1.f;
  }
  if (MeshPtr->MeshData.Textures[4] != "") {
    InsData.CustomizedData1.w = 1.f;
  } else {
    InsData.CustomizedData1.w = -1.f;
  }
  if (MeshPtr->MeshData.Textures[4] != "") {
    InsData.CustomizedData2.x = EmissiveIntensity;
  } else {
    InsData.CustomizedData2.x = 0.f;
  }

  MeshPtr->InstanceMap.insert({getCompName(), InsData});

  return true;
}

void AMeshComponent::syncTransformDataToInstance() {
  ATransformComponent *Atc =
      getActorOwner()->getComponent<ATransformComponent>();
#ifdef _DEBUG
  assert(Atc);
#endif // _DEBUG
  size_t Index = 0;
  std::set<std::string> HasChecked = {};

  for (const auto &SubMeshName : SubMeshesNameArray) {
    if (HasChecked.find(SubMeshName) != HasChecked.end()) {
      continue;
    }

    auto &InsMap = getActorOwner()
                       ->getSceneNode()
                       .getAssetsPool()
                       ->getSubMeshIfExisted(SubMeshName)
                       ->InstanceMap;
    std::pair<std::unordered_multimap<std::string, RS_INSTANCE_DATA>::iterator,
              std::unordered_multimap<std::string, RS_INSTANCE_DATA>::iterator>
        MyRange;
    MyRange = InsMap.equal_range(getCompName());

    for (auto &It = MyRange.first, &End = MyRange.second; It != End; ++It) {
      auto &InsData = It->second;
      DirectX::XMFLOAT3 Delta = OffsetPositionArray[Index];
      DirectX::XMFLOAT3 World = Atc->getPosition();
      DirectX::XMFLOAT3 Angle = Atc->getRotation();
      DirectX::XMFLOAT3 Scale = Atc->getScaling();

      DirectX::XMMATRIX Matrix = {};
      Matrix = DirectX::XMMatrixMultiply(
          DirectX::XMMatrixTranslation(Delta.x, Delta.y, Delta.z),
          DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z));
      Matrix = DirectX::XMMatrixMultiply(
          Matrix,
          DirectX::XMMatrixRotationRollPitchYaw(Angle.x, Angle.y, Angle.z));
      Matrix = DirectX::XMMatrixMultiply(
          Matrix, DirectX::XMMatrixTranslation(World.x, World.y, World.z));
      DirectX::XMStoreFloat4x4(&(InsData.WorldMatrix), Matrix);

      InsData.CustomizedData2.x = EmissiveIntensity;

      ++Index;
    }

    HasChecked.insert(SubMeshName);
  }
}
