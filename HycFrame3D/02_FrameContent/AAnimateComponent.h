#pragma once

#include "ActorComponent.h"
#include <vector>
#include "ModelHelper.h"

class AAnimateComponent :public ActorComponent
{
public:
    AAnimateComponent(std::string&& _compName, class ActorObject* _actorOwner);
    AAnimateComponent(std::string& _compName, class ActorObject* _actorOwner);
    virtual ~AAnimateComponent();

public:
    virtual bool Init();
    virtual void Update(Timer& _timer);
    virtual void Destory();

public:
    void ChangeAnimationTo(std::string& _aniName);
    void ChangeAnimationTo(std::string&& _aniName);
    void ChangeAnimationTo(int _aniIndex);

    void ResetTimeStamp();
    void SetSpeedFactor(float _factor);
    SUBMESH_BONES* GetBonesData()
    {
        return mSubMeshBoneDataPtr;
    }

private:
    void ProcessNodes(float _aniTime, const MESH_NODE* _node,
        const DirectX::XMFLOAT4X4& _parentTrans,
        const DirectX::XMFLOAT4X4& _glbInvTrans,
        const ANIMATION_INFO* const _aniInfo);

    void InterpPos(DirectX::XMVECTOR& _result, float _aniTime,
        const ANIMATION_CHANNEL* const _aniInfo);
    void InterpRot(DirectX::XMVECTOR& _result, float _aniTime,
        const ANIMATION_CHANNEL* const _aniInfo);
    void InterpSca(DirectX::XMVECTOR& _result, float _aniTime,
        const ANIMATION_CHANNEL* const _aniInfo);

private:
    MESH_ANIMATION_DATA* mMeshAnimationDataPtr;
    SUBMESH_BONES* mSubMeshBoneDataPtr;
    std::vector<std::string> mAnimationNames;
    
    ANIMATION_INFO* mCurrentAnimationInfo;
    std::string mCurrentAnimationName;
    std::string mNextAnimationName;
    bool mResetTimeStampFlag;

    float mTotalTime;
    float mAnimationSpeedFactor;
};
