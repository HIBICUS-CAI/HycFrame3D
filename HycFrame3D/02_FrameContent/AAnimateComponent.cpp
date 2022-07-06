#include "AAnimateComponent.h"
#include "ActorObject.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "AMeshComponent.h"

AAnimateComponent::AAnimateComponent(std::string&& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mMeshAnimationDataPtr(nullptr),
    mSubMeshNameVec({}),
    mSubMeshBoneDataPtrVec({}),
    mShareBoneData(true),
    mAnimationNames({}),
    mCurrentAnimationInfo(nullptr),
    mCurrentAnimationName(""),
    mNextAnimationName(""),
    mResetTimeStampFlag(false),
    mTotalTime(0.f),
    mAnimationSpeedFactor(1.f)
{

}

AAnimateComponent::AAnimateComponent(std::string& _compName,
    ActorObject* _actorOwner) :
    ActorComponent(_compName, _actorOwner),
    mMeshAnimationDataPtr(nullptr),
    mSubMeshNameVec({}),
    mSubMeshBoneDataPtrVec({}),
    mShareBoneData(true),
    mAnimationNames({}),
    mCurrentAnimationInfo(nullptr),
    mCurrentAnimationName(""),
    mNextAnimationName(""),
    mResetTimeStampFlag(false),
    mTotalTime(0.f),
    mAnimationSpeedFactor(1.f)
{

}

AAnimateComponent::~AAnimateComponent()
{

}

bool AAnimateComponent::Init()
{
    auto amc = GetActorOwner()->GetComponent<AMeshComponent>();
    if (!amc) { return false; }

    std::string meshName = amc->mMeshesName[0];
    mMeshAnimationDataPtr = GetActorOwner()->GetSceneNode().
        GetAssetsPool()->GetAnimationIfExisted(meshName);
    if (!mMeshAnimationDataPtr) { return false; }

    auto subVec = GetActorOwner()->GetSceneNode().
        GetAssetsPool()->GetMeshIfExisted(meshName);
#ifdef _DEBUG
    assert(subVec);
#endif // _DEBUG
    for (auto& subMeshName : *subVec)
    {
        auto mesh = GetActorOwner()->GetSceneNode().
            GetAssetsPool()->GetSubMeshIfExisted(subMeshName);
#ifdef _DEBUG
        assert(mesh);
#endif // _DEBUG
        if (!mesh->mMeshData.AnimationFlag)
        {
            P_LOG(LOG_ERROR, "this mesh doesn't have animation info : %s\n",
                meshName.c_str());
            return false;
        }
        mesh->mBonesMap.insert({ GetCompName(),mesh->mOriginBoneData });
        mSubMeshBoneDataPtrVec.push_back(&(mesh->mBonesMap[GetCompName()]));
        mSubMeshNameVec.push_back(subMeshName);
    }

    auto subMeshSize = mSubMeshBoneDataPtrVec.size();
    if (!subMeshSize) { return false; }
    auto boneSize = mSubMeshBoneDataPtrVec[0]->size();
    if (subMeshSize > 1)
    {
        for (size_t i = 0; i < boneSize; i++)
        {
            if (mSubMeshBoneDataPtrVec[0]->at(i).BoneName !=
                mSubMeshBoneDataPtrVec[1]->at(i).BoneName)
            {
                mShareBoneData = false;
                break;
            }
        }
    }

    auto aniSize = mMeshAnimationDataPtr->AllAnimations.size();
    if (!aniSize) { return false; }
    mAnimationNames.resize(aniSize);
    for (size_t i = 0; i < aniSize; i++)
    {
        mAnimationNames[i] = mMeshAnimationDataPtr->
            AllAnimations[i].AnimationName;
    }
    if (mNextAnimationName != "" && mCurrentAnimationName != mNextAnimationName)
    {
        int index = 0;
        bool found = false;
        for (auto& aniName : mAnimationNames)
        {
            if (aniName == mNextAnimationName)
            {
                found = true;
                break;
            }
            ++index;
        }
        if (!found)
        {
            P_LOG(LOG_ERROR, "this animation name %s doesn't exist\n",
                mNextAnimationName.c_str());
            mNextAnimationName = "";
            return false;
        }
        else
        {
            mCurrentAnimationName = mNextAnimationName;
            mNextAnimationName = "";
            mCurrentAnimationInfo = &mMeshAnimationDataPtr->AllAnimations[index];
            mResetTimeStampFlag = true;
        }
    }

    return true;
}

void AAnimateComponent::Update(Timer& _timer)
{
    if (mNextAnimationName != "" && mCurrentAnimationName != mNextAnimationName)
    {
        int index = 0;
        bool found = false;
        for (auto& aniName : mAnimationNames)
        {
            if (aniName == mNextAnimationName)
            {
                found = true;
                break;
            }
            ++index;
        }
        if (!found)
        {
            P_LOG(LOG_ERROR, "this animation name %s doesn't exist\n",
                mNextAnimationName.c_str());
            mNextAnimationName = "";
        }
        else
        {
            mCurrentAnimationName = mNextAnimationName;
            mNextAnimationName = "";
            mCurrentAnimationInfo = &mMeshAnimationDataPtr->AllAnimations[index];
            mResetTimeStampFlag = true;
        }
    }

    if (mResetTimeStampFlag)
    {
        mTotalTime = 0.f;
        mResetTimeStampFlag = false;
    }
    else
    {
        mTotalTime += _timer.floatDeltaTime() / 1000.f * mAnimationSpeedFactor;
    }

    float aniTime = 0.f;
    {
        float ticks = mTotalTime * mCurrentAnimationInfo->TicksPerSecond;
        float duration = mCurrentAnimationInfo->Duration;
        aniTime = fmodf(ticks, duration);
        if (aniTime < 0.f) { aniTime = duration + aniTime; }
    }

    DirectX::XMFLOAT4X4 identity = {};
    DirectX::XMMATRIX identityM = DirectX::XMMatrixIdentity();
    DirectX::XMStoreFloat4x4(&identity, identityM);
    DirectX::XMFLOAT4X4 glbInv = {};
    DirectX::XMMATRIX glbInvM = {};
    glbInvM = DirectX::XMLoadFloat4x4(&mMeshAnimationDataPtr->
        RootNode->ThisToParent);
    {
        DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(glbInvM);
        glbInvM = DirectX::XMMatrixInverse(&det, glbInvM);
        DirectX::XMStoreFloat4x4(&glbInv, glbInvM);
    }

    ProcessNodes(aniTime, mMeshAnimationDataPtr->RootNode, identity,
        glbInv, mCurrentAnimationInfo);

    if (mSubMeshBoneDataPtrVec.size() != 1 && mShareBoneData)
    {
        auto boneSize = mSubMeshBoneDataPtrVec[0]->size();
        auto submSize = mSubMeshBoneDataPtrVec.size();
        for (size_t mIndex = 1; mIndex < submSize; mIndex++)
        {
            for (size_t bIndex = 0; bIndex < boneSize; bIndex++)
            {
                mSubMeshBoneDataPtrVec[mIndex]->at(bIndex).BoneTransform =
                    mSubMeshBoneDataPtrVec[0]->at(bIndex).BoneTransform;
            }
        }
    }
}

void AAnimateComponent::Destory()
{
    for (auto& subMeshName : mSubMeshNameVec)
    {
        SUBMESH_DATA* mesh = GetActorOwner()->GetSceneNode().GetAssetsPool()->
            GetSubMeshIfExisted(subMeshName);
        if (mesh) { mesh->mBonesMap.erase(GetCompName()); }
    }
    mSubMeshBoneDataPtrVec.clear();
    mSubMeshNameVec.clear();
}

void AAnimateComponent::ChangeAnimationTo(std::string& _aniName)
{
    mNextAnimationName = _aniName;
}

void AAnimateComponent::ChangeAnimationTo(std::string&& _aniName)
{
    mNextAnimationName = _aniName;
}

void AAnimateComponent::ChangeAnimationTo(int _aniIndex)
{
    if ((size_t)_aniIndex >= mAnimationNames.size())
    {
        P_LOG(LOG_ERROR, "this animation index %d is overflow\n",
            _aniIndex);
        return;
    }
    mNextAnimationName = mAnimationNames[_aniIndex];
}

void AAnimateComponent::ResetTimeStamp()
{
    mTotalTime = 0.f;
}

void AAnimateComponent::SetSpeedFactor(float _factor)
{
    mAnimationSpeedFactor = _factor;
}

void AAnimateComponent::ProcessNodes(float _aniTime, const MESH_NODE* _node,
    const DirectX::XMFLOAT4X4& _parentTrans,
    const DirectX::XMFLOAT4X4& _glbInvTrans,
    const ANIMATION_INFO* const _aniInfo)
{
    std::string nodeName = _node->NodeName;
    DirectX::XMMATRIX nodeTrans = DirectX::XMLoadFloat4x4(
        &_node->ThisToParent);
    nodeTrans = DirectX::XMMatrixTranspose(nodeTrans);
    SUBMESH_BONE_DATA* bone = nullptr;
    for (auto& mb : mSubMeshBoneDataPtrVec)
    {
        for (auto& b : *mb)
        {
            if (b.BoneName == nodeName) { bone = &b; break; }
        }
        if (bone) { break; }
    }
    const ANIMATION_CHANNEL* nodeAct = nullptr;
    for (auto& act : _aniInfo->NodeActions)
    {
        if (nodeName == act.NodeName) { nodeAct = &act; break; }
    }

    if (nodeAct)
    {
        DirectX::XMVECTOR sca = {};
        InterpSca(sca, _aniTime, nodeAct);
        DirectX::XMVECTOR rot = {};
        InterpRot(rot, _aniTime, nodeAct);
        DirectX::XMVECTOR pos = {};
        InterpPos(pos, _aniTime, nodeAct);

        DirectX::XMVECTOR zero = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
        nodeTrans = DirectX::XMMatrixAffineTransformation(sca, zero, rot, pos);
    }

    DirectX::XMMATRIX parentGlb = DirectX::XMLoadFloat4x4(&_parentTrans);
    DirectX::XMMATRIX glbTrans = nodeTrans * parentGlb;
    DirectX::XMFLOAT4X4 thisGlbTrans = {};
    DirectX::XMStoreFloat4x4(&thisGlbTrans, glbTrans);

    if (bone)
    {
        DirectX::XMMATRIX boneSpace = DirectX::XMLoadFloat4x4(
            &bone->LocalToBone);
        boneSpace = DirectX::XMMatrixTranspose(boneSpace);
        DirectX::XMMATRIX finalTrans = boneSpace * glbTrans;
        DirectX::XMStoreFloat4x4(&bone->BoneTransform, finalTrans);
    }

    for (auto child : _node->Children)
    {
        ProcessNodes(_aniTime, child, thisGlbTrans, _glbInvTrans, _aniInfo);
    }
}

void AAnimateComponent::InterpPos(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo)
{
    auto size = _aniInfo->PositionKeys.size();
    assert(size > 0);
    if (size == 1)
    {
        _result = DirectX::XMLoadFloat3(&(_aniInfo->PositionKeys[0].second));
        return;
    }

    size_t baseIndex = 0;
    size_t nextIndex = 0;
    for (size_t i = 0; i < (size - 1); i++)
    {
        if (_aniTime < _aniInfo->PositionKeys[i + 1].first)
        {
            baseIndex = i;
            nextIndex = baseIndex + 1;
            assert(nextIndex < size);
            break;
        }
    }

    float startTime = _aniInfo->PositionKeys[baseIndex].first;
    float deltaTime = _aniInfo->PositionKeys[nextIndex].first - startTime;
    float factor = (_aniTime - startTime) / deltaTime;
    assert(factor >= 0.0f && factor <= 1.0f);
    DirectX::XMVECTOR basePos = DirectX::XMLoadFloat3(
        &_aniInfo->PositionKeys[baseIndex].second);
    DirectX::XMVECTOR nextPos = DirectX::XMLoadFloat3(
        &_aniInfo->PositionKeys[nextIndex].second);
    _result = DirectX::XMVectorLerp(basePos, nextPos, factor);
}

void AAnimateComponent::InterpRot(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo)
{
    auto size = _aniInfo->RotationKeys.size();
    assert(size > 0);
    if (size == 1)
    {
        _result = DirectX::XMLoadFloat4(
            &_aniInfo->RotationKeys[0].second);
        _result = DirectX::XMQuaternionNormalize(_result);
        return;
    }

    size_t baseIndex = 0;
    size_t nextIndex = 0;
    for (size_t i = 0; i < (size - 1); i++)
    {
        if (_aniTime < _aniInfo->RotationKeys[i + 1].first)
        {
            baseIndex = i;
            nextIndex = baseIndex + 1;
            assert(nextIndex < size);
            break;
        }
    }

    float startTime = _aniInfo->RotationKeys[baseIndex].first;
    float deltaTime = _aniInfo->RotationKeys[nextIndex].first - startTime;
    float factor = (_aniTime - startTime) / deltaTime;
    assert(factor >= 0.0f && factor <= 1.0f);
    DirectX::XMVECTOR baseRot = DirectX::XMLoadFloat4(
        &_aniInfo->RotationKeys[baseIndex].second);
    baseRot = DirectX::XMQuaternionNormalize(baseRot);
    DirectX::XMVECTOR nextRot = DirectX::XMLoadFloat4(
        &_aniInfo->RotationKeys[nextIndex].second);
    nextRot = DirectX::XMQuaternionNormalize(nextRot);
    _result = DirectX::XMQuaternionSlerp(baseRot, nextRot, factor);
    _result = DirectX::XMQuaternionNormalize(_result);
}

void AAnimateComponent::InterpSca(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo)
{
    auto size = _aniInfo->ScalingKeys.size();
    assert(size > 0);
    if (size == 1)
    {
        _result = DirectX::XMLoadFloat3(&(_aniInfo->ScalingKeys[0].second));
        return;
    }

    size_t baseIndex = 0;
    size_t nextIndex = 0;
    for (size_t i = 0; i < (size - 1); i++)
    {
        if (_aniTime < _aniInfo->ScalingKeys[i + 1].first)
        {
            baseIndex = i;
            nextIndex = baseIndex + 1;
            assert(nextIndex < size);
            break;
        }
    }

    float startTime = _aniInfo->ScalingKeys[baseIndex].first;
    float deltaTime = _aniInfo->ScalingKeys[nextIndex].first - startTime;
    float factor = (_aniTime - startTime) / deltaTime;
    assert(factor >= 0.0f && factor <= 1.0f);
    DirectX::XMVECTOR baseSca = DirectX::XMLoadFloat3(
        &_aniInfo->ScalingKeys[baseIndex].second);
    DirectX::XMVECTOR nextSca = DirectX::XMLoadFloat3(
        &_aniInfo->ScalingKeys[nextIndex].second);
    _result = DirectX::XMVectorLerp(baseSca, nextSca, factor);
}
