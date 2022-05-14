#include "SPInput.h"
#include "RSRoot_DX11.h"
#include "RSPipelinesManager.h"

void RegisterSPInput(ObjectFactory* _factory)
{
#ifdef _DEBUG
    assert(_factory);
#endif // _DEBUG
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TestASpInput),TestASpInput });
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TempToTitle),TempToTitle });
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TempToSelect),TempToSelect });
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TempToRun),TempToRun });
    _factory->GetAInputMapPtr()->insert(
        { FUNC_NAME(TempToResult),TempToResult });
    _factory->GetAInitMapPtr()->insert(
        { FUNC_NAME(TestASpInit),TestASpInit });
    _factory->GetAUpdateMapPtr()->insert(
        { FUNC_NAME(TestASpUpdate),TestASpUpdate });
    _factory->GetADestoryMapPtr()->insert(
        { FUNC_NAME(TestASpDestory),TestASpDestory });
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(TestUSpInput),TestUSpInput });
    _factory->GetUInputMapPtr()->insert(
        { FUNC_NAME(TestUSpBtnInput),TestUSpBtnInput });
    _factory->GetUInitMapPtr()->insert(
        { FUNC_NAME(TestUSpInit),TestUSpInit });
    _factory->GetUUpdateMapPtr()->insert(
        { FUNC_NAME(TestUSpUpdate),TestUSpUpdate });
    _factory->GetUDestoryMapPtr()->insert(
        { FUNC_NAME(TestUSpDestory),TestUSpDestory });

    _factory->GetAInitMapPtr()->insert({ FUNC_NAME(AniInit),AniInit });
    _factory->GetAUpdateMapPtr()->insert({ FUNC_NAME(AniUpdate),AniUpdate });
    _factory->GetADestoryMapPtr()->insert({ FUNC_NAME(AniDestory),AniDestory });
}

void TestASpInput(AInputComponent* _aic, Timer& _timer)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test2\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("sample2-scene", "sample2-scene.json");
    }

    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        _aic->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            TranslateZAsix(0.1f * _timer.FloatDeltaTime());
    }
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        _aic->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            TranslateXAsix(-0.1f * _timer.FloatDeltaTime());
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        _aic->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            TranslateZAsix(-0.1f * _timer.FloatDeltaTime());
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        _aic->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            TranslateXAsix(0.1f * _timer.FloatDeltaTime());
    }
    if (InputInterface::IsKeyPushedInSingle(KB_P))
    {
        static bool simp = true;
        std::string basic = "light-pipeline";
        std::string simple = "simple-pipeline";
        if (simp)
        {
            GetRSRoot_DX11_Singleton()->PipelinesManager()->SetPipeline(basic);
        }
        else
        {
            GetRSRoot_DX11_Singleton()->PipelinesManager()->SetPipeline(simple);
        }
        simp = !simp;
    }
}

bool TestASpInit(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "a sp init\n");

    _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        StartTimer("timer1");

    return true;
}
void TestASpUpdate(AInteractComponent* _aitc, Timer&)
{
    //P_LOG(LOG_DEBUG, "a sp update\n");
    /*float time0 = _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        GetTimer("timer0")->mTime;
    float time1 = _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER)->
        GetTimer("timer1")->mTime;
    P_LOG(LOG_DEBUG, "timer0 : %f , timer1 : %f\n", time0, time1);*/

    CONTACT_PONT_PAIR contact = {};
    if (_aitc->GetActorOwner()->GetSceneNode().
        GetActorObject("sp-point-light-actor")->
        GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION)->
        CheckCollisionWith("sp-actor", &contact))
    {
        _aitc->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            RollBackPosition();
        P_LOG(LOG_DEBUG, "a : %f, %f, %f ; b : %f, %f, %f\n",
            contact.first.x, contact.first.y, contact.first.z,
            contact.second.x, contact.second.y, contact.second.z);
        auto center = ACollisionComponent::CalcCenterOfContact(contact);
        P_LOG(LOG_DEBUG, "center of contact : %f, %f, %f\n",
            center.x, center.y, center.z);
    }

    _aitc->GetActorOwner()->GetSceneNode().
        GetActorObject("sp-particle-actor")->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
        SetPosition(_aitc->GetActorOwner()->GetSceneNode().
            GetActorObject("sp-point-light-actor")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
            GetProcessingPosition());
    _aitc->GetActorOwner()->GetSceneNode().
        GetActorObject("sp-particle-actor")->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
        TranslateYAsix(5.f);
}

void TestASpDestory(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "a sp destory\n");
}

void TestUSpInput(UInputComponent* _uic, Timer& _timer)
{
    float delta = _timer.FloatDeltaTime();
    auto utc = _uic->GetUiOwner()->
        GetUComponent<UTransformComponent>(COMP_TYPE::U_TRANSFORM);

    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        utc->TranslateYAsix(0.1f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        utc->TranslateXAsix(-0.1f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        utc->TranslateYAsix(-0.1f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        utc->TranslateXAsix(0.1f * delta);
    }

    if (InputInterface::IsKeyPushedInSingle(KB_Z))
    {
        _uic->GetUiOwner()->
            GetUComponent<USpriteComponent>(COMP_TYPE::U_SPRITE)->
            ResetTexture();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_X))
    {
        _uic->GetUiOwner()->
            GetUComponent<UAnimateComponent>(COMP_TYPE::U_ANIMATE)->
            ChangeAnimateTo("number");
    }
    if (InputInterface::IsKeyPushedInSingle(KB_C))
    {
        _uic->GetUiOwner()->
            GetUComponent<UAnimateComponent>(COMP_TYPE::U_ANIMATE)->
            ChangeAnimateTo("runman");
    }

    if (InputInterface::IsKeyPushedInSingle(KB_N))
    {
        _uic->GetUiOwner()->
            GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlayBgm("test", 0.8f);
    }
    if (InputInterface::IsKeyPushedInSingle(KB_M))
    {
        _uic->GetUiOwner()->
            GetUComponent<UAudioComponent>(COMP_TYPE::U_AUDIO)->
            PlayBgm("test", 0.4f);
    }

    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test1\n");
        _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("sample1-scene", "sample1-scene.json");
    }
}

void TestUSpBtnInput(UInputComponent* _uic, Timer& _timer)
{
    auto ubc = _uic->GetUiOwner()->
        GetUComponent<UButtonComponent>(COMP_TYPE::U_BUTTON);
    if (!ubc) { return; }

    if (InputInterface::IsKeyPushedInSingle(KB_UP))
    {
        ubc->SelectUpBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_LEFT))
    {
        ubc->SelectLeftBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_DOWN))
    {
        ubc->SelectDownBtn();
    }
    if (InputInterface::IsKeyPushedInSingle(KB_RIGHT))
    {
        ubc->SelectRightBtn();
    }

    if (ubc->IsCursorOnBtn() && InputInterface::IsKeyPushedInSingle(M_LEFTBTN))
    {
        P_LOG(LOG_DEBUG, "this btn has been click : %s\n", ubc->GetCompName().c_str());
    }
}

bool TestUSpInit(UInteractComponent* _uitc)
{
    P_LOG(LOG_DEBUG, "u sp init\n");
    return true;
}

void TestUSpUpdate(UInteractComponent* _uitc, Timer& _timer)
{

}

void TestUSpDestory(UInteractComponent* _uitc)
{
    P_LOG(LOG_DEBUG, "u sp destory\n");
}

void TempToTitle(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to title\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("title-scene", "title-scene.json");
    }
}

void TempToSelect(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to select\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("select-scene", "select-scene.json");
    }
}

void TempToRun(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to run\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("run-scene", "run-scene.json");
    }
}

void TempToResult(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RCONTROL))
    {
        P_LOG(LOG_DEBUG, "to result\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("result-scene", "result-scene.json");
    }
}

static MESH_ANIMATION_DATA* g_RexAniData = nullptr;
static SUBMESH_BONES* g_RexBoneData = nullptr;
static int g_AniIndex = 0;
static std::vector<std::string> g_AniMap = {
    "run","bite","roar","attack_tail","idle" };
static float g_TotalTime = 0.f;

SUBMESH_BONES* TempGetBoneData()
{
    return g_RexBoneData;
}

bool AniInit(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "animate init\n");

    g_RexAniData = _aitc->GetActorOwner()->GetSceneNode().
        GetAssetsPool()->GetAnimationIfExisted("bonetest");
    if (!g_RexAniData) { return false; }

    g_RexBoneData = &(_aitc->GetActorOwner()->GetSceneNode().
        GetAssetsPool()->GetMeshIfExisted("bonetest")->mBoneData);
    if (!g_RexBoneData) { return false; }

    g_TotalTime = 0.f;

    return true;
}

void ProcessNodes(float _aniTime, const MESH_NODE* _node,
    const DirectX::XMFLOAT4X4& _parentTrans,
    const DirectX::XMFLOAT4X4& _glbInvTrans,
    const ANIMATION_INFO* const _aniInfo);

void CalcPos(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo);
void CalcRot(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo);
void CalcSca(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo);

void AniUpdate(AInteractComponent* _aitc, Timer& _timer)
{
    _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM)->
        RotateYAsix(_timer.FloatDeltaTime() / 1000.f);

    if (InputInterface::IsKeyPushedInSingle(KB_1))
    {
        g_AniIndex = 0;
        g_TotalTime = 0.f;
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_2))
    {
        g_AniIndex = 1;
        g_TotalTime = 0.f;
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_3))
    {
        g_AniIndex = 2;
        g_TotalTime = 0.f;
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_4))
    {
        g_AniIndex = 3;
        g_TotalTime = 0.f;
    }
    else if (InputInterface::IsKeyPushedInSingle(KB_5))
    {
        g_AniIndex = 4;
        g_TotalTime = 0.f;
    }

    g_TotalTime += _timer.FloatDeltaTime() / 1000.f;
    float aniTime = 0.f;
    ANIMATION_INFO* runAni = nullptr;
    for (auto& ani : g_RexAniData->mAllAnimations)
    {
        if (ani.mAnimationName != g_AniMap[g_AniIndex]) { continue; }
        runAni = &ani;
        float ticks = g_TotalTime * runAni->mTicksPerSecond * 50.f;
        float duration = runAni->mDuration;
        aniTime = fmodf(ticks, duration);
        break;
    }

    DirectX::XMFLOAT4X4 identity = {};
    DirectX::XMMATRIX identityM = DirectX::XMMatrixIdentity();
    DirectX::XMStoreFloat4x4(&identity, identityM);
    DirectX::XMFLOAT4X4 glbInv = {};
    DirectX::XMMATRIX glbInvM = {};
    glbInvM = DirectX::XMLoadFloat4x4(&g_RexAniData->mRootNode->mThisToParent);
    {
        DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(glbInvM);
        glbInvM = DirectX::XMMatrixInverse(&det, glbInvM);
        DirectX::XMStoreFloat4x4(&glbInv, glbInvM);
    }

    ProcessNodes(aniTime, g_RexAniData->mRootNode, identity, glbInv, runAni);
}

void AniDestory(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "animate destory\n");
    g_RexAniData = nullptr;
    g_RexBoneData = nullptr;
}

void ProcessNodes(float _aniTime, const MESH_NODE* _node,
    const DirectX::XMFLOAT4X4& _parentTrans,
    const DirectX::XMFLOAT4X4& _glbInvTrans,
    const ANIMATION_INFO* const _aniInfo)
{
    std::string nodeName = _node->mNodeName;
    DirectX::XMMATRIX nodeTrans = DirectX::XMLoadFloat4x4(
        &_node->mThisToParent);
    nodeTrans = DirectX::XMMatrixTranspose(nodeTrans);
    SUBMESH_BONE_DATA* bone = nullptr;
    for (auto& b : *g_RexBoneData)
    {
        if (b.mBoneName == nodeName) { bone = &b; break; }
    }
    const ANIMATION_CHANNEL* nodeAct = nullptr;
    for (auto& act : _aniInfo->mNodeActions)
    {
        if (nodeName == act.mNodeName) { nodeAct = &act; break; }
    }

    if (nodeAct)
    {
        DirectX::XMVECTOR sca = {};
        CalcSca(sca, _aniTime, nodeAct);
        DirectX::XMVECTOR rot = {};
        CalcRot(rot, _aniTime, nodeAct);
        DirectX::XMVECTOR pos = {};
        CalcPos(pos, _aniTime, nodeAct);

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
            &bone->mLocalToBone);
        boneSpace = DirectX::XMMatrixTranspose(boneSpace);
        DirectX::XMMATRIX glbInv = DirectX::XMLoadFloat4x4(&_glbInvTrans);
        DirectX::XMMATRIX finalTrans = boneSpace * glbTrans * glbInv;
        DirectX::XMStoreFloat4x4(&bone->mBoneTransform, finalTrans);
    }

    for (auto child : _node->mChildren)
    {
        ProcessNodes(_aniTime, child, thisGlbTrans, _glbInvTrans, _aniInfo);
    }
}

void CalcPos(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo)
{
    auto size = _aniInfo->mPositionKeys.size();
    assert(size > 0);
    if (size == 1)
    {
        _result = DirectX::XMLoadFloat3(&(_aniInfo->mPositionKeys[0].second));
        return;
    }

    size_t baseIndex = 0;
    size_t nextIndex = 0;
    for (size_t i = 0; i < (size - 1); i++)
    {
        if (_aniTime < _aniInfo->mPositionKeys[i + 1].first)
        {
            baseIndex = i;
            nextIndex = baseIndex + 1;
            assert(nextIndex < size);
            break;
        }
    }

    float startTime = _aniInfo->mPositionKeys[baseIndex].first;
    float deltaTime = _aniInfo->mPositionKeys[nextIndex].first - startTime;
    float factor = (_aniTime - startTime) / deltaTime;
    assert(factor >= 0.0f && factor <= 1.0f);
    DirectX::XMVECTOR basePos = DirectX::XMLoadFloat3(
        &_aniInfo->mPositionKeys[baseIndex].second);
    DirectX::XMVECTOR nextPos = DirectX::XMLoadFloat3(
        &_aniInfo->mPositionKeys[nextIndex].second);
    _result = DirectX::XMVectorLerp(basePos, nextPos, factor);
}

void CalcRot(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo)
{
    auto size = _aniInfo->mRotationKeys.size();
    assert(size > 0);
    if (size == 1)
    {
        DirectX::XMFLOAT4 q = {};
        q.x = _aniInfo->mRotationKeys[0].second.y;
        q.y = _aniInfo->mRotationKeys[0].second.z;
        q.z = _aniInfo->mRotationKeys[0].second.w;
        q.w = _aniInfo->mRotationKeys[0].second.x;
        _result = DirectX::XMLoadFloat4(&q);
        _result = DirectX::XMQuaternionNormalize(_result);
        return;
    }

    size_t baseIndex = 0;
    size_t nextIndex = 0;
    for (size_t i = 0; i < (size - 1); i++)
    {
        if (_aniTime < _aniInfo->mRotationKeys[i + 1].first)
        {
            baseIndex = i;
            nextIndex = baseIndex + 1;
            assert(nextIndex < size);
            break;
        }
    }

    float startTime = _aniInfo->mRotationKeys[baseIndex].first;
    float deltaTime = _aniInfo->mRotationKeys[nextIndex].first - startTime;
    float factor = (_aniTime - startTime) / deltaTime;
    assert(factor >= 0.0f && factor <= 1.0f);
    DirectX::XMFLOAT4 q = {};
    q.x = _aniInfo->mRotationKeys[baseIndex].second.y;
    q.y = _aniInfo->mRotationKeys[baseIndex].second.z;
    q.z = _aniInfo->mRotationKeys[baseIndex].second.w;
    q.w = _aniInfo->mRotationKeys[baseIndex].second.x;
    DirectX::XMVECTOR baseRot = DirectX::XMLoadFloat4(&q);
    baseRot = DirectX::XMQuaternionNormalize(baseRot);
    q.x = _aniInfo->mRotationKeys[nextIndex].second.y;
    q.y = _aniInfo->mRotationKeys[nextIndex].second.z;
    q.z = _aniInfo->mRotationKeys[nextIndex].second.w;
    q.w = _aniInfo->mRotationKeys[nextIndex].second.x;
    DirectX::XMVECTOR nextRot = DirectX::XMLoadFloat4(&q);
    nextRot = DirectX::XMQuaternionNormalize(nextRot);
    _result = DirectX::XMQuaternionSlerp(baseRot, nextRot, factor);
    _result = DirectX::XMQuaternionNormalize(_result);
}

void CalcSca(DirectX::XMVECTOR& _result, float _aniTime,
    const ANIMATION_CHANNEL* const _aniInfo)
{
    auto size = _aniInfo->mScalingKeys.size();
    assert(size > 0);
    if (size == 1)
    {
        _result = DirectX::XMLoadFloat3(&(_aniInfo->mScalingKeys[0].second));
        return;
    }

    size_t baseIndex = 0;
    size_t nextIndex = 0;
    for (size_t i = 0; i < (size - 1); i++)
    {
        if (_aniTime < _aniInfo->mScalingKeys[i + 1].first)
        {
            baseIndex = i;
            nextIndex = baseIndex + 1;
            assert(nextIndex < size);
            break;
        }
    }

    float startTime = _aniInfo->mScalingKeys[baseIndex].first;
    float deltaTime = _aniInfo->mScalingKeys[nextIndex].first - startTime;
    float factor = (_aniTime - startTime) / deltaTime;
    assert(factor >= 0.0f && factor <= 1.0f);
    DirectX::XMVECTOR baseSca = DirectX::XMLoadFloat3(
        &_aniInfo->mScalingKeys[baseIndex].second);
    DirectX::XMVECTOR nextSca = DirectX::XMLoadFloat3(
        &_aniInfo->mScalingKeys[nextIndex].second);
    _result = DirectX::XMVectorLerp(baseSca, nextSca, factor);
}
