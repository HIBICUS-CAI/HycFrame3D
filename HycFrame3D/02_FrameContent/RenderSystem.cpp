#include "RenderSystem.h"
#include "RSRoot_DX11.h"
#include "RSPipelinesManager.h"
#include "RSDevices.h"
#include "RSDrawCallsPool.h"
#include "RSCamera.h"
#include "RSCamerasContainer.h"
#include "WM_Interface.h"
#include "BasicRSPipeline.h"

RenderSystem::RenderSystem(SystemExecutive* _sysExecutive) :
    System("render-system", _sysExecutive),
    mRenderSystemRoot(nullptr)
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

        CreateBasicPipeline();

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
    }

    return true;
}

void RenderSystem::Run(Timer& _timer)
{
    mRenderSystemRoot = GetRSRoot_DX11_Singleton();
#ifdef _DEBUG
    assert(mRenderSystemRoot);
#endif // _DEBUG

    mRenderSystemRoot->PipelinesManager()->ExecuateCurrentPipeline();
    mRenderSystemRoot->Devices()->PresentSwapChain();
    mRenderSystemRoot->DrawCallsPool()->ClearAllDrawCallsInPipes();
}

void RenderSystem::Destory()
{
    mRenderSystemRoot->CleanAndStop();
    delete mRenderSystemRoot;
}
