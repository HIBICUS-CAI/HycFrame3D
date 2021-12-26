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
#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"

void TestAInput(AInputComponent*, Timer&);

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

    atc0.ForcePosition({ 0.f,0.f,20.f });
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
    amc0.AddMeshInfo("floor", { 0.f,-50.f,0.f });
    a0.AddAComponent(COMP_TYPE::A_MESH);

    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_TRANSFORM, atc0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INPUT, aic0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_INTERACT, aitc0);
    _node->GetComponentContainer()->AddComponent(COMP_TYPE::A_MESH, amc0);

    _node->AddActorObject(a0);

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

void AddBumpedTexTo(RS_SUBMESH_DATA* _result, std::string _filePath)
{
    RSRoot_DX11* root = GetRSRoot_DX11_Singleton();

    static std::wstring wstr = L"";
    static std::string name = "";
    static HRESULT hr = S_OK;
    ID3D11ShaderResourceView* srv = nullptr;

    wstr = std::wstring(_filePath.begin(), _filePath.end());
    wstr = L".\\Assets\\Textures\\" + wstr;

    if (_filePath.find(".dds") != std::string::npos ||
        _filePath.find(".DDS") != std::string::npos)
    {
        hr = DirectX::CreateDDSTextureFromFile(root->Devices()->GetDevice(),
            wstr.c_str(), nullptr, &srv);
        if (SUCCEEDED(hr))
        {
            name = _filePath;
            root->ResourceManager()->AddMeshSrv(name, srv);
        }
        else
        {
            bool texture_load_fail = false;
            assert(texture_load_fail);
        }
    }
    else
    {
        hr = DirectX::CreateWICTextureFromFile(root->Devices()->GetDevice(),
            wstr.c_str(), nullptr, &srv);
        if (SUCCEEDED(hr))
        {
            name = _filePath;
            root->ResourceManager()->AddMeshSrv(name, srv);
        }
        else
        {
            bool texture_load_fail = false;
            assert(texture_load_fail);
        }
    }

    _result->mTextures.emplace_back(name);
}
