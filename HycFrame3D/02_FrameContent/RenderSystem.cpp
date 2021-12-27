#include "RenderSystem.h"
#include "RSRoot_DX11.h"
#include "RSPipelinesManager.h"
#include "RSDevices.h"
#include "RSDrawCallsPool.h"
#include "RSCamera.h"
#include "RSCamerasContainer.h"
#include "RSResourceManager.h"
#include "RSLightsContainer.h"
#include "WM_Interface.h"
#include "BasicRSPipeline.h"
#include "SystemExecutive.h"
#include "SceneManager.h"
#include "SceneNode.h"
#include "AssetsPool.h"

RenderSystem::RenderSystem(SystemExecutive* _sysExecutive) :
    System("render-system", _sysExecutive),
    mRenderSystemRoot(nullptr), mAssetsPool(nullptr)
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
        ci.mNearFarZ = { 1.f,500.f };
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

        // TEMP------------------------------------
        //name = "direct-light-1";
        //LIGHT_INFO li = {};
        //li.mType = LIGHT_TYPE::DIRECT;
        //li.mWithShadow = true;
        //li.mPosition = { 0.f,30.f,-30.f };
        //li.mDirection = { 0.f,-1.f,1.f };
        //li.mStrength = { 0.8f,0.8f,0.8f };
        //li.mSpotPower = 2.f;
        //li.mFalloffStart = 5.f;
        //li.mFalloffEnd = 15.f;
        //ci = {};
        //ci.mType = LENS_TYPE::ORTHOGRAPHIC;
        //ci.mPosition = li.mPosition;
        //ci.mLookAt = li.mDirection;
        //ci.mUpVec = { 0.f,1.f,1.f };
        //ci.mNearFarZ = { 1.f,1000.f };
        //ci.mPFovyAndRatio = { DirectX::XM_PIDIV4,16.f / 9.f };
        //ci.mOWidthAndHeight = { 128.f * 9.5f,72.f * 9.5f };
        //mRenderSystemRoot->LightsContainer()->CreateRSLight(
        //    name, &li);
        //mRenderSystemRoot->LightsContainer()->CreateLightCameraFor(name, &ci);
        // TEMP------------------------------------

        if (!CreateBasicPipeline()) { return false; }
    }

    mAssetsPool = GetSystemExecutive()->GetSceneManager()->
        GetCurrentSceneNode()->GetAssetsPool();
    if (!mAssetsPool) { return false; }

    return true;
}

void RenderSystem::Run(Timer& _timer)
{
    mRenderSystemRoot = GetRSRoot_DX11_Singleton();
#ifdef _DEBUG
    assert(mRenderSystemRoot);
#endif // _DEBUG

    static RS_DRAWCALL_DATA drawCall = {};
    auto& meshPool = mAssetsPool->mMeshPool;

    for (auto& mesh : meshPool)
    {
        mesh.second.mInstanceVector.clear();
        for (auto& instance : mesh.second.mInstanceMap)
        {
            mesh.second.mInstanceVector.emplace_back(instance.second);
        }
        if (!mesh.second.mInstanceVector.size()) { continue; }

        auto drawCallPool = mRenderSystemRoot->DrawCallsPool();
        DRAWCALL_TYPE dType = DRAWCALL_TYPE::MAX;
        MESH_TYPE mType = mesh.second.mMeshType;
        switch (mType)
        {
        case MESH_TYPE::OPACITY: dType = DRAWCALL_TYPE::OPACITY; break;
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
        drawCall.mTextureDatas[0].mUse = true;
        drawCall.mTextureDatas[0].mSrv = mRenderSystemRoot->ResourceManager()->
            GetMeshSrv(mesh.second.mMeshData.mTextures[0]);
        if (mesh.second.mMeshData.mTextures.size() > 1)
        {
            drawCall.mTextureDatas[1].mUse = true;
            drawCall.mTextureDatas[1].mSrv = mRenderSystemRoot->
                ResourceManager()->
                GetMeshSrv(mesh.second.mMeshData.mTextures[1]);
        }

        drawCallPool->AddDrawCallToPipe(dType, drawCall);
    }
    mRenderSystemRoot->LightsContainer()->UploadLightBloomDrawCall();

    mRenderSystemRoot->LightsContainer()->ForceCurrentAmbientLight(
        GetSystemExecutive()->GetSceneManager()->
        GetCurrentSceneNode()->GetCurrentAmbient());

    mRenderSystemRoot->PipelinesManager()->ExecuateCurrentPipeline();
    mRenderSystemRoot->Devices()->PresentSwapChain();
    mRenderSystemRoot->DrawCallsPool()->ClearAllDrawCallsInPipes();
}

void RenderSystem::Destory()
{
    mRenderSystemRoot->CleanAndStop();
    delete mRenderSystemRoot;
}
