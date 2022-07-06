#pragma once

#include "ActorComponent.h"

#include "ModelHelper.h"

#include <vector>

class AAnimateComponent : public ActorComponent {
private:
  MESH_ANIMATION_DATA *MeshAnimationDataPtr;
  std::vector<std::string> SubMeshNameVec;
  std::vector<SUBMESH_BONES *> SubMeshBoneDataPtrVec;
  bool ShareBoneData;
  std::vector<std::string> AnimationNames;

  ANIMATION_INFO *CurrentAnimationInfo;
  std::string CurrentAnimationName;
  std::string NextAnimationName;
  bool ResetTimeStampFlag;

  float TotalTime;
  float AnimationSpeedFactor;

public:
  AAnimateComponent(const std::string &CompName, class ActorObject *ActorOwner);
  virtual ~AAnimateComponent();

  AAnimateComponent &
  operator=(const AAnimateComponent &Source) {
    if (this == &Source) {
      return *this;
    }
    MeshAnimationDataPtr = Source.MeshAnimationDataPtr;
    SubMeshNameVec = Source.SubMeshNameVec;
    SubMeshBoneDataPtrVec = Source.SubMeshBoneDataPtrVec;
    ShareBoneData = Source.ShareBoneData;
    AnimationNames = Source.AnimationNames;
    CurrentAnimationInfo = Source.CurrentAnimationInfo;
    CurrentAnimationName = Source.CurrentAnimationName;
    NextAnimationName = Source.NextAnimationName;
    ResetTimeStampFlag = Source.ResetTimeStampFlag;
    TotalTime = Source.TotalTime;
    AnimationSpeedFactor = Source.AnimationSpeedFactor;
    ActorComponent::operator=(Source);
    return *this;
  }

public:
  virtual bool
  init();
  virtual void
  update(Timer &Timer);
  virtual void
  destory();

public:
  void
  changeAnimationTo(const std::string &AniName);
  void
  changeAnimationTo(int AniIndex);

  void
  resetTimeStamp();
  void
  SetSpeedFactor(float Factor);

private:
  void
  processNodes(float AniTime,
               const MESH_NODE *Node,
               const dx::XMFLOAT4X4 &ParentTrans,
               const dx::XMFLOAT4X4 &GlbInvTrans,
               const ANIMATION_INFO *const AniInfo);

  void
  interpolatePosition(dx::XMVECTOR &OutResult,
                      float AniTime,
                      const ANIMATION_CHANNEL *const AniInfo);
  void
  interpolateRotation(dx::XMVECTOR &OutResult,
                      float AniTime,
                      const ANIMATION_CHANNEL *const AniInfo);
  void
  interpolateScaling(dx::XMVECTOR &OutResult,
                     float AniTime,
                     const ANIMATION_CHANNEL *const AniInfo);
};
