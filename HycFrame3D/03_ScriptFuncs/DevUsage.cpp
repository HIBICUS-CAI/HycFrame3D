#include "DevUsage.h"
#include "ModelHelper.h"
#include "SceneNode.h"
#include "SceneManager.h"
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
#include "AParticleComponent.h"
#include "ACollisionComponent.h"
#include "AAudioComponent.h"
#include "ATimerComponent.h"
#include "UTransformComponent.h"
#include "UInputComponent.h"
#include "USpriteComponent.h"
#include "UInteractComponent.h"
#include "UButtonComponent.h"
#include "UAudioComponent.h"
#include "UTimerComponent.h"

void TestAInput(AInputComponent*, Timer&);
void TestA1Input(AInputComponent*, Timer&);

bool TestAInit(AInteractComponent*);
void TestAUpdate(AInteractComponent*, Timer&);
void TestADestory(AInteractComponent*);

bool TestA3Init(AInteractComponent*);
void TestA3Update(AInteractComponent*, Timer&);
void TestA3Destory(AInteractComponent*);

void TestU0Input(UInputComponent*, Timer&);

bool TestU0Init(UInteractComponent*);
void TestU0Update(UInteractComponent*, Timer&);
void TestU0Destory(UInteractComponent*);

void TestUBtnInput(UInputComponent*, Timer&);

void TestActorSpInput(AInputComponent*, Timer&);
void TestUiSpInput(UInputComponent*, Timer&);

void DevUsage(SceneNode* _node)
{
    RS_SUBMESH_DATA sd = {};
    /*LoadModelFile(".\\Assets\\Models\\Dragon.FBX.json",
        MODEL_FILE_TYPE::JSON, &sd);*/
    sd = GetRSRoot_DX11_Singleton()->MeshHelper()->GeoGenerate()->CreateBox(
        20.f, 20.f, 40.f, 1, LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
        "sand.jpg");
    std::string nameM = "copper";
    sd.mMaterial = *GetRSRoot_DX11_Singleton()->StaticResources()->
        GetStaticMaterial(nameM);
    _node->GetAssetsPool()->InsertNewMesh("dragon", sd, MESH_TYPE::OPACITY);

    sd = GetRSRoot_DX11_Singleton()->MeshHelper()->GeoGenerate()->
        CreateGrid(500.f, 500.f, 10, 10, LAYOUT_TYPE::NORMAL_TANGENT_TEX,
            false, {}, "tile.dds");
    std::string name = "copper";
    sd.mMaterial = *(GetRSRoot_DX11_Singleton()->StaticResources()->
        GetStaticMaterial(name));
    AddBumpedTexTo(&sd, "tile_nmap.dds");
    _node->GetAssetsPool()->InsertNewMesh("floor", sd, MESH_TYPE::OPACITY);

    LoadSound("test", "bgm-success.wav");
    _node->GetAssetsPool()->InsertNewSound("test");

    ActorObject asp("asp", *_node);
    AInputComponent aicsp("asp-input", nullptr);
    aicsp.SetInputFunction(TestActorSpInput);
    asp.AddAComponent(COMP_TYPE::A_INPUT);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aicsp);
    _node->AddActorObject(asp);

    //ActorObject a0("a0", *_node);
    //ATransformComponent atc0("a0-transform", nullptr);
    //AInputComponent aic0("a0-input", nullptr);
    //AInteractComponent aitc0("a0-interact", nullptr);
    //AMeshComponent amc0("a0-mesh", nullptr);
    //ACollisionComponent acc0("a0-collision", nullptr);
    //AAudioComponent aac0("a0-audio", nullptr);
    //ATimerComponent atmc0("a0-timer", nullptr);

    //atc0.ForcePosition({ 0.f,0.f,150.f });
    //atc0.ForceRotation({ 0.f,0.f,0.f });
    //atc0.ForceScaling({ 1.f,1.f,1.f });
    //a0.AddAComponent(COMP_TYPE::A_TRANSFORM);

    //aic0.SetInputFunction(TestAInput);
    //a0.AddAComponent(COMP_TYPE::A_INPUT);

    //aitc0.SetInitFunction(TestAInit);
    //aitc0.SetUpdateFunction(TestAUpdate);
    //aitc0.SetDestoryFunction(TestADestory);
    //a0.AddAComponent(COMP_TYPE::A_INTERACT);

    //amc0.AddMeshInfo("dragon");
    //amc0.AddMeshInfo("floor", { 0.f,-20.f,0.f });
    //amc0.AddMeshInfo("floor", { 0.f,-50.f,0.f });
    //a0.AddAComponent(COMP_TYPE::A_MESH);

    //acc0.CreateCollisionShape(COLLISION_SHAPE::BOX, { 20.f,20.f,40.f });
    //a0.AddAComponent(COMP_TYPE::A_COLLISION);

    //aac0.AddAudio("test", *_node);
    //a0.AddAComponent(COMP_TYPE::A_AUDIO);

    //atmc0.AddTimer("test0");
    //a0.AddAComponent(COMP_TYPE::A_TIMER);

    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic0);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INTERACT, aitc0);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_MESH, amc0);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_COLLISION, acc0);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_AUDIO, aac0);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TIMER, atmc0);

    //_node->AddActorObject(a0);

    //ActorObject a1("a1", *_node);
    //ATransformComponent atc1("a1-transform", nullptr);
    //AInputComponent aic1("a1-input", nullptr);
    //ALightComponent alc1("a1-light", nullptr);
    //ACollisionComponent acc1("a1-collision", nullptr);

    //atc1.ForcePosition({ 0.f,0.f,50.f });
    //atc1.ForceRotation({ 0.f,0.f,0.f });
    //atc1.ForceScaling({ 3.f,3.f,3.f });
    //a1.AddAComponent(COMP_TYPE::A_TRANSFORM);

    //aic1.SetInputFunction(TestA1Input);
    //a1.AddAComponent(COMP_TYPE::A_INPUT);

    //LIGHT_INFO li = {};
    //li.mType = LIGHT_TYPE::POINT;
    //li.mWithShadow = false;
    //li.mPosition = { 0.f,0.f,0.f };
    //li.mDirection = { 0.f,-0.f,0.f };
    //li.mStrength = { 0.9f,0.f,0.9f };
    //li.mSpotPower = 2.f;
    //li.mFalloffStart = 30.f;
    //li.mFalloffEnd = 80.f;
    //alc1.AddLight(li, true, false, {});
    //a1.AddAComponent(COMP_TYPE::A_LIGHT);

    //acc1.CreateCollisionShape(COLLISION_SHAPE::BOX, { 3.f,3.f,3.f });
    //a1.AddAComponent(COMP_TYPE::A_COLLISION);

    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc1);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic1);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_LIGHT, alc1);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_COLLISION, acc1);

    //_node->AddActorObject(a1);

    //ActorObject a2("a2", *_node);
    //ATransformComponent atc2("a2-transform", nullptr);
    //ALightComponent alc2("a2-light", nullptr);
    ////AInputComponent aic2("a2-input", nullptr);

    //atc2.ForcePosition({ 0.f,30.f,-30.f });
    //atc2.ForceRotation({ 0.f,0.f,0.f });
    //atc2.ForceScaling({ 1.f,1.f,1.f });
    //a2.AddAComponent(COMP_TYPE::A_TRANSFORM);

    ////aic2.SetInputFunction(TestA1Input);
    ////a2.AddAComponent(COMP_TYPE::A_INPUT);

    //li = {};
    //li.mType = LIGHT_TYPE::DIRECT;
    //li.mWithShadow = true;
    //li.mPosition = { 0.f,30.f,-30.f };
    //li.mDirection = { 0.f,-1.f,1.f };
    //li.mStrength = { 0.8f,0.8f,0.8f };
    //li.mSpotPower = 2.f;
    //li.mFalloffStart = 5.f;
    //li.mFalloffEnd = 15.f;
    //CAM_INFO ci = {};
    //ci.mType = LENS_TYPE::ORTHOGRAPHIC;
    //ci.mPosition = li.mPosition;
    //ci.mLookAt = li.mDirection;
    //ci.mUpVec = { 0.f,1.f,1.f };
    //ci.mNearFarZ = { 1.f,1000.f };
    //ci.mPFovyAndRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
    //ci.mOWidthAndHeight = { 128.f * 9.5f,72.f * 9.5f };
    //alc2.AddLight(li, false, true, ci);
    //a2.AddAComponent(COMP_TYPE::A_LIGHT);

    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc2);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_LIGHT, alc2);
    ////_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic2);

    //_node->AddActorObject(a2);

    //ActorObject a3("a3", *_node);
    //ATransformComponent atc3("a3-transform", nullptr);
    //AParticleComponent apc3("a3-particle", nullptr);
    //AInteractComponent aitc3("a3-interact", nullptr);

    //atc3.ForcePosition({ 0.f,5.f,25.f });
    //atc3.ForceRotation({ 0.f,0.f,0.f });
    //atc3.ForceScaling({ 3.f,3.f,3.f });
    //a3.AddAComponent(COMP_TYPE::A_TRANSFORM);

    //PARTICLE_EMITTER_INFO pei = {};
    //pei.mAcceleration = { 0.f,-9.8f,0.f };
    //pei.mEmitNumPerSecond = 2400.f;
    //pei.mEnableStreak = true;
    //pei.mLifeSpan = 100.f;
    //pei.mOffsetEndColor = { 0.f,0.f,0.f,0.f };
    //pei.mOffsetEndSize = 0.f;
    //pei.mOffsetStartColor = { 1.f,0.f,0.f,1.f };
    //pei.mOffsetStartSize = 0.5f;
    //pei.mParticleMass = 0.3f;
    //pei.mPosVariance = { 1.f,1.f,1.f };
    //pei.mTextureID = PARTICLE_TEXTURE::WHITE_CIRCLE;
    //pei.mVelocity = { 0.f,18.f,0.f };
    //pei.mVelVariance = 0.5f;
    //apc3.CreateEmitter(&pei);
    //a3.AddAComponent(COMP_TYPE::A_PARTICLE);

    //aitc3.SetInitFunction(TestA3Init);
    //aitc3.SetUpdateFunction(TestA3Update);
    //aitc3.SetDestoryFunction(TestA3Destory);
    //a3.AddAComponent(COMP_TYPE::A_INTERACT);

    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc3);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_PARTICLE, apc3);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INTERACT, aitc3);

    //_node->AddActorObject(a3);

    _node->SetCurrentAmbient({ 0.5f,0.5f,0.5f,0.5f });
}

SceneNode* CreateScene1(SceneManager* _manager)
{
    SceneNode* node = new SceneNode("test1", _manager);

    RS_SUBMESH_DATA sd = {};
    LoadModelFile("Dragon.FBX.meshdata", MODEL_FILE_TYPE::BIN, &sd);
    //AddDiffuseTexTo(&sd, "sand.jpg");
    //AddDiffuseTexTo(&sd, "hill_diffuse.png");
    //AddBumpedTexTo(&sd, "hill_normal.png");
    std::string nameM = "copper";
    sd.mMaterial = *GetRSRoot_DX11_Singleton()->StaticResources()->
        GetStaticMaterial(nameM);
    sd.mMaterial.mFresnelR0 = { 0.05f,0.05f,0.05f };
    sd.mMaterial.mShininess = 0.3f;
    node->GetAssetsPool()->InsertNewMesh("dragon", sd, MESH_TYPE::OPACITY);

    sd = GetRSRoot_DX11_Singleton()->MeshHelper()->GeoGenerate()->
        CreateGrid(500.f, 500.f, 10, 10, LAYOUT_TYPE::NORMAL_TANGENT_TEX,
            false, {}, "tile.dds");
    std::string name = "copper";
    sd.mMaterial = *(GetRSRoot_DX11_Singleton()->StaticResources()->
        GetStaticMaterial(name));
    AddBumpedTexTo(&sd, "tile_nmap.dds");
    node->GetAssetsPool()->InsertNewMesh("floor", sd, MESH_TYPE::OPACITY);

    LoadSound("test", "bgm-success.wav");
    node->GetAssetsPool()->InsertNewSound("test");

    ActorObject asp("asp", *node);
    AInputComponent aicsp("asp-input", nullptr);
    aicsp.SetInputFunction(TestActorSpInput);
    asp.AddAComponent(COMP_TYPE::A_INPUT);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aicsp);
    node->AddActorObject(asp);

    ActorObject a0("a0", *node);
    ATransformComponent atc0("a0-transform", nullptr);
    AInputComponent aic0("a0-input", nullptr);
    AInteractComponent aitc0("a0-interact", nullptr);
    AMeshComponent amc0("a0-mesh", nullptr);
    ACollisionComponent acc0("a0-collision", nullptr);
    AAudioComponent aac0("a0-audio", nullptr);
    ATimerComponent atmc0("a0-timer", nullptr);

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

    acc0.CreateCollisionShape(COLLISION_SHAPE::BOX, { 20.f,20.f,40.f });
    a0.AddAComponent(COMP_TYPE::A_COLLISION);

    aac0.AddAudio("test", *node);
    a0.AddAComponent(COMP_TYPE::A_AUDIO);

    atmc0.AddTimer("test0");
    a0.AddAComponent(COMP_TYPE::A_TIMER);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INTERACT, aitc0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_MESH, amc0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_COLLISION, acc0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_AUDIO, aac0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TIMER, atmc0);

    node->AddActorObject(a0);

    ActorObject a1("a1", *node);
    ATransformComponent atc1("a1-transform", nullptr);
    AInputComponent aic1("a1-input", nullptr);
    ALightComponent alc1("a1-light", nullptr);
    ACollisionComponent acc1("a1-collision", nullptr);

    atc1.ForcePosition({ 0.f,0.f,50.f });
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

    acc1.CreateCollisionShape(COLLISION_SHAPE::BOX, { 3.f,3.f,3.f });
    a1.AddAComponent(COMP_TYPE::A_COLLISION);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc1);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic1);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_LIGHT, alc1);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_COLLISION, acc1);

    node->AddActorObject(a1);

    ActorObject a2("a2", *node);
    ATransformComponent atc2("a2-transform", nullptr);
    ALightComponent alc2("a2-light", nullptr);
    //AInputComponent aic2("a2-input", nullptr);

    atc2.ForcePosition({ 0.f,30.f,-30.f });
    atc2.ForceRotation({ 0.f,0.f,0.f });
    atc2.ForceScaling({ 1.f,1.f,1.f });
    a2.AddAComponent(COMP_TYPE::A_TRANSFORM);

    ////aic2.SetInputFunction(TestA1Input);
    ////a2.AddAComponent(COMP_TYPE::A_INPUT);

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

    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc2);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_LIGHT, alc2);
    //_node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic2);

    node->AddActorObject(a2);

    ActorObject a3("a3", *node);
    ATransformComponent atc3("a3-transform", nullptr);
    AParticleComponent apc3("a3-particle", nullptr);
    AInteractComponent aitc3("a3-interact", nullptr);

    atc3.ForcePosition({ 0.f,5.f,25.f });
    atc3.ForceRotation({ 0.f,0.f,0.f });
    atc3.ForceScaling({ 3.f,3.f,3.f });
    a3.AddAComponent(COMP_TYPE::A_TRANSFORM);

    PARTICLE_EMITTER_INFO pei = {};
    pei.mAcceleration = { 0.f,-9.8f,0.f };
    pei.mEmitNumPerSecond = 2400.f;
    pei.mEnableStreak = true;
    pei.mLifeSpan = 100.f;
    pei.mOffsetEndColor = { 0.f,0.f,0.f,0.f };
    pei.mOffsetEndSize = 0.f;
    pei.mOffsetStartColor = { 1.f,0.f,0.f,1.f };
    pei.mOffsetStartSize = 0.5f;
    pei.mParticleMass = 0.3f;
    pei.mPosVariance = { 1.f,1.f,1.f };
    pei.mTextureID = PARTICLE_TEXTURE::WHITE_CIRCLE;
    pei.mVelocity = { 0.f,18.f,0.f };
    pei.mVelVariance = 0.5f;
    apc3.CreateEmitter(&pei);
    a3.AddAComponent(COMP_TYPE::A_PARTICLE);

    aitc3.SetInitFunction(TestA3Init);
    aitc3.SetUpdateFunction(TestA3Update);
    aitc3.SetDestoryFunction(TestA3Destory);
    a3.AddAComponent(COMP_TYPE::A_INTERACT);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc3);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_PARTICLE, apc3);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INTERACT, aitc3);

    node->AddActorObject(a3);

    node->SetCurrentAmbient({ 0.5f,0.5f,0.5f,0.5f });

    return node;
}

SceneNode* CreateScene2(SceneManager* _manager)
{
    SceneNode* node = new SceneNode("test2", _manager);

    LoadSound("test", "bgm-success.wav");
    node->GetAssetsPool()->InsertNewSound("test");

    UiObject usp("usp", *node);
    UInputComponent uicsp("usp-input", nullptr);
    uicsp.SetInputFunction(TestUiSpInput);
    usp.AddUComponent(COMP_TYPE::U_INPUT);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_INPUT, uicsp);
    node->AddUiObject(usp);

    UiObject u0("u0", *node);
    UTransformComponent utc0("u0-transform", nullptr);
    UInputComponent uic0("u0-input", nullptr);
    USpriteComponent usc0("u0-sprite", nullptr);
    UAnimateComponent uamc0("u0-animate", nullptr);
    UInteractComponent uitc0("u0-interact", nullptr);
    UAudioComponent uac0("u0-audio", nullptr);
    UTimerComponent utmc0("u0-timer", nullptr);

    utc0.ForcePosition({ -400.f,300.f,0.f });
    utc0.ForceRotation({ 0.f,0.f,0.f });
    utc0.ForceScaling({ 200.f,100.f,1.f });
    u0.AddUComponent(COMP_TYPE::U_TRANSFORM);

    uic0.SetInputFunction(TestU0Input);
    u0.AddUComponent(COMP_TYPE::U_INPUT);

    usc0.CreateSpriteMesh(node, { 1.f,1.f,1.f,1.f }, "cloud.png");
    u0.AddUComponent(COMP_TYPE::U_SPRITE);

    uamc0.LoadAnimate("number", "number.png", { 0.2f,0.2f },
        13, false, 0.5f);
    uamc0.LoadAnimate("runman", "runman.png", { 0.2f,0.5f },
        10, true, 0.1f);
    u0.AddUComponent(COMP_TYPE::U_ANIMATE);

    uitc0.SetInitFunction(TestU0Init);
    uitc0.SetUpdateFunction(TestU0Update);
    uitc0.SetDestoryFunction(TestU0Destory);
    u0.AddUComponent(COMP_TYPE::U_INTERACT);

    uac0.AddAudio("test", *node);
    u0.AddUComponent(COMP_TYPE::U_AUDIO);

    utmc0.AddTimer("u0-test");
    u0.AddUComponent(COMP_TYPE::U_TIMER);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_TRANSFORM, utc0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_INPUT, uic0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_SPRITE, usc0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_ANIMATE, uamc0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_INTERACT, uitc0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_AUDIO, uac0);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_TIMER, utmc0);

    node->AddUiObject(u0);

    UiObject u1("u1", *node);
    UTransformComponent utc1("u1-transform", nullptr);
    UInputComponent uic1("u1-input", nullptr);
    USpriteComponent usc1("u1-sprite", nullptr);
    UButtonComponent ubc1("u1-button", nullptr);

    utc1.ForcePosition({ -200.f,200.f,0.f });
    utc1.ForceRotation({ 0.f,0.f,0.f });
    utc1.ForceScaling({ 100.f,50.f,1.f });
    u1.AddUComponent(COMP_TYPE::U_TRANSFORM);

    uic1.SetInputFunction(TestUBtnInput);
    u1.AddUComponent(COMP_TYPE::U_INPUT);

    usc1.CreateSpriteMesh(node, { 1.f,1.f,1.f,1.f }, "basic_btn.png");
    u1.AddUComponent(COMP_TYPE::U_SPRITE);

    ubc1.SetIsBeingSelected(true);
    ubc1.SetRightBtnObjName("u2");
    u1.AddUComponent(COMP_TYPE::U_BUTTON);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_TRANSFORM, utc1);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_INPUT, uic1);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_SPRITE, usc1);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_BUTTON, ubc1);

    node->AddUiObject(u1);

    UiObject u2("u2", *node);
    UTransformComponent utc2("u2-transform", nullptr);
    UInputComponent uic2("u2-input", nullptr);
    USpriteComponent usc2("u2-sprite", nullptr);
    UButtonComponent ubc2("u2-button", nullptr);

    utc2.ForcePosition({ 0.f,200.f,0.f });
    utc2.ForceRotation({ 0.f,0.f,0.f });
    utc2.ForceScaling({ 100.f,50.f,1.f });
    u2.AddUComponent(COMP_TYPE::U_TRANSFORM);

    uic2.SetInputFunction(TestUBtnInput);
    u2.AddUComponent(COMP_TYPE::U_INPUT);

    usc2.CreateSpriteMesh(node, { 1.f,1.f,1.f,1.f }, "basic_btn.png");
    u2.AddUComponent(COMP_TYPE::U_SPRITE);

    ubc2.SetLeftBtnObjName("u1");
    ubc2.SetRightBtnObjName("u3");
    ubc2.SetDownBtnObjName("u4");
    u2.AddUComponent(COMP_TYPE::U_BUTTON);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_TRANSFORM, utc2);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_INPUT, uic2);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_SPRITE, usc2);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_BUTTON, ubc2);

    node->AddUiObject(u2);

    UiObject u3("u3", *node);
    UTransformComponent utc3("u3-transform", nullptr);
    UInputComponent uic3("u3-input", nullptr);
    USpriteComponent usc3("u3-sprite", nullptr);
    UButtonComponent ubc3("u3-button", nullptr);

    utc3.ForcePosition({ 200.f,200.f,0.f });
    utc3.ForceRotation({ 0.f,0.f,0.f });
    utc3.ForceScaling({ 100.f,50.f,1.f });
    u3.AddUComponent(COMP_TYPE::U_TRANSFORM);

    uic3.SetInputFunction(TestUBtnInput);
    u3.AddUComponent(COMP_TYPE::U_INPUT);

    usc3.CreateSpriteMesh(node, { 1.f,1.f,1.f,1.f }, "basic_btn.png");
    u3.AddUComponent(COMP_TYPE::U_SPRITE);

    ubc3.SetLeftBtnObjName("u2");
    u3.AddUComponent(COMP_TYPE::U_BUTTON);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_TRANSFORM, utc3);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_INPUT, uic3);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_SPRITE, usc3);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_BUTTON, ubc3);

    node->AddUiObject(u3);

    UiObject u4("u4", *node);
    UTransformComponent utc4("u4-transform", nullptr);
    UInputComponent uic4("u4-input", nullptr);
    USpriteComponent usc4("u4-sprite", nullptr);
    UButtonComponent ubc4("u4-button", nullptr);

    utc4.ForcePosition({ 0.f,0.f,0.f });
    utc4.ForceRotation({ 0.f,0.f,0.f });
    utc4.ForceScaling({ 100.f,50.f,1.f });
    u4.AddUComponent(COMP_TYPE::U_TRANSFORM);

    uic4.SetInputFunction(TestUBtnInput);
    u4.AddUComponent(COMP_TYPE::U_INPUT);

    usc4.CreateSpriteMesh(node, { 1.f,1.f,1.f,1.f }, "basic_btn.png");
    u4.AddUComponent(COMP_TYPE::U_SPRITE);

    ubc4.SetUpBtnObjName("u2");
    ubc4.SetDownBtnObjName("u5");
    u4.AddUComponent(COMP_TYPE::U_BUTTON);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_TRANSFORM, utc4);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_INPUT, uic4);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_SPRITE, usc4);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_BUTTON, ubc4);

    node->AddUiObject(u4);

    UiObject u5("u5", *node);
    UTransformComponent utc5("u5-transform", nullptr);
    UInputComponent uic5("u5-input", nullptr);
    USpriteComponent usc5("u5-sprite", nullptr);
    UButtonComponent ubc5("u5-button", nullptr);

    utc5.ForcePosition({ 0.f,-200.f,0.f });
    utc5.ForceRotation({ 0.f,0.f,0.f });
    utc5.ForceScaling({ 100.f,50.f,1.f });
    u5.AddUComponent(COMP_TYPE::U_TRANSFORM);

    uic5.SetInputFunction(TestUBtnInput);
    u5.AddUComponent(COMP_TYPE::U_INPUT);

    usc5.CreateSpriteMesh(node, { 1.f,1.f,1.f,1.f }, "basic_btn.png");
    u5.AddUComponent(COMP_TYPE::U_SPRITE);

    ubc5.SetUpBtnObjName("u4");
    u5.AddUComponent(COMP_TYPE::U_BUTTON);

    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_TRANSFORM, utc5);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_INPUT, uic5);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_SPRITE, usc5);
    node->GetComponentContainer()->AddComponent(COMP_TYPE::U_BUTTON, ubc5);

    node->AddUiObject(u5);

    return node;
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

    auto aac = _aic->GetActorOwner()->
        GetAComponent<AAudioComponent>(COMP_TYPE::A_AUDIO);
    assert(aac);
    if (InputInterface::IsKeyPushedInSingle(KB_Z))
    {
        aac->StopBgm();
        aac->PlayBgm("test", 0.5f);
    }
    if (InputInterface::IsKeyPushedInSingle(KB_X))
    {
        aac->StopBgm();
        aac->PlayBgm("test", 0.3f);
    }
    if (InputInterface::IsKeyPushedInSingle(KB_C))
    {
        aac->StopBgm();
        aac->PlayBgm("test", 0.1f);
    }

    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        /*_aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("test2", "test2");*/
    }
}

static int* g_IntArray = nullptr;
static ACollisionComponent* g_DragonCC = nullptr;
static ATransformComponent* g_LightTC = nullptr;

bool TestAInit(AInteractComponent* _aitc)
{
    P_LOG(LOG_DEBUG, "test a init!\n");
    g_IntArray = new int[5];
    for (int i = 0; i < 5; i++) { g_IntArray[i] = i * 2; }

    return true;
}

void TestAUpdate(AInteractComponent* _aitc, Timer&)
{
    if (!g_DragonCC || !g_LightTC)
    {
        g_DragonCC = _aitc->GetActorOwner()->
            GetAComponent<ACollisionComponent>(COMP_TYPE::A_COLLISION);
        g_LightTC = _aitc->GetActorOwner()->GetSceneNode().GetActorObject("a1")->
            GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    }

    bool coll = g_DragonCC->CheckCollisionWith("a1");
    if (coll)
    {
        g_LightTC->RollBackPosition();
    }

    auto atmc = _aitc->GetActorOwner()->
        GetAComponent<ATimerComponent>(COMP_TYPE::A_TIMER);
    if (!atmc->GetTimer("test0")->mActive)
    {
        atmc->StartTimer("test0");
    }
    if (atmc->GetTimer("test0")->mActive)
    {
        _aitc->GetActorOwner()->GetSceneNode().GetActorObject("a3")->
            GetAComponent<AParticleComponent>(COMP_TYPE::A_PARTICLE)->
            GetEmitterInfo().mEmitNumPerSecond = powf(
                sinf(atmc->GetTimer("test0")->mTime), 2.f) * 2400.f;
    }
}

void TestADestory(AInteractComponent*)
{
    P_LOG(LOG_DEBUG, "test a destory!\n");
    delete[] g_IntArray;
    g_DragonCC = nullptr;
    g_LightTC = nullptr;
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

    if (InputInterface::IsKeyPushedInSingle(KB_N))
    {
        _aic->GetActorOwner()->GetSceneNode().GetActorObject("a0")->
            SetObjectStatus(STATUS::ACTIVE);
    }
    if (InputInterface::IsKeyPushedInSingle(KB_M))
    {
        _aic->GetActorOwner()->GetSceneNode().GetActorObject("a0")->
            SetObjectStatus(STATUS::PAUSE);
    }
}

static ATransformComponent* g_PointAtc = nullptr;
static ATransformComponent* g_PartiAtc = nullptr;

bool TestA3Init(AInteractComponent* _aitc)
{
    g_PointAtc = _aitc->GetActorOwner()->GetSceneNode().GetActorObject("a1")->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    if (!g_PointAtc) { return false; }

    g_PartiAtc = _aitc->GetActorOwner()->
        GetAComponent<ATransformComponent>(COMP_TYPE::A_TRANSFORM);
    if (!g_PartiAtc) { return false; }

    return true;
}

void TestA3Update(AInteractComponent* _aitc, Timer& _timer)
{
    float x = g_PointAtc->GetProcessingPosition().x;
    float z = g_PointAtc->GetProcessingPosition().z;
    float y = g_PartiAtc->GetProcessingPosition().y;
    g_PartiAtc->SetPosition({ x,y,z });
}

void TestA3Destory(AInteractComponent* _aitc)
{
    g_PointAtc = nullptr;
    g_PartiAtc = nullptr;
}

void TestU0Input(UInputComponent* _uic, Timer& _timer)
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

    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        /*_uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("test1", "test1");*/
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
}

bool TestU0Init(UInteractComponent* _uitc)
{
    P_LOG(LOG_DEBUG, "u0 interact init!!!\n");

    return true;
}

void TestU0Update(UInteractComponent* _uitc, Timer& _timer)
{
    /*auto timer = _uitc->GetUiOwner()->
        GetUComponent<UTimerComponent>(COMP_TYPE::U_TIMER)->GetTimer("u0-test");
    if (!timer->mActive)
    {
        _uitc->GetUiOwner()->
            GetUComponent<UTimerComponent>(COMP_TYPE::U_TIMER)->
            StartTimer("u0-test");
    }

    if (timer->IsGreaterThan(10.f))
    {
        _uitc->GetUiOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("test1", "test1");
    }*/
}

void TestU0Destory(UInteractComponent* _uitc)
{
    P_LOG(LOG_DEBUG, "u0 interact destory!!!\n");
}

void TestUBtnInput(UInputComponent* _uic, Timer& _timer)
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

void TestActorSpInput(AInputComponent* _aic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test2\n");
        _aic->GetActorOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("test2", "test2");
    }
}

void TestUiSpInput(UInputComponent* _uic, Timer&)
{
    if (InputInterface::IsKeyPushedInSingle(KB_RETURN))
    {
        P_LOG(LOG_DEBUG, "to test1\n");
        _uic->GetUiOwner()->GetSceneNode().GetSceneManager()->
            LoadSceneNode("test1", "test1");
    }
}
