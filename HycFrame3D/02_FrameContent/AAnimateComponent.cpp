#include "AAnimateComponent.h"

#include "AMeshComponent.h"
#include "ActorObject.h"
#include "AssetsPool.h"
#include "SceneNode.h"

AAnimateComponent::AAnimateComponent(const std::string &CompName,
                                     ActorObject *ActorOwner)
    : ActorComponent(CompName, ActorOwner), MeshAnimationDataPtr(nullptr),
      SubMeshNameVec({}), SubMeshBoneDataPtrVec({}), ShareBoneData(true),
      AnimationNames({}), CurrentAnimationInfo(nullptr),
      CurrentAnimationName(""), NextAnimationName(""),
      ResetTimeStampFlag(false), TotalTime(0.f), AnimationSpeedFactor(1.f) {}

AAnimateComponent::~AAnimateComponent() {}

bool
AAnimateComponent::init() {
  auto Amc = getActorOwner()->GetComponent<AMeshComponent>();
  if (!Amc) {
    return false;
  }

  const std::string &MeshName = Amc->MeshesNameArray[0];
  MeshAnimationDataPtr =
      getActorOwner()->GetSceneNode().GetAssetsPool()->getAnimationIfExisted(
          MeshName);
  if (!MeshAnimationDataPtr) {
    return false;
  }

  auto SubArray =
      getActorOwner()->GetSceneNode().GetAssetsPool()->getMeshIfExisted(
          MeshName);
#ifdef _DEBUG
  assert(SubArray);
#endif // _DEBUG
  for (const auto &SubMeshName : *SubArray) {
    auto SubMesh =
        getActorOwner()->GetSceneNode().GetAssetsPool()->getSubMeshIfExisted(
            SubMeshName);
#ifdef _DEBUG
    assert(SubMesh);
#endif // _DEBUG
    if (!SubMesh->MeshData.AnimationFlag) {
      P_LOG(LOG_ERROR, "this mesh doesn't have animation info : %s\n",
            MeshName.c_str());
      return false;
    }
    SubMesh->BonesMap.insert({getCompName(), SubMesh->OriginBoneData});
    SubMeshBoneDataPtrVec.push_back(&(SubMesh->BonesMap[getCompName()]));
    SubMeshNameVec.push_back(SubMeshName);
  }

  auto SubMeshSize = SubMeshBoneDataPtrVec.size();
  if (!SubMeshSize) {
    return false;
  }
  auto BoneSize = SubMeshBoneDataPtrVec[0]->size();
  if (SubMeshSize > 1) {
    for (size_t I = 0; I < BoneSize; I++) {
      if (SubMeshBoneDataPtrVec[0]->at(I).BoneName !=
          SubMeshBoneDataPtrVec[1]->at(I).BoneName) {
        ShareBoneData = false;
        break;
      }
    }
  }

  auto AniSize = MeshAnimationDataPtr->AllAnimations.size();
  if (!AniSize) {
    return false;
  }
  AnimationNames.resize(AniSize);
  for (size_t I = 0; I < AniSize; I++) {
    AnimationNames[I] = MeshAnimationDataPtr->AllAnimations[I].AnimationName;
  }
  if (NextAnimationName != "" && CurrentAnimationName != NextAnimationName) {
    int Index = 0;
    bool Found = false;
    for (auto &AniName : AnimationNames) {
      if (AniName == NextAnimationName) {
        Found = true;
        break;
      }
      ++Index;
    }
    if (!Found) {
      P_LOG(LOG_ERROR, "this animation name %s doesn't exist\n",
            NextAnimationName.c_str());
      NextAnimationName = "";
      return false;
    } else {
      CurrentAnimationName = NextAnimationName;
      NextAnimationName = "";
      CurrentAnimationInfo = &MeshAnimationDataPtr->AllAnimations[Index];
      ResetTimeStampFlag = true;
    }
  }

  return true;
}

void
AAnimateComponent::update(Timer &Timer) {
  if (NextAnimationName != "" && CurrentAnimationName != NextAnimationName) {
    int Index = 0;
    bool Found = false;
    for (auto &AniName : AnimationNames) {
      if (AniName == NextAnimationName) {
        Found = true;
        break;
      }
      ++Index;
    }
    if (!Found) {
      P_LOG(LOG_ERROR, "this animation name %s doesn't exist\n",
            NextAnimationName.c_str());
      NextAnimationName = "";
    } else {
      CurrentAnimationName = NextAnimationName;
      NextAnimationName = "";
      CurrentAnimationInfo = &MeshAnimationDataPtr->AllAnimations[Index];
      ResetTimeStampFlag = true;
    }
  }

  if (ResetTimeStampFlag) {
    TotalTime = 0.f;
    ResetTimeStampFlag = false;
  } else {
    TotalTime += Timer.floatDeltaTime() / 1000.f * AnimationSpeedFactor;
  }

  float AniTime = 0.f;
  {
    float Ticks = TotalTime * CurrentAnimationInfo->TicksPerSecond;
    float Duration = CurrentAnimationInfo->Duration;
    AniTime = fmodf(Ticks, Duration);
    if (AniTime < 0.f) {
      AniTime = Duration + AniTime;
    }
  }

  dx::XMFLOAT4X4 Identity = {};
  dx::XMMATRIX IdentityM = dx::XMMatrixIdentity();
  dx::XMStoreFloat4x4(&Identity, IdentityM);
  dx::XMFLOAT4X4 GlbInv = {};
  dx::XMMATRIX GlbInvM = {};
  GlbInvM = dx::XMLoadFloat4x4(&MeshAnimationDataPtr->RootNode->ThisToParent);
  {
    dx::XMVECTOR Det = dx::XMMatrixDeterminant(GlbInvM);
    GlbInvM = dx::XMMatrixInverse(&Det, GlbInvM);
    dx::XMStoreFloat4x4(&GlbInv, GlbInvM);
  }

  ProcessNodes(AniTime, MeshAnimationDataPtr->RootNode, Identity, GlbInv,
               CurrentAnimationInfo);

  if (SubMeshBoneDataPtrVec.size() != 1 && ShareBoneData) {
    auto BoneSize = SubMeshBoneDataPtrVec[0]->size();
    auto SubMSize = SubMeshBoneDataPtrVec.size();
    for (size_t MIndex = 1; MIndex < SubMSize; MIndex++) {
      for (size_t BIndex = 0; BIndex < BoneSize; BIndex++) {
        SubMeshBoneDataPtrVec[MIndex]->at(BIndex).BoneTransform =
            SubMeshBoneDataPtrVec[0]->at(BIndex).BoneTransform;
      }
    }
  }
}

void
AAnimateComponent::destory() {
  for (auto &SubMeshName : SubMeshNameVec) {
    SUBMESH_DATA *MeshPtr =
        getActorOwner()->GetSceneNode().GetAssetsPool()->getSubMeshIfExisted(
            SubMeshName);
    if (MeshPtr) {
      MeshPtr->BonesMap.erase(getCompName());
    }
  }
  SubMeshBoneDataPtrVec.clear();
  SubMeshNameVec.clear();
}

void
AAnimateComponent::changeAnimationTo(const std::string &AniName) {
  NextAnimationName = AniName;
}

void
AAnimateComponent::changeAnimationTo(int AniIndex) {
  if (static_cast<size_t>(AniIndex) >= AnimationNames.size()) {
    P_LOG(LOG_ERROR, "this animation index %d is overflow\n", AniIndex);
    return;
  }
  NextAnimationName = AnimationNames[AniIndex];
}

void
AAnimateComponent::ResetTimeStamp() {
  TotalTime = 0.f;
}

void
AAnimateComponent::SetSpeedFactor(float Factor) {
  AnimationSpeedFactor = Factor;
}

void
AAnimateComponent::ProcessNodes(float AniTime,
                                const MESH_NODE *Node,
                                const dx::XMFLOAT4X4 &ParentTrans,
                                const dx::XMFLOAT4X4 &GlbInvTrans,
                                const ANIMATION_INFO *const AniInfo) {
  const std::string &NodeName = Node->NodeName;
  dx::XMMATRIX NodeTrans = dx::XMLoadFloat4x4(&Node->ThisToParent);
  NodeTrans = dx::XMMatrixTranspose(NodeTrans);
  SUBMESH_BONE_DATA *BonePtr = nullptr;
  for (const auto &MeshBone : SubMeshBoneDataPtrVec) {
    for (auto &B : *MeshBone) {
      if (B.BoneName == NodeName) {
        BonePtr = &B;
        break;
      }
    }
    if (BonePtr) {
      break;
    }
  }
  const ANIMATION_CHANNEL *NodeActPtr = nullptr;
  for (const auto &Act : AniInfo->NodeActions) {
    if (NodeName == Act.NodeName) {
      NodeActPtr = &Act;
      break;
    }
  }

  if (NodeActPtr) {
    dx::XMVECTOR Sca = {};
    InterpSca(Sca, AniTime, NodeActPtr);
    dx::XMVECTOR Rot = {};
    InterpRot(Rot, AniTime, NodeActPtr);
    dx::XMVECTOR Pos = {};
    InterpPos(Pos, AniTime, NodeActPtr);

    dx::XMVECTOR Zero = dx::XMVectorSet(0.f, 0.f, 0.f, 1.f);
    NodeTrans = dx::XMMatrixAffineTransformation(Sca, Zero, Rot, Pos);
  }

  dx::XMMATRIX ParentGlb = dx::XMLoadFloat4x4(&ParentTrans);
  dx::XMMATRIX GlbTrans = NodeTrans * ParentGlb;
  dx::XMFLOAT4X4 ThisGlbTrans = {};
  dx::XMStoreFloat4x4(&ThisGlbTrans, GlbTrans);

  if (BonePtr) {
    dx::XMMATRIX BoneSpace = dx::XMLoadFloat4x4(&BonePtr->LocalToBone);
    BoneSpace = dx::XMMatrixTranspose(BoneSpace);
    dx::XMMATRIX FinalTrans = BoneSpace * GlbTrans;
    dx::XMStoreFloat4x4(&BonePtr->BoneTransform, FinalTrans);
  }

  for (const auto Child : Node->Children) {
    ProcessNodes(AniTime, Child, ThisGlbTrans, GlbInvTrans, AniInfo);
  }
}

void
AAnimateComponent::InterpPos(dx::XMVECTOR &OutResult,
                             float AniTime,
                             const ANIMATION_CHANNEL *const AniInfo) {
  auto Size = AniInfo->PositionKeys.size();
  assert(Size > 0);
  if (Size == 1) {
    OutResult = dx::XMLoadFloat3(&(AniInfo->PositionKeys[0].second));
    return;
  }

  size_t BaseIndex = 0;
  size_t NextIndex = 0;
  for (size_t I = 0, E = Size - 1; I < E; I++) {
    if (AniTime < AniInfo->PositionKeys[I + 1].first) {
      BaseIndex = I;
      NextIndex = BaseIndex + 1;
      assert(NextIndex < Size);
      break;
    }
  }

  float StartTime = AniInfo->PositionKeys[BaseIndex].first;
  float DeltaTime = AniInfo->PositionKeys[NextIndex].first - StartTime;
  float Factor = (AniTime - StartTime) / DeltaTime;
  assert(Factor >= 0.0f && Factor <= 1.0f);
  dx::XMVECTOR BasePos =
      dx::XMLoadFloat3(&AniInfo->PositionKeys[BaseIndex].second);
  dx::XMVECTOR NextPos =
      dx::XMLoadFloat3(&AniInfo->PositionKeys[NextIndex].second);
  OutResult = dx::XMVectorLerp(BasePos, NextPos, Factor);
}

void
AAnimateComponent::InterpRot(dx::XMVECTOR &OutResult,
                             float AniTime,
                             const ANIMATION_CHANNEL *const AniInfo) {
  auto Size = AniInfo->RotationKeys.size();
  assert(Size > 0);
  if (Size == 1) {
    OutResult = dx::XMLoadFloat4(&AniInfo->RotationKeys[0].second);
    OutResult = dx::XMQuaternionNormalize(OutResult);
    return;
  }

  size_t BaseIndex = 0;
  size_t NextIndex = 0;
  for (size_t I = 0, E = Size - 1; I < E; I++) {
    if (AniTime < AniInfo->RotationKeys[I + 1].first) {
      BaseIndex = I;
      NextIndex = BaseIndex + 1;
      assert(NextIndex < Size);
      break;
    }
  }

  float StartTime = AniInfo->RotationKeys[BaseIndex].first;
  float DeltaTime = AniInfo->RotationKeys[NextIndex].first - StartTime;
  float Factor = (AniTime - StartTime) / DeltaTime;
  assert(Factor >= 0.0f && Factor <= 1.0f);
  dx::XMVECTOR BaseRot =
      dx::XMLoadFloat4(&AniInfo->RotationKeys[BaseIndex].second);
  BaseRot = dx::XMQuaternionNormalize(BaseRot);
  dx::XMVECTOR NextRot =
      dx::XMLoadFloat4(&AniInfo->RotationKeys[NextIndex].second);
  NextRot = dx::XMQuaternionNormalize(NextRot);
  OutResult = dx::XMQuaternionSlerp(BaseRot, NextRot, Factor);
  OutResult = dx::XMQuaternionNormalize(OutResult);
}

void
AAnimateComponent::InterpSca(dx::XMVECTOR &OuResult,
                             float AniTime,
                             const ANIMATION_CHANNEL *const AniInfo) {
  auto Size = AniInfo->ScalingKeys.size();
  assert(Size > 0);
  if (Size == 1) {
    OuResult = dx::XMLoadFloat3(&(AniInfo->ScalingKeys[0].second));
    return;
  }

  size_t BaseIndex = 0;
  size_t NextIndex = 0;
  for (size_t I = 0, E = Size - 1; I < E; I++) {
    if (AniTime < AniInfo->ScalingKeys[I + 1].first) {
      BaseIndex = I;
      NextIndex = BaseIndex + 1;
      assert(NextIndex < Size);
      break;
    }
  }

  float StartTime = AniInfo->ScalingKeys[BaseIndex].first;
  float DeltaTime = AniInfo->ScalingKeys[NextIndex].first - StartTime;
  float Factor = (AniTime - StartTime) / DeltaTime;
  assert(Factor >= 0.0f && Factor <= 1.0f);
  dx::XMVECTOR BaseSca =
      dx::XMLoadFloat3(&AniInfo->ScalingKeys[BaseIndex].second);
  dx::XMVECTOR NextSca =
      dx::XMLoadFloat3(&AniInfo->ScalingKeys[NextIndex].second);
  OuResult = dx::XMVectorLerp(BaseSca, NextSca, Factor);
}
