#include "RenderSystem.h"
#include "RSRoot_DX11.h"
#include "RSPipelinesManager.h"
#include "RSDevices.h"
#include "RSDrawCallsPool.h"
#include "RSCamera.h"
#include "RSCamerasContainer.h"
#include "RSResourceManager.h"
#include "RSLightsContainer.h"
#include "RSParticlesContainer.h"
#include "WM_Interface.h"
#include "BasicRSPipeline.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "AssetsPool.h"
#include "UButtonComponent.h"

RenderSystem::RenderSystem(SystemExecutive* _sysExecutive) :
    System("render-system", _sysExecutive),
    mRenderSystemRoot(nullptr), mAssetsPool(nullptr),
    mEnvTex(nullptr), mDiffTex(nullptr), mSpecTex(nullptr)
{

}

RenderSystem::~RenderSystem()
{

}

bool RenderSystem::Init()
{
    if (!mRenderSystemRoot)
    {
        mRenderSystemRoot = new RSRoot_DX11();
        if (!mRenderSystemRoot->
            startUp(window::getWindowPtr()->getWndHandle()))
        {
            return false;
        }

        std::string name = "temp-cam";
        CAM_INFO ci = {};
        ci.mType = LENS_TYPE::PERSPECTIVE;
        ci.Position = { 0.f,0.f,0.f };
        ci.LookAtVector = { 0.f,0.f,1.f };
        ci.UpVector = { 0.f,1.f,0.f };
        ci.NearFarZ = { 1.f,800.f };
        ci.PerspFovYRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
        ci.OrthoWidthHeight = { 12.8f,7.2f };
        mRenderSystemRoot->getCamerasContainer()->CreateRSCamera(name, &ci);

        name = "temp-ui-cam";
        ci = {};
        ci.mType = LENS_TYPE::ORTHOGRAPHIC;
        ci.Position = { 0.f,0.f,0.f };
        ci.LookAtVector = { 0.f,0.f,1.f };
        ci.UpVector = { 0.f,1.f,0.f };
        ci.NearFarZ = { 1.f,100.f };
        ci.PerspFovYRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
        ci.OrthoWidthHeight = { 1280.f,720.f };
        mRenderSystemRoot->getCamerasContainer()->CreateRSCamera(name, &ci);

        if (!CreateBasicPipeline()) { return false; }
    }

    mAssetsPool = nullptr;
    mAssetsPool = GetSystemExecutive()->GetSceneManager()->
        GetCurrentSceneNode()->GetAssetsPool();
    if (!mAssetsPool) { return false; }

    mEnvTex = GetSystemExecutive()->GetSceneManager()->
        GetCurrentSceneNode()->GetIBLEnvironment();
    mDiffTex = GetSystemExecutive()->GetSceneManager()->
        GetCurrentSceneNode()->GetIBLDiffuse();
    mSpecTex = GetSystemExecutive()->GetSceneManager()->
        GetCurrentSceneNode()->GetIBLSpecular();

    getRSDX11RootInstance()->getParticlesContainer()->ResetRSParticleSystem();

    return true;
}

void RenderSystem::Run(Timer& _timer)
{
    mRenderSystemRoot = getRSDX11RootInstance();
#ifdef _DEBUG
    assert(mRenderSystemRoot);
#endif // _DEBUG

    static RS_DRAWCALL_DATA drawCall = {};
    auto& meshPool = mAssetsPool->mSubMeshPool;
    RS_DRAWCALL_DATA btnSelectFlg = {};
    bool hasBtnSelect = false;
    for (auto& mesh : meshPool)
    {
        mesh.second.mInstanceVector.clear();
        for (auto& instance : mesh.second.mInstanceMap)
        {
            mesh.second.mInstanceVector.emplace_back(instance.second);
        }
        if (!mesh.second.mInstanceVector.size()) { continue; }

        mesh.second.mBonesVector.clear();
        for (auto& bone : mesh.second.mBonesMap)
        {
            mesh.second.mBonesVector.emplace_back(bone.second);
        }

        auto drawCallPool = mRenderSystemRoot->getDrawCallsPool();
        DRAWCALL_TYPE dType = DRAWCALL_TYPE::MAX;
        MESH_TYPE mType = mesh.second.mMeshType;
        switch (mType)
        {
        case MESH_TYPE::OPACITY: dType = DRAWCALL_TYPE::OPACITY; break;
        case MESH_TYPE::TRANSPARENCY: dType = DRAWCALL_TYPE::TRANSPARENCY; break;
        case MESH_TYPE::LIGHT: dType = DRAWCALL_TYPE::LIGHT; break;
        case MESH_TYPE::UI_SPRITE: dType = DRAWCALL_TYPE::UI_SPRITE; break;
        default: break;
        }

        drawCall = {};
        drawCall.MeshData.InputLayout = mesh.second.mMeshData.InputLayout;
        drawCall.MeshData.TopologyType = mesh.second.mMeshData.TopologyType;
        drawCall.MeshData.VertexBuffer = mesh.second.mMeshData.VertexBuffer;
        drawCall.MeshData.IndexBuffer = mesh.second.mMeshData.IndexBuffer;
        drawCall.MeshData.IndexSize = mesh.second.mMeshData.IndexSize;
        drawCall.InstanceData.DataArrayPtr = &(mesh.second.mInstanceVector);
        drawCall.InstanceData.BonesArrayPtr = &(mesh.second.mBonesVector);
        auto texSize = mesh.second.mMeshData.Textures.size();
        auto& texArray = mesh.second.mMeshData.Textures;
        for (size_t i = 0; i < texSize; i++)
        {
            if (texArray[i] != "")
            {
                drawCall.TextureData[i].EnabledFlag = true;
                drawCall.TextureData[i].Srv = mRenderSystemRoot->
                    getResourceManager()->GetMeshSrv(texArray[i]);
            }
        }

        drawCallPool->AddDrawCallToPipe(dType, drawCall);
        if (mesh.first == SELECTED_BTN_SPRITE_NAME)
        {
            hasBtnSelect = true;
            btnSelectFlg = drawCall;
        }
    }
    if (hasBtnSelect)
    {
        mRenderSystemRoot->getDrawCallsPool()->AddDrawCallToPipe(
            DRAWCALL_TYPE::UI_SPRITE, btnSelectFlg);
    }
    mRenderSystemRoot->getLightsContainer()->UploadLightBloomDrawCall();

    mRenderSystemRoot->getLightsContainer()->ForceCurrentAmbientLight(
        GetSystemExecutive()->GetSceneManager()->
        GetCurrentSceneNode()->GetCurrentAmbientFactor());

    SetPipelineIBLTextures(mEnvTex, mDiffTex, mSpecTex);
    SetPipelineDeltaTime(_timer.FloatDeltaTime());

    getRSDX11RootInstance()->getPipelinesManager()->ProcessNextPipeline();
    mRenderSystemRoot->getPipelinesManager()->ExecuateCurrentPipeline();
    mRenderSystemRoot->getDevices()->PresentSwapChain();
    mRenderSystemRoot->getDrawCallsPool()->ClearAllDrawCallsInPipes();
}

void RenderSystem::Destory()
{
    mRenderSystemRoot->cleanAndStop();
    delete mRenderSystemRoot;
}
