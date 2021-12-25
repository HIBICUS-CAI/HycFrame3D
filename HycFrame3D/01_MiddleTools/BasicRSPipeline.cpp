#include "BasicRSPipeline.h"
#include <string>
#include <vector>
#include "RSRoot_DX11.h"
#include "RSDevices.h"
#include "RSPipeline.h"
#include "RSPipelinesManager.h"

#define RS_RELEASE(p) { if (p) { (p)->Release(); (p)=nullptr; } }
static RSRoot_DX11* g_Root = nullptr;
static RSPipeline* g_TempPipeline = nullptr;
static D3D11_VIEWPORT g_ViewPort = {};

bool CreateBasicPipeline()
{
    g_Root = GetRSRoot_DX11_Singleton();

    std::string name = "light-pipeline";
    g_TempPipeline = new RSPipeline(name);
    g_TempPipeline->StartPipelineAssembly();
    g_TempPipeline->FinishPipelineAssembly();

    if (!g_TempPipeline->InitAllTopics(g_Root->Devices(), true))
    {
        return false;
    }

    name = g_TempPipeline->GetPipelineName();
    g_Root->PipelinesManager()->AddPipeline(name, g_TempPipeline);
    g_Root->PipelinesManager()->SetPipeline(name);
    g_Root->PipelinesManager()->ProcessNextPipeline();

    g_ViewPort.Width = 1280.f;
    g_ViewPort.Height = 720.f;
    g_ViewPort.MinDepth = 0.f;
    g_ViewPort.MaxDepth = 1.f;
    g_ViewPort.TopLeftX = 0.f;
    g_ViewPort.TopLeftY = 0.f;

    return true;
}
