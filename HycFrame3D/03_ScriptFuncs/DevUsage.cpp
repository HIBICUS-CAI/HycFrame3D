#include "DevUsage.h"
#include "ModelHelper.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "ActorObject.h"
#include "UiObject.h"
#include "ID_Interface.h"
#include "RSRoot_DX11.h"
#include "RSDevices.h"
#include "RSResourceManager.h"
#include "RSMeshHelper.h"
#include "RSStaticResources.h"
#include "ComponentContainer.h"
#include "ATransformComponent.h"
#include "UAnimateComponent.h"
#include "AInputComponent.h"
#include "AInteractComponent.h"
#include "AMeshComponent.h"
#include "ALightComponent.h"

void TestAInput(AInputComponent*, Timer&);
void TestA1Input(AInputComponent*, Timer&);

bool TestAInit(AInteractComponent*);
void TestAUpdate(AInteractComponent*, Timer&);
void TestADestory(AInteractComponent*);

void DevUsage(SceneNode* _node)
{
    RS_SUBMESH_DATA sd = {};
    LoadModelFile(".\\Assets\\Models\\Dragon.FBX.json",
        MODEL_FILE_TYPE::JSON, &sd);
    _node->GetAssetsPool()->InsertNewMesh("dragon", sd, MESH_TYPE::OPACITY);

    sd = GetRSRoot_DX11_Singleton()->MeshHelper()->GeoGenerate()->
        CreateGrid(500.f, 500.f, 10, 10, LAYOUT_TYPE::NORMAL_TANGENT_TEX,
            false, {}, "tile.dds");
    std::string name = "copper";
    sd.mMaterial = *(GetRSRoot_DX11_Singleton()->StaticResources()->
        GetStaticMaterial(name));
    AddBumpedTexTo(&sd, "tile_nmap.dds");
    _node->GetAssetsPool()->InsertNewMesh("floor", sd, MESH_TYPE::OPACITY);

    ActorObject a0("a0", *_node);
    ATransformComponent atc0("a0-transform", nullptr);
    AInputComponent aic0("a0-input", nullptr);
    AInteractComponent aitc0("a0-interact", nullptr);
    AMeshComponent amc0("a0-mesh", nullptr);

    atc0.ForcePosition({ 0.f,0.f,150.f });
    atc0.ForceRotation({ 0.f,0.f,0.f });
    atc0.ForceScaling({ 1.f,1.f,1.f });
    a0.AddAComponent(COMP_TYPE::A_TRANSFORM);

    aic0.SetInputFunction(TestAInput);
    a0.AddAComponent(COMP_TYPE::A_INPUT);

    aitc0.SetInitFunction(TestAInit);
    aitc0.SetUpdateFunction(TestAUpdate);
    aitc0.SetDestoryFunction(TestADestory);
    a0.AddAComponent(COMP_TYPE::A_INTERACT);

    amc0.AddMeshInfo("dragon");
    amc0.AddMeshInfo("floor", { 0.f,-20.f,0.f });
    amc0.AddMeshInfo("floor", { 0.f,-50.f,0.f });
    a0.AddAComponent(COMP_TYPE::A_MESH);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INTERACT, aitc0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_MESH, amc0);

    _node->AddActorObject(a0);

    ActorObject a1("a1", *_node);
    ATransformComponent atc1("a1-transform", nullptr);
    AInputComponent aic1("a1-input", nullptr);
    ALightComponent alc1("a1-light", nullptr);

    atc1.ForcePosition({ 0.f,0.f,75.f });
    atc1.ForceRotation({ 0.f,0.f,0.f });
    atc1.ForceScaling({ 3.f,3.f,3.f });
    a1.AddAComponent(COMP_TYPE::A_TRANSFORM);

    aic1.SetInputFunction(TestA1Input);
    a1.AddAComponent(COMP_TYPE::A_INPUT);

    LIGHT_INFO li = {};
    li.mType = LIGHT_TYPE::POINT;
    li.mWithShadow = false;
    li.mPosition = { 0.f,0.f,0.f };
    li.mDirection = { 0.f,-0.f,0.f };
    li.mStrength = { 0.9f,0.f,0.9f };
    li.mSpotPower = 2.f;
    li.mFalloffStart = 30.f;
    li.mFalloffEnd = 80.f;
    alc1.AddLight(li, true, false, {});
    a1.AddAComponent(COMP_TYPE::A_LIGHT);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc1);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic1);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_LIGHT, alc1);

    _node->AddActorObject(a1);

    ActorObject a2("a2", *_node);
    ATransformComponent atc2("a2-transform", nullptr);
    ALightComponent alc2("a2-light", nullptr);
    //AInputComponent aic2("a2-input", nullptr);

    atc2.ForcePosition({ 0.f,30.f,-30.f });
    atc2.ForceRotation({ 0.f,0.f,0.f });
    atc2.ForceScaling({ 1.f,1.f,1.f });
    a2.AddAComponent(COMP_TYPE::A_TRANSFORM);

    //aic2.SetInputFunction(TestA1Input);
    //a2.AddAComponent(COMP_TYPE::A_INPUT);

    li = {};
    li.mType = LIGHT_TYPE::DIRECT;
    li.mWithShadow = true;
    li.mPosition = { 0.f,30.f,-30.f };
    li.mDirection = { 0.f,-1.f,1.f };
    li.mStrength = { 0.8f,0.8f,0.8f };
    li.mSpotPower = 2.f;
    li.mFalloffStart = 5.f;
    li.mFalloffEnd = 15.f;
    CAM_INFO ci = {};
    ci.mType = LENS_TYPE::ORTHOGRAPHIC;
    ci.mPosition = li.mPosition;
    ci.mLookAt = li.mDirection;
    ci.mUpVec = { 0.f,1.f,1.f };
    ci.mNearFarZ = { 1.f,1000.f };
    ci.mPFovyAndRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
    ci.mOWidthAndHeight = { 128.f * 9.5f,72.f * 9.5f };
    alc2.AddLight(li, false, true, ci);
    a2.AddAComponent(COMP_TYPE::A_LIGHT);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc2);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_LIGHT, alc2);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic2);

    _node->AddActorObject(a2);

    _node->SetCurrentAmbient({ 0.5f,0.5f,0.5f,0.5f });
}

void TestAInput(AInputComponent* _aic, Timer& _timer)
{
    auto atc = _aic->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    assert(atc);
    float delta = _timer.FloatDeltaTime();

    if (InputInterface::IsKeyDownInSingle(KB_W))
    {
        atc->TranslateZAsix(0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_A))
    {
        atc->TranslateXAsix(-0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_S))
    {
        atc->TranslateZAsix(-0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_D))
    {
        atc->TranslateXAsix(0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_Q))
    {
        atc->RotateYAsix(0.005f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_E))
    {
        atc->RotateYAsix(-0.005f * delta);
    }

    P_LOG(LOG_DEBUG, "%f , %f , %f\n",
        atc->GetProcessingPosition().x,
        atc->GetProcessingPosition().y,
        atc->GetProcessingPosition().z);
}

static int* g_IntArray = nullptr;

bool TestAInit(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "test a init!\n");
    g_IntArray = new int[5];
    for (int i = 0; i < 5; i++) { g_IntArray[i] = i * 2; }

    return true;
}

void TestAUpdate(AInteractComponent*, Timer&)
{
    std::string str = "";
    for (int i = 0; i < 5; i++) { str += std::to_string(g_IntArray[i]); }
    P_LOG(LOG_DEBUG, "%s\n", str.c_str());
}

void TestADestory(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "test a destory!\n");
    delete[] g_IntArray;
}

void TestA1Input(AInputComponent* _aic, Timer& _timer)
{
    auto atc = _aic->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    assert(atc);
    float delta = _timer.FloatDeltaTime();

    if (InputInterface::IsKeyDownInSingle(KB_I))
    {
        atc->TranslateZAsix(0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_J))
    {
        atc->TranslateXAsix(-0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_K))
    {
        atc->TranslateZAsix(-0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_L))
    {
        atc->TranslateXAsix(0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_U))
    {
        atc->TranslateYAsix(0.2f * delta);
    }
    if (InputInterface::IsKeyDownInSingle(KB_O))
    {
        atc->TranslateYAsix(-0.2f * delta);
    }
}
