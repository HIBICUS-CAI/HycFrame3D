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
            StartUp(WindowInterface::GetWindowPtr()->GetWndHandle()))
        {
            return false;
        }

        std::string name = "temp-cam";
        CAM_INFO ci = {};
        ci.mType = LENS_TYPE::PERSPECTIVE;
        ci.mPosition = { 0.f,0.f,0.f };
        ci.mLookAt = { 0.f,0.f,1.f };
        ci.mUpVec = { 0.f,1.f,0.f };
        ci.mNearFarZ = { 1.f,800.f };
        ci.mPFovyAndRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
        ci.mOWidthAndHeight = { 12.8f,7.2f };
        mRenderSystemRoot->CamerasContainer()->CreateRSCamera(name, &ci);

        name = "temp-ui-cam";
        ci = {};
        ci.mType = LENS_TYPE::ORTHOGRAPHIC;
        ci.mPosition = { 0.f,0.f,0.f };
        ci.mLookAt = { 0.f,0.f,1.f };
        ci.mUpVec = { 0.f,1.f,0.f };
        ci.mNearFarZ = { 1.f,100.f };
        ci.mPFovyAndRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
        ci.mOWidthAndHeight = { 1280.f,720.f };
        mRenderSystemRoot->CamerasContainer()->CreateRSCamera(name, &ci);

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

    GetRSRoot_DX11_Singleton()->ParticlesContainer()->ResetRSParticleSystem();

    return true;
}

void RenderSystem::Run(Timer& _timer)
{
    mRenderSystemRoot = GetRSRoot_DX11_Singleton();
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

        auto drawCallPool = mRenderSystemRoot->DrawCallsPool();
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
        drawCall.mMeshData.mLayout = mesh.second.mMeshData.mLayout;
        drawCall.mMeshData.mTopologyType = mesh.second.mMeshData.mTopologyType;
        drawCall.mMeshData.mVertexBuffer = mesh.second.mMeshData.mVertexBuffer;
        drawCall.mMeshData.mIndexBuffer = mesh.second.mMeshData.mIndexBuffer;
        drawCall.mMeshData.mIndexCount = mesh.second.mMeshData.mIndexCount;
        drawCall.mInstanceData.mDataPtr = &(mesh.second.mInstanceVector);
        drawCall.mInstanceData.mBonesDataPtr = &(mesh.second.mBonesVector);
        auto texSize = mesh.second.mMeshData.mTextures.size();
        auto& texArray = mesh.second.mMeshData.mTextures;
        for (size_t i = 0; i < texSize; i++)
        {
            if (texArray[i] != "")
            {
                drawCall.mTextureDatas[i].mUse = true;
                drawCall.mTextureDatas[i].mSrv = mRenderSystemRoot->
                    ResourceManager()->GetMeshSrv(texArray[i]);
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
        mRenderSystemRoot->DrawCallsPool()->AddDrawCallToPipe(
            DRAWCALL_TYPE::UI_SPRITE, btnSelectFlg);
    }
    mRenderSystemRoot->LightsContainer()->UploadLightBloomDrawCall();

    mRenderSystemRoot->LightsContainer()->ForceCurrentAmbientLight(
        GetSystemExecutive()->GetSceneManager()->
        GetCurrentSceneNode()->GetCurrentAmbientFactor());

    SetPipelineIBLTextures(mEnvTex, mDiffTex, mSpecTex);
    SetPipelineDeltaTime(_timer.FloatDeltaTime());

    GetRSRoot_DX11_Singleton()->PipelinesManager()->ProcessNextPipeline();
    mRenderSystemRoot->PipelinesManager()->ExecuateCurrentPipeline();
    mRenderSystemRoot->Devices()->PresentSwapChain();
    mRenderSystemRoot->DrawCallsPool()->ClearAllDrawCallsInPipes();
}

void RenderSystem::Destory()
{
    mRenderSystemRoot->CleanAndStop();
    delete mRenderSystemRoot;
}
