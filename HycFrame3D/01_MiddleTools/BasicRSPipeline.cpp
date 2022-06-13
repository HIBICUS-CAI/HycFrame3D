#include "BasicRSPipeline.h"
#include "WM_Interface.h"
#include <string>
#include <vector>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <ctime>
#include <cstdlib>
#include "RSRoot_DX11.h"
#include "RSDevices.h"
#include "RSTopic.h"
#include "RSPipeline.h"
#include "RSPipelinesManager.h"
#include "RSDrawCallsPool.h"
#include "RSMeshHelper.h"
#include "RSCamerasContainer.h"
#include "RSCamera.h"
#include "RSLight.h"
#include "RSLightsContainer.h"
#include "RSParticleEmitter.h"
#include "RSParticlesContainer.h"
#include "RSResourceManager.h"
#include "RSStaticResources.h"
#include "RSShaderCompile.h"
#include "RSUtilityFunctions.h"
#include "DDSTextureLoader11.h"
#include "WICTextureLoader11.h"
#include "JsonHelper.h"

//#define ONE_PASS_PER_TOPIC

#define RS_RELEASE(p) { if (p) { (p)->Release(); (p)=nullptr; } }
#define RS_ADD(p) { if(p) { p->AddRef(); } }
static RSRoot_DX11* g_Root = nullptr;
static RSPass_PriticleSetUp* g_ParticleSetUpPass = nullptr;
static RSPipeline* g_BasicPipeline = nullptr;
static RSPipeline* g_SimplePipeline = nullptr;
static D3D11_VIEWPORT g_ViewPort = {};

static float g_DeltaTimeInSecond = 0.f;

// TEMP FOR IBL
static ID3D11ShaderResourceView* g_IblBrdfSrv = nullptr;
static ID3D11ShaderResourceView* g_EnviMapSrv = nullptr;
static ID3D11ShaderResourceView* g_DiffMapSrv = nullptr;
static ID3D11ShaderResourceView* g_SpecMapSrv = nullptr;
// TEMP FOR IBL

enum class SAMPLER_LEVEL
{
    POINT = 0,
    BILINEAR = 1,
    ANISO_8X = 2,
    ANISO_16X = 3
};

struct RENDER_EFFECT_CONFIG
{
    float mSsaoRadius = 0.5f;
    float mSsaoStart = 0.2f;
    float mSsaoEnd = 1.f;
    float mSsaoEpsilon = 0.05f;
    UINT mSsaoBlurCount = 4;
    SAMPLER_LEVEL mSamplerLevel = SAMPLER_LEVEL::ANISO_16X;
    bool mParticleOff = false;
};

static RENDER_EFFECT_CONFIG g_RenderEffectConfig = {};

void SetPipeLineDeltaTime(float _deltaMilliSecond)
{
    g_DeltaTimeInSecond = _deltaMilliSecond / 1000.f;
}

bool CreateBasicPipeline()
{
    {
        JsonFile config = {};
        LoadJsonFile(&config, ".\\Assets\\Configs\\render-effect-config.json");
        if (config.HasParseError()) { return false; }
        g_RenderEffectConfig.mSsaoRadius = config["ssao-radius"].GetFloat();
        g_RenderEffectConfig.mSsaoStart = config["ssao-start"].GetFloat();
        g_RenderEffectConfig.mSsaoEnd = config["ssao-end"].GetFloat();
        g_RenderEffectConfig.mSsaoEpsilon = config["ssao-epsilon"].GetFloat();
        g_RenderEffectConfig.mSsaoBlurCount =
            config["ssao-blur-loop-count"].GetUint();
        g_RenderEffectConfig.mSamplerLevel =
            (SAMPLER_LEVEL)(config["filter-level"].GetUint());
        g_RenderEffectConfig.mParticleOff =
            config["particle-off"].GetBool();

        if ((UINT)g_RenderEffectConfig.mSamplerLevel < 0 ||
            (UINT)g_RenderEffectConfig.mSamplerLevel > 3)
        {
            return false;
        }
    }

    g_Root = GetRSRoot_DX11_Singleton();
    std::string name = "";

#ifdef ONE_PASS_PER_TOPIC
    name = "mrt-pass";
    RSPass_MRT* mrt = new RSPass_MRT(name, PASS_TYPE::RENDER, g_Root);
    mrt->SetExecuateOrder(1);

    name = "mrt-topic";
    RSTopic* mrt_topic = new RSTopic(name);
    mrt_topic->StartTopicAssembly();
    mrt_topic->InsertPass(mrt);
    mrt_topic->SetExecuateOrder(1);
    mrt_topic->FinishTopicAssembly();

    name = "basic-ssao";
    RSPass_Ssao* ssao = new RSPass_Ssao(
        name, PASS_TYPE::RENDER, g_Root);
    ssao->SetExecuateOrder(2);

    name = "kbblur-ssao";
    RSPass_KBBlur* kbblur = new RSPass_KBBlur(
        name, PASS_TYPE::COMPUTE, g_Root);
    kbblur->SetExecuateOrder(3);

    name = "ssao-topic1";
    RSTopic* ssao_topic1 = new RSTopic(name);
    ssao_topic1->StartTopicAssembly();
    ssao_topic1->InsertPass(ssao);
    ssao_topic1->SetExecuateOrder(2);
    ssao_topic1->FinishTopicAssembly();
    name = "ssao-topic2";
    RSTopic* ssao_topic2 = new RSTopic(name);
    ssao_topic2->StartTopicAssembly();
    ssao_topic2->InsertPass(kbblur);
    ssao_topic2->SetExecuateOrder(3);
    ssao_topic2->FinishTopicAssembly();

    name = "basic-shadowmap";
    RSPass_Shadow* shadow = new RSPass_Shadow(
        name, PASS_TYPE::RENDER, g_Root);
    shadow->SetExecuateOrder(1);

    name = "shadowmap-topic";
    RSTopic* shadow_topic = new RSTopic(name);
    shadow_topic->StartTopicAssembly();
    shadow_topic->InsertPass(shadow);
    shadow_topic->SetExecuateOrder(4);
    shadow_topic->FinishTopicAssembly();

    name = "defered-light";
    RSPass_Defered* defered = new RSPass_Defered(
        name, PASS_TYPE::RENDER, g_Root);
    defered->SetExecuateOrder(1);

    name = "defered-light-topic";
    RSTopic* defered_topic = new RSTopic(name);
    defered_topic->StartTopicAssembly();
    defered_topic->InsertPass(defered);
    defered_topic->SetExecuateOrder(5);
    defered_topic->FinishTopicAssembly();

    name = "sky-skysphere";
    RSPass_SkyShpere* skysphere = new RSPass_SkyShpere(
        name, PASS_TYPE::RENDER, g_Root);
    skysphere->SetExecuateOrder(1);

    name = "skysphere-topic";
    RSTopic* sky_topic = new RSTopic(name);
    sky_topic->StartTopicAssembly();
    sky_topic->InsertPass(skysphere);
    sky_topic->SetExecuateOrder(6);
    sky_topic->FinishTopicAssembly();

    name = "bloomdraw-pass";
    RSPass_Bloom* bloomdraw = new RSPass_Bloom(
        name, PASS_TYPE::RENDER, g_Root);
    bloomdraw->SetExecuateOrder(1);

    name = "bloomblur-pass";
    RSPass_Blur* bloomblur = new RSPass_Blur(
        name, PASS_TYPE::COMPUTE, g_Root);
    bloomblur->SetExecuateOrder(2);

    name = "bloomblend-pass";
    RSPass_BloomOn* bloomblend = new RSPass_BloomOn(
        name, PASS_TYPE::RENDER, g_Root);
    bloomblend->SetExecuateOrder(3);

    name = "bloom-topic1";
    RSTopic* bloom_topic1 = new RSTopic(name);
    bloom_topic1->StartTopicAssembly();
    bloom_topic1->InsertPass(bloomdraw);
    bloom_topic1->SetExecuateOrder(7);
    bloom_topic1->FinishTopicAssembly();
    name = "bloom-topic2";
    RSTopic* bloom_topic2 = new RSTopic(name);
    bloom_topic2->StartTopicAssembly();
    bloom_topic2->InsertPass(bloomblur);
    bloom_topic2->SetExecuateOrder(8);
    bloom_topic2->FinishTopicAssembly();
    name = "bloom-topic3";
    RSTopic* bloom_topic3 = new RSTopic(name);
    bloom_topic3->StartTopicAssembly();
    bloom_topic3->InsertPass(bloomblend);
    bloom_topic3->SetExecuateOrder(9);
    bloom_topic3->FinishTopicAssembly();

    RSTopic* particle_topic1 = nullptr;
    RSTopic* particle_topic2 = nullptr;
    RSTopic* particle_topic3 = nullptr;
    if (!g_RenderEffectConfig.mParticleOff)
    {
        name = "particle-setup-pass";
        RSPass_PriticleSetUp* ptcsetup = new RSPass_PriticleSetUp(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptcsetup->SetExecuateOrder(1);

        name = "particle-emit-simulate-pass";
        RSPass_PriticleEmitSimulate* ptcemitsimul = new RSPass_PriticleEmitSimulate(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptcemitsimul->SetExecuateOrder(2);

        name = "particle-tile-render-pass";
        RSPass_PriticleTileRender* ptctile = new RSPass_PriticleTileRender(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptctile->SetExecuateOrder(3);

        name = "paricle-topic1";
        particle_topic1 = new RSTopic(name);
        particle_topic1->StartTopicAssembly();
        particle_topic1->InsertPass(ptcsetup);
        particle_topic1->SetExecuateOrder(10);
        particle_topic1->FinishTopicAssembly();
        name = "paricle-topic2";
        particle_topic2 = new RSTopic(name);
        particle_topic2->StartTopicAssembly();
        particle_topic2->InsertPass(ptcemitsimul);
        particle_topic2->SetExecuateOrder(11);
        particle_topic2->FinishTopicAssembly();
        name = "paricle-topic3";
        particle_topic3 = new RSTopic(name);
        particle_topic3->StartTopicAssembly();
        particle_topic3->InsertPass(ptctile);
        particle_topic3->SetExecuateOrder(12);
        particle_topic3->FinishTopicAssembly();
    }

    name = "sprite-ui";
    RSPass_Sprite* sprite = new RSPass_Sprite(
        name, PASS_TYPE::RENDER, g_Root);
    sprite->SetExecuateOrder(1);

    name = "sprite-topic";
    RSTopic* sprite_topic = new RSTopic(name);
    sprite_topic->StartTopicAssembly();
    sprite_topic->InsertPass(sprite);
    sprite_topic->SetExecuateOrder(13);
    sprite_topic->FinishTopicAssembly();

    name = "light-pipeline";
    g_BasicPipeline = new RSPipeline(name);
    g_BasicPipeline->StartPipelineAssembly();
    g_BasicPipeline->InsertTopic(mrt_topic);
    g_BasicPipeline->InsertTopic(ssao_topic1);
    g_BasicPipeline->InsertTopic(ssao_topic2);
    g_BasicPipeline->InsertTopic(shadow_topic);
    g_BasicPipeline->InsertTopic(defered_topic);
    g_BasicPipeline->InsertTopic(sky_topic);
    g_BasicPipeline->InsertTopic(bloom_topic1);
    g_BasicPipeline->InsertTopic(bloom_topic2);
    g_BasicPipeline->InsertTopic(bloom_topic3);
    if (!g_RenderEffectConfig.mParticleOff)
    {
        g_BasicPipeline->InsertTopic(particle_topic1);
        g_BasicPipeline->InsertTopic(particle_topic2);
        g_BasicPipeline->InsertTopic(particle_topic3);
    }
    g_BasicPipeline->InsertTopic(sprite_topic);
    g_BasicPipeline->FinishPipelineAssembly();
#else
    name = "mrt-pass";
    RSPass_MRT* mrt = new RSPass_MRT(name, PASS_TYPE::RENDER, g_Root);
    mrt->SetExecuateOrder(1);

    name = "mrt-topic";
    RSTopic* mrt_topic = new RSTopic(name);
    mrt_topic->StartTopicAssembly();
    mrt_topic->InsertPass(mrt);
    mrt_topic->SetExecuateOrder(1);
    mrt_topic->FinishTopicAssembly();

    name = "basic-ssao";
    RSPass_Ssao* ssao = new RSPass_Ssao(
        name, PASS_TYPE::RENDER, g_Root);
    ssao->SetExecuateOrder(2);

    name = "kbblur-ssao";
    RSPass_KBBlur* kbblur = new RSPass_KBBlur(
        name, PASS_TYPE::COMPUTE, g_Root);
    kbblur->SetExecuateOrder(3);

    name = "ssao-topic";
    RSTopic* ssao_topic = new RSTopic(name);
    ssao_topic->StartTopicAssembly();
    ssao_topic->InsertPass(ssao);
    ssao_topic->InsertPass(kbblur);
    ssao_topic->SetExecuateOrder(2);
    ssao_topic->FinishTopicAssembly();

    name = "basic-shadowmap";
    RSPass_Shadow* shadow = new RSPass_Shadow(
        name, PASS_TYPE::RENDER, g_Root);
    shadow->SetExecuateOrder(1);

    name = "shadowmap-topic";
    RSTopic* shadow_topic = new RSTopic(name);
    shadow_topic->StartTopicAssembly();
    shadow_topic->InsertPass(shadow);
    shadow_topic->SetExecuateOrder(3);
    shadow_topic->FinishTopicAssembly();

    name = "defered-light";
    RSPass_Defered* defered = new RSPass_Defered(
        name, PASS_TYPE::RENDER, g_Root);
    defered->SetExecuateOrder(1);

    name = "defered-light-topic";
    RSTopic* defered_topic = new RSTopic(name);
    defered_topic->StartTopicAssembly();
    defered_topic->InsertPass(defered);
    defered_topic->SetExecuateOrder(4);
    defered_topic->FinishTopicAssembly();

    name = "sky-skysphere";
    RSPass_SkyShpere* skysphere = new RSPass_SkyShpere(
        name, PASS_TYPE::RENDER, g_Root);
    skysphere->SetExecuateOrder(1);

    name = "skysphere-topic";
    RSTopic* sky_topic = new RSTopic(name);
    sky_topic->StartTopicAssembly();
    sky_topic->InsertPass(skysphere);
    sky_topic->SetExecuateOrder(5);
    sky_topic->FinishTopicAssembly();

    name = "bloomdraw-pass";
    RSPass_Bloom* bloomdraw = new RSPass_Bloom(
        name, PASS_TYPE::RENDER, g_Root);
    bloomdraw->SetExecuateOrder(1);

    name = "bloomblur-pass";
    RSPass_Blur* bloomblur = new RSPass_Blur(
        name, PASS_TYPE::COMPUTE, g_Root);
    bloomblur->SetExecuateOrder(2);

    name = "bloomblend-pass";
    RSPass_BloomOn* bloomblend = new RSPass_BloomOn(
        name, PASS_TYPE::RENDER, g_Root);
    bloomblend->SetExecuateOrder(3);

    name = "bloom-topic";
    RSTopic* bloom_topic = new RSTopic(name);
    bloom_topic->StartTopicAssembly();
    bloom_topic->InsertPass(bloomdraw);
    bloom_topic->InsertPass(bloomblur);
    bloom_topic->InsertPass(bloomblend);
    bloom_topic->SetExecuateOrder(6);
    bloom_topic->FinishTopicAssembly();

    RSTopic* particle_topic = nullptr;
    if (!g_RenderEffectConfig.mParticleOff)
    {
        name = "particle-setup-pass";
        RSPass_PriticleSetUp* ptcsetup = new RSPass_PriticleSetUp(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptcsetup->SetExecuateOrder(1);

        name = "particle-emit-simulate-pass";
        RSPass_PriticleEmitSimulate* ptcemitsimul = new RSPass_PriticleEmitSimulate(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptcemitsimul->SetExecuateOrder(2);

        name = "particle-tile-render-pass";
        RSPass_PriticleTileRender* ptctile = new RSPass_PriticleTileRender(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptctile->SetExecuateOrder(3);

        name = "paricle-topic";
        particle_topic = new RSTopic(name);
        particle_topic->StartTopicAssembly();
        particle_topic->InsertPass(ptcsetup);
        particle_topic->InsertPass(ptcemitsimul);
        particle_topic->InsertPass(ptctile);
        particle_topic->SetExecuateOrder(7);
        particle_topic->FinishTopicAssembly();
    }

    name = "sprite-ui";
    RSPass_Sprite* sprite = new RSPass_Sprite(
        name, PASS_TYPE::RENDER, g_Root);
    sprite->SetExecuateOrder(1);

    name = "sprite-topic";
    RSTopic* sprite_topic = new RSTopic(name);
    sprite_topic->StartTopicAssembly();
    sprite_topic->InsertPass(sprite);
    sprite_topic->SetExecuateOrder(8);
    sprite_topic->FinishTopicAssembly();

    name = "light-pipeline";
    g_BasicPipeline = new RSPipeline(name);
    g_BasicPipeline->StartPipelineAssembly();
    g_BasicPipeline->InsertTopic(mrt_topic);
    g_BasicPipeline->InsertTopic(ssao_topic);
    g_BasicPipeline->InsertTopic(shadow_topic);
    g_BasicPipeline->InsertTopic(defered_topic);
    g_BasicPipeline->InsertTopic(sky_topic);
    g_BasicPipeline->InsertTopic(bloom_topic);
    if (!g_RenderEffectConfig.mParticleOff)
    {
        g_BasicPipeline->InsertTopic(particle_topic);
    }
    g_BasicPipeline->InsertTopic(sprite_topic);
    g_BasicPipeline->FinishPipelineAssembly();
#endif // ONE_PASS_PER_TOPIC

    if (!g_BasicPipeline->InitAllTopics(g_Root->Devices()))
    {
        return false;
    }

    name = g_BasicPipeline->GetPipelineName();
    g_Root->PipelinesManager()->AddPipeline(name, g_BasicPipeline);
    g_Root->PipelinesManager()->SetPipeline(name);
    g_Root->PipelinesManager()->ProcessNextPipeline();

    name = "simp-mrt-pass";
    RSPass_MRT* simp_mrt = mrt->ClonePass();
    simp_mrt->SetExecuateOrder(1);

    name = "simp-mrt-topic";
    RSTopic* simp_mrt_topic = new RSTopic(name);
    simp_mrt_topic->StartTopicAssembly();
    simp_mrt_topic->InsertPass(simp_mrt);
    simp_mrt_topic->SetExecuateOrder(1);
    simp_mrt_topic->FinishTopicAssembly();

    name = "simp-basic-ssao";
    RSPass_Ssao* simp_ssao = ssao->ClonePass();
    simp_ssao->SetExecuateOrder(1);

    name = "simp-ssao-topic";
    RSTopic* simp_ssao_topic = new RSTopic(name);
    simp_ssao_topic->StartTopicAssembly();
    simp_ssao_topic->InsertPass(simp_ssao);
    simp_ssao_topic->SetExecuateOrder(2);
    simp_ssao_topic->FinishTopicAssembly();

    name = "simp-lit-shadowmap";
    RSPass_SimpleLight* simp_lit = new RSPass_SimpleLight(
        name, PASS_TYPE::RENDER, g_Root);
    simp_lit->SetExecuateOrder(1);

    name = "simp-lit-topic";
    RSTopic* simp_lit_topic = new RSTopic(name);
    simp_lit_topic->StartTopicAssembly();
    simp_lit_topic->InsertPass(simp_lit);
    simp_lit_topic->SetExecuateOrder(3);
    simp_lit_topic->FinishTopicAssembly();

    name = "simp-sprite-ui";
    RSPass_Sprite* simp_sprite = sprite->ClonePass();
    simp_sprite->SetExecuateOrder(1);

    name = "simp-sprite-topic";
    RSTopic* simp_sprite_topic = new RSTopic(name);
    simp_sprite_topic->StartTopicAssembly();
    simp_sprite_topic->InsertPass(simp_sprite);
    simp_sprite_topic->SetExecuateOrder(4);
    simp_sprite_topic->FinishTopicAssembly();

    name = "simple-pipeline";
    g_SimplePipeline = new RSPipeline(name);
    g_SimplePipeline->StartPipelineAssembly();
    g_SimplePipeline->InsertTopic(simp_mrt_topic);
    g_SimplePipeline->InsertTopic(simp_ssao_topic);
    g_SimplePipeline->InsertTopic(simp_lit_topic);
    g_SimplePipeline->InsertTopic(simp_sprite_topic);
    g_SimplePipeline->FinishPipelineAssembly();

    if (!g_SimplePipeline->InitAllTopics(g_Root->Devices()))
    {
        return false;
    }

    name = g_SimplePipeline->GetPipelineName();
    g_Root->PipelinesManager()->AddPipeline(name, g_SimplePipeline);
    g_Root->PipelinesManager()->SetPipeline(name);
    g_Root->PipelinesManager()->ProcessNextPipeline();

    g_ViewPort.Width = (float)g_Root->Devices()->GetCurrWndWidth();
    g_ViewPort.Height = (float)g_Root->Devices()->GetCurrWndHeight();
    g_ViewPort.MinDepth = 0.f;
    g_ViewPort.MaxDepth = 1.f;
    g_ViewPort.TopLeftX = 0.f;
    g_ViewPort.TopLeftY = 0.f;

    return true;
}

RSPass_MRT::RSPass_MRT(std::string& _name, PASS_TYPE _type,
    RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mDrawCallType(DRAWCALL_TYPE::OPACITY), mDrawCallPipe(nullptr),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mNDPixelShader(nullptr), mAniVertexShader(nullptr),
    mViewProjStructedBuffer(nullptr),
    mViewProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
    mBonesStructedBuffer(nullptr),
    mBonesStructedBufferSrv(nullptr),
    mLinearSampler(nullptr), mDepthDsv(nullptr),
    mDiffuseRtv(nullptr), mNormalRtv(nullptr),
    mRSCameraInfo(nullptr), mWorldPosRtv(nullptr),
    mDiffAlbeRtv(nullptr), mFresShinRtv(nullptr)
{

}

RSPass_MRT::RSPass_MRT(const RSPass_MRT& _source) :
    RSPass_Base(_source),
    mDrawCallType(_source.mDrawCallType),
    mDrawCallPipe(_source.mDrawCallPipe),
    mVertexShader(_source.mVertexShader),
    mAniVertexShader(_source.mAniVertexShader),
    mPixelShader(_source.mPixelShader),
    mNDPixelShader(_source.mNDPixelShader),
    mViewProjStructedBuffer(_source.mViewProjStructedBuffer),
    mInstanceStructedBuffer(_source.mInstanceStructedBuffer),
    mBonesStructedBuffer(_source.mBonesStructedBuffer),
    mViewProjStructedBufferSrv(_source.mViewProjStructedBufferSrv),
    mInstanceStructedBufferSrv(_source.mInstanceStructedBufferSrv),
    mBonesStructedBufferSrv(_source.mBonesStructedBufferSrv),
    mLinearSampler(_source.mLinearSampler),
    mDiffuseRtv(_source.mDiffuseRtv),
    mNormalRtv(_source.mNormalRtv),
    mDepthDsv(_source.mDepthDsv),
    mRSCameraInfo(_source.mRSCameraInfo),
    mWorldPosRtv(_source.mWorldPosRtv),
    mDiffAlbeRtv(_source.mDiffAlbeRtv),
    mFresShinRtv(_source.mFresShinRtv)
{
    if (mHasBeenInited)
    {
        RS_ADD(mVertexShader);
        RS_ADD(mAniVertexShader);
        RS_ADD(mPixelShader);
        RS_ADD(mNDPixelShader);
        RS_ADD(mViewProjStructedBuffer);
        RS_ADD(mViewProjStructedBufferSrv);
        RS_ADD(mInstanceStructedBuffer);
        RS_ADD(mInstanceStructedBufferSrv);
        RS_ADD(mBonesStructedBuffer);
        RS_ADD(mBonesStructedBufferSrv);
        RS_ADD(mLinearSampler);
    }
}

RSPass_MRT::~RSPass_MRT()
{

}

RSPass_MRT* RSPass_MRT::ClonePass()
{
    return new RSPass_MRT(*this);
}

bool RSPass_MRT::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallType = DRAWCALL_TYPE::OPACITY;
    mDrawCallPipe = g_Root->DrawCallsPool()->GetDrawCallsPipe(mDrawCallType);

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->CamerasContainer()->GetRSCameraInfo(name);

    mHasBeenInited = true;

    return true;
}

void RSPass_MRT::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mAniVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mNDPixelShader);
    RS_RELEASE(mViewProjStructedBuffer);
    RS_RELEASE(mViewProjStructedBufferSrv);
    RS_RELEASE(mInstanceStructedBuffer);
    RS_RELEASE(mInstanceStructedBufferSrv);
    RS_RELEASE(mBonesStructedBuffer);
    RS_RELEASE(mBonesStructedBufferSrv);
    RS_RELEASE(mLinearSampler);

    std::string name = "mrt-depth";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "mrt-normal";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "mrt-diffuse";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "mrt-worldpos";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "mrt-diffuse-albedo";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "mrt-fresnel-shinese";
    g_Root->ResourceManager()->DeleteResource(name);
}

void RSPass_MRT::ExecuatePass()
{
    ID3D11RenderTargetView* rtvnull = nullptr;
    static ID3D11RenderTargetView* mrt[] = { mDiffuseRtv,
        mNormalRtv,mWorldPosRtv,mDiffAlbeRtv,mFresShinRtv };
    STContext()->OMSetRenderTargets(5, mrt, mDepthDsv);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->ClearRenderTargetView(
        mDiffuseRtv, DirectX::Colors::DarkGreen);
    STContext()->ClearRenderTargetView(
        mNormalRtv, DirectX::Colors::Transparent);
    STContext()->ClearRenderTargetView(
        mWorldPosRtv, DirectX::Colors::Transparent);
    STContext()->ClearRenderTargetView(
        mDiffAlbeRtv, DirectX::Colors::Transparent);
    STContext()->ClearRenderTargetView(
        mFresShinRtv, DirectX::Colors::Transparent);
    STContext()->ClearDepthStencilView(mDepthDsv, D3D11_CLEAR_DEPTH, 1.f, 0);
    //STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);

    STContext()->PSSetSamplers(0, 1, &mLinearSampler);

    DirectX::XMMATRIX mat = {};
    DirectX::XMFLOAT4X4 flt44 = {};
    UINT stride = sizeof(VertexType::TangentVertex);
    UINT aniStride = sizeof(VertexType::AnimationVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    STContext()->Map(mViewProjStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    ViewProj* vp_data = (ViewProj*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mViewMat, mat);
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
    STContext()->Unmap(mViewProjStructedBuffer, 0);

    STContext()->VSSetShaderResources(0, 1, &mViewProjStructedBufferSrv);

    static std::string A_NAME = "AnimationVertex";
    static const auto ANIMAT_LAYOUT =
        GetRSRoot_DX11_Singleton()->StaticResources()->
        GetStaticInputLayout(A_NAME);

    for (auto& call : mDrawCallPipe->mDatas)
    {
        if (call.mMeshData.mLayout == ANIMAT_LAYOUT)
        {
            STContext()->VSSetShader(mAniVertexShader, nullptr, 0);
            STContext()->IASetVertexBuffers(
                0, 1, &call.mMeshData.mVertexBuffer, &aniStride, &offset);

            STContext()->Map(mBonesStructedBuffer, 0,
                D3D11_MAP_WRITE_DISCARD, 0, &msr);
            DirectX::XMFLOAT4X4* b_data = (DirectX::XMFLOAT4X4*)msr.pData;
            void* boneData = call.mInstanceData.mBonesDataPtr;
            std::vector<std::vector<RS_SUBMESH_BONE_DATA>>* bones = nullptr;
            bones = static_cast<decltype(bones)>(boneData);
            // TEMP-----------------------
            auto boneInsSize = bones->size();
            // TEMP-----------------------
            for (size_t i = 0; i < boneInsSize; i++)
            {
                for (size_t j = 0; j < MAX_STRUCTURED_BUFFER_SIZE; j++)
                {
                    if (j < (*bones)[i].size())
                    {
                        DirectX::XMMATRIX trans = DirectX::XMLoadFloat4x4(
                            &((*bones)[i][j].mBoneTransform));
                        trans = DirectX::XMMatrixTranspose(trans);
                        DirectX::XMStoreFloat4x4(
                            b_data + i * MAX_STRUCTURED_BUFFER_SIZE + j,
                            trans);
                    }
                    else
                    {
                        DirectX::XMStoreFloat4x4(
                            b_data + i * MAX_STRUCTURED_BUFFER_SIZE + j,
                            DirectX::XMMatrixIdentity());
                    }
                }
            }
            STContext()->Unmap(mBonesStructedBuffer, 0);

            STContext()->VSSetShaderResources(2, 1,
                &mBonesStructedBufferSrv);
        }
        else
        {
            STContext()->VSSetShader(mVertexShader, nullptr, 0);
            STContext()->IASetVertexBuffers(
                0, 1, &call.mMeshData.mVertexBuffer, &stride, &offset);
        }

        auto vecPtr = call.mInstanceData.mDataPtr;
        auto size = vecPtr->size();
        STContext()->Map(mInstanceStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            mat = DirectX::XMLoadFloat4x4(&(*vecPtr)[i].mWorldMat);
            mat = DirectX::XMMatrixTranspose(mat);
            DirectX::XMStoreFloat4x4(&ins_data[i].mWorldMat, mat);
            ins_data[i].mMaterialData = (*vecPtr)[i].mMaterialData;
            ins_data[i].mCustomizedData1 = (*vecPtr)[i].mCustomizedData1;
            ins_data[i].mCustomizedData2 = (*vecPtr)[i].mCustomizedData2;
        }
        STContext()->Unmap(mInstanceStructedBuffer, 0);

        STContext()->IASetInputLayout(call.mMeshData.mLayout);
        STContext()->IASetPrimitiveTopology(call.mMeshData.mTopologyType);
        /*STContext()->IASetVertexBuffers(
            0, 1, &call.mMeshData.mVertexBuffer, &stride, &offset);*/
        STContext()->IASetIndexBuffer(
            call.mMeshData.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        STContext()->VSSetShaderResources(1, 1, &mInstanceStructedBufferSrv);
        STContext()->PSSetShaderResources(0, 1, &(call.mTextureDatas[0].mSrv));
        if (call.mTextureDatas[1].mUse)
        {
            STContext()->PSSetShaderResources(
                1, 1, &(call.mTextureDatas[1].mSrv));
        }

        STContext()->DrawIndexedInstanced(
            call.mMeshData.mIndexCount,
            (UINT)call.mInstanceData.mDataPtr->size(), 0, 0, 0);

        if (call.mMeshData.mLayout == ANIMAT_LAYOUT)
        {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            STContext()->VSSetShaderResources(2, 1,
                &nullSRV);
        }
    }

    STContext()->OMSetRenderTargets(1, &rtvnull, nullptr);
}

bool RSPass_MRT::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\mrt_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    D3D_SHADER_MACRO macro[] =
    { "ANIMATION_VERTEX","1",nullptr,nullptr };
    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\mrt_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob, macro);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mAniVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\mrt_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\mrt_nd_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mNDPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_MRT::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bdc = {};

    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_DYNAMIC;
    bdc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bdc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bdc.ByteWidth = MAX_INSTANCE_SIZE * sizeof(RS_INSTANCE_DATA);
    bdc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bdc.StructureByteStride = sizeof(RS_INSTANCE_DATA);
    hr = Device()->CreateBuffer(&bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE *
        (UINT)sizeof(DirectX::XMFLOAT4X4);
    bdc.StructureByteStride = sizeof(DirectX::XMFLOAT4X4);
    hr = Device()->CreateBuffer(&bdc, nullptr, &mBonesStructedBuffer);

    bdc.ByteWidth = sizeof(ViewProj);
    bdc.StructureByteStride = sizeof(ViewProj);
    hr = Device()->CreateBuffer(&bdc, nullptr, &mViewProjStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_MRT::CreateViews()
{
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC texDesc = {};
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    ID3D11ShaderResourceView* srv = nullptr;

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
    hr = Device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &srvDesc, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE;
    hr = Device()->CreateShaderResourceView(
        mBonesStructedBuffer,
        &srvDesc, &mBonesStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = 1;
    hr = Device()->CreateShaderResourceView(
        mViewProjStructedBuffer,
        &srvDesc, &mViewProjStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&texDesc, sizeof(texDesc));
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    RS_RESOURCE_INFO dti = {};
    std::string name = "";

    ID3D11Texture2D* texture = nullptr;
    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth();
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    hr = Device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    hr = Device()->CreateDepthStencilView(
        texture, &dsvDesc, &mDepthDsv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(
        texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-depth";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mDsv = mDepthDsv;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth();
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(texture, &rtvDesc, &mNormalRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_UINT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-normal";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = mNormalRtv;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndWidth();
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(texture, &rtvDesc, &mWorldPosRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-worldpos";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = mWorldPosRtv;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndWidth();
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(texture, &rtvDesc, &mDiffuseRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-diffuse";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = mDiffuseRtv;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndWidth();
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(
        texture, &rtvDesc, &mDiffAlbeRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-diffuse-albedo";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = mDiffAlbeRtv;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndWidth();
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(texture, &rtvDesc, &mFresShinRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-fresnel-shinese";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = mFresShinRtv;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    return true;
}

bool RSPass_MRT::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    auto filter = g_RenderEffectConfig.mSamplerLevel;
    switch (filter)
    {
    case SAMPLER_LEVEL::POINT:
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        break;
    case SAMPLER_LEVEL::BILINEAR:
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    case SAMPLER_LEVEL::ANISO_8X:
        sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampDesc.MaxAnisotropy = 8;
        break;
    case SAMPLER_LEVEL::ANISO_16X:
        sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampDesc.MaxAnisotropy = 16;
        break;
    default: return false;
    }
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = Device()->CreateSamplerState(&sampDesc, &mLinearSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_Ssao::RSPass_Ssao(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mRenderTargetView(nullptr),
    mSsaoInfoStructedBuffer(nullptr),
    mSsaoInfoStructedBufferSrv(nullptr),
    mNormalMapSrv(nullptr),
    mDepthMapSrv(nullptr),
    mRandomMapSrv(nullptr),
    mSamplePointClamp(nullptr), mSampleLinearClamp(nullptr),
    mSampleDepthMap(nullptr), mSampleLinearWrap(nullptr),
    mVertexBuffer(nullptr), mIndexBuffer(nullptr),
    mRSCameraInfo(nullptr), mCompressVertexShader(nullptr),
    mCompressPixelShader(nullptr), mNotCompressSrv(nullptr),
    mCompressRtv(nullptr)
{
    for (UINT i = 0; i < 14; i++)
    {
        mOffsetVec[i] = {};
    }
}

RSPass_Ssao::RSPass_Ssao(const RSPass_Ssao& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mRenderTargetView(_source.mRenderTargetView),
    mSamplePointClamp(_source.mSamplePointClamp),
    mSampleLinearClamp(_source.mSampleLinearClamp),
    mSampleDepthMap(_source.mSampleDepthMap),
    mSampleLinearWrap(_source.mSampleLinearWrap),
    mSsaoInfoStructedBuffer(_source.mSsaoInfoStructedBuffer),
    mSsaoInfoStructedBufferSrv(_source.mSsaoInfoStructedBufferSrv),
    mNormalMapSrv(_source.mNormalMapSrv),
    mDepthMapSrv(_source.mDepthMapSrv),
    mRandomMapSrv(_source.mRandomMapSrv),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer),
    mRSCameraInfo(_source.mRSCameraInfo),
    mCompressVertexShader(_source.mCompressVertexShader),
    mCompressPixelShader(_source.mCompressPixelShader),
    mNotCompressSrv(_source.mNotCompressSrv),
    mCompressRtv(_source.mCompressRtv)
{
    for (UINT i = 0; i < 14; i++)
    {
        mOffsetVec[i] = _source.mOffsetVec[i];
    }

    if (mHasBeenInited)
    {
        RS_ADD(mVertexShader);
        RS_ADD(mPixelShader);
        RS_ADD(mCompressVertexShader);
        RS_ADD(mCompressPixelShader);
        RS_ADD(mSamplePointClamp);
        RS_ADD(mSampleLinearClamp);
        RS_ADD(mSampleDepthMap);
        RS_ADD(mSampleLinearWrap);
        RS_ADD(mSsaoInfoStructedBuffer);
        RS_ADD(mSsaoInfoStructedBufferSrv);
        RS_ADD(mVertexBuffer);
        RS_ADD(mIndexBuffer);
    }
}

RSPass_Ssao::~RSPass_Ssao()
{

}

RSPass_Ssao* RSPass_Ssao::ClonePass()
{
    return new RSPass_Ssao(*this);
}

bool RSPass_Ssao::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateTextures()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    DirectX::XMFLOAT4 vec[14] = {};
    vec[0] = DirectX::XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
    vec[1] = DirectX::XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);
    vec[2] = DirectX::XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
    vec[3] = DirectX::XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);
    vec[4] = DirectX::XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
    vec[5] = DirectX::XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);
    vec[6] = DirectX::XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
    vec[7] = DirectX::XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);
    vec[8] = DirectX::XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
    vec[9] = DirectX::XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);
    vec[10] = DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
    vec[11] = DirectX::XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);
    vec[12] = DirectX::XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
    vec[13] = DirectX::XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);
    const int basic = 25;
    const int range = 75;
    for (int i = 0; i < 14; i++)
    {
        std::srand((unsigned int)std::time(nullptr) +
            (unsigned int)std::rand());
        float s = (float)(std::rand() % range + basic) / 100.f;
        DirectX::XMVECTOR v = DirectX::XMVector4Normalize(
            DirectX::XMLoadFloat4(&vec[i]));
        DirectX::XMStoreFloat4(&vec[i], v);
        vec[i].x *= s;
        vec[i].y *= s;
        vec[i].z *= s;
        mOffsetVec[i] = vec[i];
    }

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->CamerasContainer()->
        GetRSCameraInfo(name);

    mHasBeenInited = true;

    return true;
}

void RSPass_Ssao::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mCompressVertexShader);
    RS_RELEASE(mCompressPixelShader);
    RS_RELEASE(mSamplePointClamp);
    RS_RELEASE(mSampleLinearClamp);
    RS_RELEASE(mSampleDepthMap);
    RS_RELEASE(mSampleLinearWrap);
    RS_RELEASE(mSsaoInfoStructedBuffer);
    RS_RELEASE(mSsaoInfoStructedBufferSrv);
    RS_RELEASE(mVertexBuffer);
    RS_RELEASE(mIndexBuffer);

    std::string name = "random-tex-ssao";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "ssao-tex-ssao";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "ssao-tex-compress-ssao";
    g_Root->ResourceManager()->DeleteResource(name);
}

void RSPass_Ssao::ExecuatePass()
{
    ID3D11RenderTargetView* null = nullptr;
    ID3D11ShaderResourceView* srvnull = nullptr;
    STContext()->OMSetRenderTargets(1, &mRenderTargetView, nullptr);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);
    STContext()->RSSetState(nullptr);

    DirectX::XMMATRIX mat = {};
    DirectX::XMFLOAT4X4 flt44 = {};
    UINT stride = sizeof(VertexType::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};

    STContext()->Map(mSsaoInfoStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    SsaoInfo* ss_data = (SsaoInfo*)msr.pData;

    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&ss_data[0].mProj, mat);

    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&ss_data[0].mView, mat);

    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvProjMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&ss_data[0].mInvProj, mat);

    static DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);
    mat = DirectX::XMMatrixTranspose(
        DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat) * T);
    DirectX::XMStoreFloat4x4(&ss_data[0].mTexProj, mat);

    for (UINT i = 0; i < 14; i++)
    {
        ss_data[0].mOffsetVec[i] = mOffsetVec[i];
    }

    ss_data[0].mOcclusionRadius = g_RenderEffectConfig.mSsaoRadius;
    ss_data[0].mOcclusionFadeStart = g_RenderEffectConfig.mSsaoStart;
    ss_data[0].mOcclusionFadeEnd = g_RenderEffectConfig.mSsaoEnd;
    ss_data[0].mSurfaceEpsilon = g_RenderEffectConfig.mSsaoEpsilon;
    STContext()->Unmap(mSsaoInfoStructedBuffer, 0);

    STContext()->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    STContext()->IASetVertexBuffers(
        0, 1, &mVertexBuffer,
        &stride, &offset);
    STContext()->IASetIndexBuffer(
        mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    STContext()->VSSetShaderResources(
        0, 1, &mSsaoInfoStructedBufferSrv);
    STContext()->PSSetShaderResources(
        0, 1, &mSsaoInfoStructedBufferSrv);
    STContext()->PSSetShaderResources(
        1, 1, &mNormalMapSrv);
    STContext()->PSSetShaderResources(
        2, 1, &mDepthMapSrv);
    STContext()->PSSetShaderResources(
        3, 1, &mRandomMapSrv);

    STContext()->PSSetSamplers(0, 1, &mSamplePointClamp);
    STContext()->PSSetSamplers(1, 1, &mSampleLinearClamp);
    STContext()->PSSetSamplers(2, 1, &mSampleDepthMap);
    STContext()->PSSetSamplers(3, 1, &mSampleLinearWrap);

    STContext()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    STContext()->OMSetRenderTargets(1, &mCompressRtv, nullptr);
    STContext()->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    STContext()->IASetVertexBuffers(
        0, 1, &mVertexBuffer,
        &stride, &offset);
    STContext()->IASetIndexBuffer(
        mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->VSSetShader(mCompressVertexShader, nullptr, 0);
    STContext()->PSSetShader(mCompressPixelShader, nullptr, 0);
    STContext()->PSSetSamplers(0, 1, &mSampleLinearWrap);
    STContext()->PSSetShaderResources(
        0, 1, &mNotCompressSrv);

    STContext()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    STContext()->OMSetRenderTargets(1, &null, nullptr);
    STContext()->RSSetState(nullptr);
    STContext()->PSSetShaderResources(
        1, 1, &srvnull);
    STContext()->PSSetShaderResources(
        2, 1, &srvnull);
    STContext()->PSSetShaderResources(
        3, 1, &srvnull);
}

bool RSPass_Ssao::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\ssao_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\ssao_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\compress_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mCompressVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\compress_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mCompressPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Ssao::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bdc = {};

    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_DYNAMIC;
    bdc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bdc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bdc.ByteWidth = sizeof(SsaoInfo);
    bdc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bdc.StructureByteStride = sizeof(SsaoInfo);
    hr = Device()->CreateBuffer(
        &bdc, nullptr, &mSsaoInfoStructedBuffer);
    if (FAILED(hr)) { return false; }

    VertexType::TangentVertex v[4] = {};
    v[0].Position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
    v[1].Position = DirectX::XMFLOAT3(-1.0f, +1.0f, 0.0f);
    v[2].Position = DirectX::XMFLOAT3(+1.0f, +1.0f, 0.0f);
    v[3].Position = DirectX::XMFLOAT3(+1.0f, -1.0f, 0.0f);
    v[0].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    v[1].Normal = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    v[2].Normal = DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f);
    v[3].Normal = DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f);
    v[0].Tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    v[1].Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    v[2].Tangent = DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f);
    v[3].Tangent = DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f);
    v[0].TexCoord = DirectX::XMFLOAT2(0.0f, 1.0f);
    v[1].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
    v[2].TexCoord = DirectX::XMFLOAT2(1.0f, 0.0f);
    v[3].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);
    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_IMMUTABLE;
    bdc.ByteWidth = sizeof(VertexType::TangentVertex) * 4;
    bdc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bdc.CPUAccessFlags = 0;
    bdc.MiscFlags = 0;
    bdc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = Device()->CreateBuffer(
        &bdc, &vinitData, &mVertexBuffer);
    if (FAILED(hr)) { return false; }

    UINT indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };
    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_IMMUTABLE;
    bdc.ByteWidth = sizeof(UINT) * 6;
    bdc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bdc.CPUAccessFlags = 0;
    bdc.StructureByteStride = 0;
    bdc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    ZeroMemory(&iinitData, sizeof(iinitData));
    iinitData.pSysMem = indices;
    hr = Device()->CreateBuffer(
        &bdc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Ssao::CreateTextures()
{
    HRESULT hr = S_OK;
    std::string name = "";
    RS_RESOURCE_INFO dti = {};
    D3D11_TEXTURE2D_DESC texDesc = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    D3D11_SUBRESOURCE_DATA iniData = {};
    ZeroMemory(&texDesc, sizeof(texDesc));
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    ZeroMemory(&iniData, sizeof(iniData));

    ID3D11Texture2D* texture = nullptr;
    ID3D11RenderTargetView* rtv = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    ID3D11UnorderedAccessView* uav = nullptr;

    DirectX::PackedVector::XMCOLOR* random = nullptr;
    random = new DirectX::PackedVector::XMCOLOR[256 * 256];
    int basic = 1;
    int range = 100;
    DirectX::XMFLOAT3 v = { 0.f,0.f,0.f };
    std::srand((unsigned int)std::time(nullptr) +
        (unsigned int)std::rand());
    for (int i = 0; i < 256; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            v.x = (float)(std::rand() % range + basic) / 100.f;
            v.y = (float)(std::rand() % range + basic) / 100.f;
            v.z = (float)(std::rand() % range + basic) / 100.f;
            random[i * 256 + j] =
                DirectX::PackedVector::XMCOLOR(v.x, v.y, v.z, 0.f);
        }
    }

    iniData.SysMemPitch = 256 *
        sizeof(DirectX::PackedVector::XMCOLOR);
    iniData.pSysMem = random;

    texDesc.Width = 256;
    texDesc.Height = 256;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(
        &texDesc, &iniData, &texture);

    delete[] random;
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(texture,
        &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "random-tex-ssao";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth();
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags =
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(
        &texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(
        texture, &rtvDesc, &rtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(
        texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "ssao-tex-ssao";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = rtv;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth() / 2;
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight() / 2;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags =
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE |
        D3D11_BIND_UNORDERED_ACCESS;
    hr = Device()->CreateTexture2D(
        &texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(
        texture, &rtvDesc, &rtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(
        texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    hr = Device()->CreateUnorderedAccessView(
        texture, &uavDesc, &uav);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "ssao-tex-compress-ssao";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = rtv;
    dti.mSrv = srv;
    dti.mUav = uav;
    g_Root->ResourceManager()->AddResource(name, dti);

    return true;
}

bool RSPass_Ssao::CreateViews()
{
    std::string name = "random-tex-ssao";
    mRandomMapSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "mrt-normal";
    mNormalMapSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "mrt-depth";
    mDepthMapSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "ssao-tex-ssao";
    mRenderTargetView = g_Root->ResourceManager()->
        GetResourceInfo(name)->mRtv;
    mNotCompressSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "ssao-tex-compress-ssao";
    mCompressRtv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mRtv;

    D3D11_SHADER_RESOURCE_VIEW_DESC desSRV = {};
    HRESULT hr = S_OK;
    ZeroMemory(&desSRV, sizeof(desSRV));
    desSRV.Format = DXGI_FORMAT_UNKNOWN;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desSRV.Buffer.ElementWidth = 1;
    hr = Device()->CreateShaderResourceView(
        mSsaoInfoStructedBuffer,
        &desSRV, &mSsaoInfoStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Ssao::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC samDesc = {};
    ZeroMemory(&samDesc, sizeof(samDesc));

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &samDesc, &mSamplePointClamp);
    if (FAILED(hr)) { return false; }

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &samDesc, &mSampleLinearClamp);
    if (FAILED(hr)) { return false; }

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &samDesc, &mSampleDepthMap);
    if (FAILED(hr)) { return false; }

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &samDesc, &mSampleLinearWrap);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_KBBlur::RSPass_KBBlur(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mHoriBlurShader(nullptr), mVertBlurShader(nullptr),
    mSsaoTexUav(nullptr),
    mNormalMapSrv(nullptr),
    mDepthMapSrv(nullptr)
{

}

RSPass_KBBlur::RSPass_KBBlur(const RSPass_KBBlur& _source) :
    RSPass_Base(_source),
    mHoriBlurShader(_source.mHoriBlurShader),
    mVertBlurShader(_source.mVertBlurShader),
    mSsaoTexUav(_source.mSsaoTexUav),
    mNormalMapSrv(_source.mNormalMapSrv),
    mDepthMapSrv(_source.mDepthMapSrv)
{
    if (mHasBeenInited)
    {
        RS_ADD(mHoriBlurShader);
        RS_ADD(mVertBlurShader);
    }
}

RSPass_KBBlur::~RSPass_KBBlur()
{

}

RSPass_KBBlur* RSPass_KBBlur::ClonePass()
{
    return new RSPass_KBBlur(*this);
}

bool RSPass_KBBlur::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }

    mHasBeenInited = true;

    return true;
}

void RSPass_KBBlur::ReleasePass()
{
    RS_RELEASE(mHoriBlurShader);
    RS_RELEASE(mVertBlurShader);
}

void RSPass_KBBlur::ExecuatePass()
{
    ID3D11ShaderResourceView* srv[] =
    {
        mNormalMapSrv, mDepthMapSrv
    };
    static ID3D11UnorderedAccessView* nullUav = nullptr;
    static ID3D11ShaderResourceView* nullSrv[] =
    {
        nullptr, nullptr
    };

    static const UINT loopCount = g_RenderEffectConfig.mSsaoBlurCount;
    static UINT width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth() / 2;
    static UINT height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight() / 2;
    UINT dispatchVert = Tool::Align(width, 256) / 256;
    UINT dispatchHori = Tool::Align(height, 256) / 256;

    for (UINT i = 0; i < loopCount; i++)
    {
        STContext()->CSSetShader(mHoriBlurShader, nullptr, 0);
        STContext()->CSSetUnorderedAccessViews(0, 1,
            &mSsaoTexUav, nullptr);
        STContext()->CSSetShaderResources(0, 2, srv);
        STContext()->Dispatch(dispatchVert, height, 1);
        STContext()->CSSetUnorderedAccessViews(0, 1,
            &nullUav, nullptr);
        STContext()->CSSetShaderResources(0, 2, nullSrv);

        STContext()->CSSetShader(mVertBlurShader, nullptr, 0);
        STContext()->CSSetUnorderedAccessViews(0, 1,
            &mSsaoTexUav, nullptr);
        STContext()->CSSetShaderResources(0, 2, srv);
        STContext()->Dispatch(width, dispatchHori, 1);
        STContext()->CSSetUnorderedAccessViews(0, 1,
            &nullUav, nullptr);
        STContext()->CSSetShaderResources(0, 2, nullSrv);
    }
}

bool RSPass_KBBlur::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\ssao_compute.hlsl",
        "HMain", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mHoriBlurShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\ssao_compute.hlsl",
        "VMain", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertBlurShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_KBBlur::CreateViews()
{
    std::string name = "mrt-normal";
    mNormalMapSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "mrt-depth";
    mDepthMapSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "ssao-tex-compress-ssao";
    mSsaoTexUav = g_Root->ResourceManager()->
        GetResourceInfo(name)->mUav;

    return true;
}

RSPass_Shadow::RSPass_Shadow(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mAniVertexShader(nullptr),
    mRasterizerState(nullptr),
    mDepthStencilView({ nullptr,nullptr,nullptr,nullptr }),
    mDrawCallType(DRAWCALL_TYPE::OPACITY),
    mDrawCallPipe(nullptr),
    mViewProjStructedBuffer(nullptr),
    mViewProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
    mBonesStructedBuffer(nullptr),
    mBonesStructedBufferSrv(nullptr)
{

}

RSPass_Shadow::RSPass_Shadow(const RSPass_Shadow& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mAniVertexShader(_source.mAniVertexShader),
    mRasterizerState(_source.mRasterizerState),
    mDepthStencilView(_source.mDepthStencilView),
    mDrawCallType(_source.mDrawCallType),
    mDrawCallPipe(_source.mDrawCallPipe),
    mViewProjStructedBuffer(_source.mViewProjStructedBuffer),
    mViewProjStructedBufferSrv(_source.mViewProjStructedBufferSrv),
    mInstanceStructedBuffer(_source.mInstanceStructedBuffer),
    mInstanceStructedBufferSrv(_source.mInstanceStructedBufferSrv),
    mBonesStructedBuffer(_source.mBonesStructedBuffer),
    mBonesStructedBufferSrv(_source.mBonesStructedBufferSrv)
{

}

RSPass_Shadow::~RSPass_Shadow()
{

}

RSPass_Shadow* RSPass_Shadow::ClonePass()
{
    return new RSPass_Shadow(*this);
}

bool RSPass_Shadow::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateStates()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallType = DRAWCALL_TYPE::OPACITY;
    mDrawCallPipe = g_Root->DrawCallsPool()->
        GetDrawCallsPipe(mDrawCallType);

    mHasBeenInited = true;

    return true;
}

void RSPass_Shadow::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mAniVertexShader);
    RS_RELEASE(mRasterizerState);
    RS_RELEASE(mViewProjStructedBufferSrv);
    RS_RELEASE(mViewProjStructedBuffer);
    RS_RELEASE(mInstanceStructedBufferSrv);
    RS_RELEASE(mInstanceStructedBuffer);

    std::string name = "light-depth-light-other";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "light-depth-light-dep0";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "light-depth-light-dep1";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "light-depth-light-dep2";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "light-depth-light-dep3";
    g_Root->ResourceManager()->DeleteResource(name);
}

void RSPass_Shadow::ExecuatePass()
{
    ID3D11RenderTargetView* null = nullptr;
    //STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(nullptr, nullptr, 0);
    STContext()->RSSetState(mRasterizerState);

    DirectX::XMMATRIX mat = {};
    DirectX::XMFLOAT4X4 flt44 = {};
    UINT stride = sizeof(VertexType::TangentVertex);
    UINT aniStride = sizeof(VertexType::AnimationVertex);
    UINT offset = 0;
    auto shadowLights = g_Root->LightsContainer()->
        GetShadowLights();
    UINT shadowSize = (UINT)shadowLights->size();
    D3D11_MAPPED_SUBRESOURCE msr = {};

    static std::string A_NAME = "AnimationVertex";
    static const auto ANIMAT_LAYOUT =
        GetRSRoot_DX11_Singleton()->StaticResources()->
        GetStaticInputLayout(A_NAME);

    for (UINT i = 0; i < shadowSize; i++)
    {
        STContext()->OMSetRenderTargets(1,
            &null, mDepthStencilView[i]);
        STContext()->RSSetViewports(1, &g_ViewPort);
        STContext()->ClearDepthStencilView(
            mDepthStencilView[i], D3D11_CLEAR_DEPTH, 1.f, 0);

        STContext()->Map(mViewProjStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        ViewProj* vp_data = (ViewProj*)msr.pData;
        auto light = (*shadowLights)[i];
        auto lcam = light->GetRSLightCamera();
        mat = DirectX::XMLoadFloat4x4(
            &(lcam->GetRSCameraInfo()->mViewMat));
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&vp_data[0].mViewMat, mat);
        mat = DirectX::XMLoadFloat4x4(
            &(lcam->GetRSCameraInfo()->mProjMat));
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
        STContext()->Unmap(mViewProjStructedBuffer, 0);

        STContext()->VSSetShaderResources(
            0, 1, &mViewProjStructedBufferSrv);

        for (auto& call : mDrawCallPipe->mDatas)
        {
            if (call.mMeshData.mLayout == ANIMAT_LAYOUT)
            {
                STContext()->VSSetShader(mAniVertexShader, nullptr, 0);
                STContext()->IASetVertexBuffers(
                    0, 1, &call.mMeshData.mVertexBuffer, &aniStride, &offset);

                STContext()->Map(mBonesStructedBuffer, 0,
                    D3D11_MAP_WRITE_DISCARD, 0, &msr);
                DirectX::XMFLOAT4X4* b_data = (DirectX::XMFLOAT4X4*)msr.pData;
                void* boneData = call.mInstanceData.mBonesDataPtr;
                std::vector<std::vector<RS_SUBMESH_BONE_DATA>>* bones = nullptr;
                bones = static_cast<decltype(bones)>(boneData);
                // TEMP-----------------------
                auto boneInsSize = bones->size();
                // TEMP-----------------------
                for (size_t i = 0; i < boneInsSize; i++)
                {
                    for (size_t j = 0; j < MAX_STRUCTURED_BUFFER_SIZE; j++)
                    {
                        if (j < (*bones)[i].size())
                        {
                            DirectX::XMMATRIX trans = DirectX::XMLoadFloat4x4(
                                &((*bones)[i][j].mBoneTransform));
                            trans = DirectX::XMMatrixTranspose(trans);
                            DirectX::XMStoreFloat4x4(
                                b_data + i * MAX_STRUCTURED_BUFFER_SIZE + j,
                                trans);
                        }
                        else
                        {
                            DirectX::XMStoreFloat4x4(
                                b_data + i * MAX_STRUCTURED_BUFFER_SIZE + j,
                                DirectX::XMMatrixIdentity());
                        }
                    }
                }
                STContext()->Unmap(mBonesStructedBuffer, 0);

                STContext()->VSSetShaderResources(3, 1,
                    &mBonesStructedBufferSrv);
            }
            else
            {
                STContext()->VSSetShader(mVertexShader, nullptr, 0);
                STContext()->IASetVertexBuffers(
                    0, 1, &call.mMeshData.mVertexBuffer, &stride, &offset);
            }

            auto vecPtr = call.mInstanceData.mDataPtr;
            auto size = vecPtr->size();
            STContext()->Map(mInstanceStructedBuffer, 0,
                D3D11_MAP_WRITE_DISCARD, 0, &msr);
            RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
            for (size_t i = 0; i < size; i++)
            {
                mat = DirectX::XMLoadFloat4x4(
                    &(*vecPtr)[i].mWorldMat);
                mat = DirectX::XMMatrixTranspose(mat);
                DirectX::XMStoreFloat4x4(&ins_data[i].mWorldMat, mat);
                ins_data[i].mMaterialData =
                    (*vecPtr)[i].mMaterialData;
                ins_data[i].mCustomizedData1 =
                    (*vecPtr)[i].mCustomizedData1;
                ins_data[i].mCustomizedData2 =
                    (*vecPtr)[i].mCustomizedData2;
            }
            STContext()->Unmap(mInstanceStructedBuffer, 0);

            STContext()->IASetInputLayout(
                call.mMeshData.mLayout);
            STContext()->IASetPrimitiveTopology(
                call.mMeshData.mTopologyType);
            /*STContext()->IASetVertexBuffers(
                0, 1, &call.mMeshData.mVertexBuffer,
                &stride, &offset);*/
            STContext()->IASetIndexBuffer(
                call.mMeshData.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
            STContext()->VSSetShaderResources(
                1, 1, &mInstanceStructedBufferSrv);

            STContext()->DrawIndexedInstanced(
                call.mMeshData.mIndexCount,
                (UINT)call.mInstanceData.mDataPtr->size(), 0, 0, 0);

            if (call.mMeshData.mLayout == ANIMAT_LAYOUT)
            {
                ID3D11ShaderResourceView* nullSRV = nullptr;
                STContext()->VSSetShaderResources(3, 1,
                    &nullSRV);
            }
        }
    }

    STContext()->OMSetRenderTargets(1, &null, nullptr);
    STContext()->RSSetState(nullptr);
}

bool RSPass_Shadow::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\light_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    D3D_SHADER_MACRO macro[] =
    { "ANIMATION_VERTEX","1",nullptr,nullptr };
    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\light_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob, macro);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mAniVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Shadow::CreateStates()
{
    HRESULT hr = S_OK;
    D3D11_RASTERIZER_DESC shadowRasterDesc = {};
    ZeroMemory(&shadowRasterDesc, sizeof(shadowRasterDesc));

    shadowRasterDesc.FillMode = D3D11_FILL_SOLID;
    shadowRasterDesc.CullMode = D3D11_CULL_BACK;
    shadowRasterDesc.FrontCounterClockwise = FALSE;
    shadowRasterDesc.DepthBias = 50000;
    shadowRasterDesc.SlopeScaledDepthBias = 1.f;
    shadowRasterDesc.DepthBiasClamp = 0.f;
    shadowRasterDesc.DepthClipEnable = TRUE;
    shadowRasterDesc.ScissorEnable = FALSE;
    shadowRasterDesc.MultisampleEnable = FALSE;
    shadowRasterDesc.AntialiasedLineEnable = FALSE;

    hr = Device()->CreateRasterizerState(&shadowRasterDesc,
        &mRasterizerState);
    if (FAILED(hr))
    {
        return false;
    }

    return true;
}

bool RSPass_Shadow::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bdc = {};

    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_DYNAMIC;
    bdc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bdc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bdc.ByteWidth = MAX_INSTANCE_SIZE * sizeof(RS_INSTANCE_DATA);
    bdc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bdc.StructureByteStride = sizeof(RS_INSTANCE_DATA);
    hr = Device()->CreateBuffer(
        &bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE * (UINT)sizeof(DirectX::XMFLOAT4X4);
    bdc.StructureByteStride = sizeof(DirectX::XMFLOAT4X4);
    hr = Device()->CreateBuffer(&bdc, nullptr, &mBonesStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = sizeof(ViewProj);
    bdc.StructureByteStride = sizeof(ViewProj);
    hr = Device()->CreateBuffer(
        &bdc, nullptr, &mViewProjStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Shadow::CreateViews()
{
    HRESULT hr = S_OK;
    ID3D11Texture2D* depthTex = nullptr;
    D3D11_TEXTURE2D_DESC texDepSte = {};
    texDepSte.Width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth();
    texDepSte.Height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight();
    texDepSte.MipLevels = 1;
    texDepSte.ArraySize = MAX_SHADOW_SIZE;
    texDepSte.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDepSte.SampleDesc.Count = 1;
    texDepSte.SampleDesc.Quality = 0;
    texDepSte.Usage = D3D11_USAGE_DEFAULT;
    texDepSte.BindFlags = D3D11_BIND_DEPTH_STENCIL |
        D3D11_BIND_SHADER_RESOURCE;
    texDepSte.CPUAccessFlags = 0;
    texDepSte.MiscFlags = 0;
    hr = Device()->CreateTexture2D(
        &texDepSte, nullptr, &depthTex);
    if (FAILED(hr)) { return false; }

    D3D11_DEPTH_STENCIL_VIEW_DESC desDSV = {};
    desDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    desDSV.Texture2DArray.MipSlice = 0;
    desDSV.Texture2DArray.ArraySize = 1;
    for (UINT i = 0; i < MAX_SHADOW_SIZE; i++)
    {
        desDSV.Texture2DArray.FirstArraySlice =
            D3D11CalcSubresource(0, i, 1);
        hr = Device()->CreateDepthStencilView(
            depthTex, &desDSV, &(mDepthStencilView[i]));
        if (FAILED(hr)) { return false; }
    }

    ID3D11ShaderResourceView* srv = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC desSRV = {};
    desSRV.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    desSRV.Texture2DArray.FirstArraySlice = 0;
    desSRV.Texture2DArray.MostDetailedMip = 0;
    desSRV.Texture2DArray.MipLevels = 1;
    desSRV.Texture2DArray.ArraySize = MAX_SHADOW_SIZE;
    hr = Device()->CreateShaderResourceView(
        depthTex, &desSRV, &srv);
    if (FAILED(hr)) { return false; }

    RS_RESOURCE_INFO dti = {};
    std::string name = "light-depth-light-other";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = depthTex;
    dti.mSrv = srv;
    g_Root->ResourceManager()->AddResource(name, dti);

    dti = {};
    name = "light-depth-light-dep0";
    dti.mDsv = mDepthStencilView[0];
    g_Root->ResourceManager()->AddResource(name, dti);
    name = "light-depth-light-dep1";
    dti.mDsv = mDepthStencilView[1];
    g_Root->ResourceManager()->AddResource(name, dti);
    name = "light-depth-light-dep2";
    dti.mDsv = mDepthStencilView[2];
    g_Root->ResourceManager()->AddResource(name, dti);
    name = "light-depth-light-dep3";
    dti.mDsv = mDepthStencilView[3];
    g_Root->ResourceManager()->AddResource(name, dti);

    ZeroMemory(&desSRV, sizeof(desSRV));
    desSRV.Format = DXGI_FORMAT_UNKNOWN;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desSRV.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
    hr = Device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &desSRV, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    desSRV.Buffer.ElementWidth = MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE;
    hr = Device()->CreateShaderResourceView(
        mBonesStructedBuffer,
        &desSRV, &mBonesStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    desSRV.Buffer.ElementWidth = 1;
    hr = Device()->CreateShaderResourceView(
        mViewProjStructedBuffer,
        &desSRV, &mViewProjStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Shadow::CreateSamplers()
{
    return true;
}

RSPass_Defered::RSPass_Defered(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mLinearWrapSampler(nullptr), mRenderTargetView(nullptr),
    mShadowTexSampler(nullptr), mPointClampSampler(nullptr),
    mLightStructedBuffer(nullptr),
    mLightStructedBufferSrv(nullptr),
    mLightInfoStructedBuffer(nullptr),
    mLightInfoStructedBufferSrv(nullptr),
    mAmbientStructedBuffer(nullptr),
    mAmbientStructedBufferSrv(nullptr),
    mShadowStructedBuffer(nullptr),
    mShadowStructedBufferSrv(nullptr),
    mSsaoSrv(nullptr),
    mVertexBuffer(nullptr), mIndexBuffer(nullptr),
    mWorldPosSrv(nullptr), mNormalSrv(nullptr), mDiffuseSrv(nullptr),
    mDiffuseAlbedoSrv(nullptr), mFresenlShineseSrv(nullptr),
    mRSCameraInfo(nullptr), mShadowDepthSrv(nullptr)
{

}

RSPass_Defered::RSPass_Defered(const RSPass_Defered& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mRenderTargetView(_source.mRenderTargetView),
    mLinearWrapSampler(_source.mLinearWrapSampler),
    mPointClampSampler(_source.mPointClampSampler),
    mShadowTexSampler(_source.mShadowTexSampler),
    mLightStructedBuffer(_source.mLightStructedBuffer),
    mLightStructedBufferSrv(_source.mLightStructedBufferSrv),
    mLightInfoStructedBuffer(_source.mLightInfoStructedBuffer),
    mLightInfoStructedBufferSrv(_source.mLightInfoStructedBufferSrv),
    mAmbientStructedBuffer(_source.mAmbientStructedBuffer),
    mAmbientStructedBufferSrv(_source.mAmbientStructedBufferSrv),
    mShadowStructedBuffer(_source.mShadowStructedBuffer),
    mShadowStructedBufferSrv(_source.mShadowStructedBufferSrv),
    mSsaoSrv(_source.mSsaoSrv),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer),
    mWorldPosSrv(_source.mWorldPosSrv),
    mNormalSrv(_source.mNormalSrv),
    mDiffuseSrv(_source.mDiffuseSrv),
    mDiffuseAlbedoSrv(_source.mDiffuseAlbedoSrv),
    mFresenlShineseSrv(_source.mFresenlShineseSrv),
    mRSCameraInfo(_source.mRSCameraInfo),
    mShadowDepthSrv(_source.mShadowDepthSrv)
{

}

RSPass_Defered::~RSPass_Defered()
{

}

RSPass_Defered* RSPass_Defered::ClonePass()
{
    return new RSPass_Defered(*this);
}

bool RSPass_Defered::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->CamerasContainer()->
        GetRSCameraInfo(name);

    mHasBeenInited = true;

    return true;
}

void RSPass_Defered::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mLinearWrapSampler);
    RS_RELEASE(mPointClampSampler);
    RS_RELEASE(mShadowTexSampler);
    RS_RELEASE(mLightInfoStructedBuffer);
    RS_RELEASE(mLightInfoStructedBufferSrv);
    RS_RELEASE(mLightStructedBuffer);
    RS_RELEASE(mLightStructedBufferSrv);
    RS_RELEASE(mAmbientStructedBuffer);
    RS_RELEASE(mAmbientStructedBufferSrv);
    RS_RELEASE(mShadowStructedBuffer);
    RS_RELEASE(mShadowStructedBufferSrv);
    RS_RELEASE(mVertexBuffer);
    RS_RELEASE(mIndexBuffer);
}

void RSPass_Defered::ExecuatePass()
{
    STContext()->OMSetRenderTargets(1,
        &mRenderTargetView, nullptr);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->ClearRenderTargetView(
        mRenderTargetView, DirectX::Colors::DarkGreen);
    STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);

    DirectX::XMMATRIX mat = {};
    DirectX::XMFLOAT4X4 flt44 = {};
    UINT stride = sizeof(VertexType::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    STContext()->Map(mAmbientStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    Ambient* amb_data = (Ambient*)msr.pData;
    DirectX::XMFLOAT4 ambientL = GetRSRoot_DX11_Singleton()->
        LightsContainer()->GetCurrentAmbientLight();
    amb_data[0].mAmbient = ambientL;
    STContext()->Unmap(mAmbientStructedBuffer, 0);

    static auto lights = g_Root->LightsContainer()->GetLights();
    STContext()->Map(mLightInfoStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    LightInfo* li_data = (LightInfo*)msr.pData;
    li_data[0].mCameraPos = mRSCameraInfo->mEyePosition;
    UINT dNum = 0;
    UINT sNum = 0;
    UINT pNum = 0;
    for (auto& l : *lights)
    {
        auto type = l->GetRSLightType();
        switch (type)
        {
        case LIGHT_TYPE::DIRECT:
            ++dNum; break;
        case LIGHT_TYPE::POINT:
            ++pNum; break;
        case LIGHT_TYPE::SPOT:
            ++sNum; break;
        default: break;
        }
    }
    li_data[0].mDirectLightNum = dNum;
    li_data[0].mPointLightNum = pNum;
    li_data[0].mSpotLightNum = sNum;
    li_data[0].mShadowLightNum = (UINT)g_Root->LightsContainer()->
        GetShadowLights()->size();
    li_data[0].mShadowLightIndex[0] = -1;
    li_data[0].mShadowLightIndex[1] = -1;
    li_data[0].mShadowLightIndex[2] = -1;
    li_data[0].mShadowLightIndex[3] = -1;
    auto shadowIndeices = g_Root->LightsContainer()->
        GetShadowLightIndeices();
    for (UINT i = 0; i < li_data[0].mShadowLightNum; i++)
    {
        li_data[0].mShadowLightIndex[i] = (*shadowIndeices)[i];
        if (i >= 3)
        {
            break;
        }
    }
    STContext()->Unmap(mLightInfoStructedBuffer, 0);

    STContext()->Map(mLightStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    RS_LIGHT_INFO* l_data = (RS_LIGHT_INFO*)msr.pData;
    UINT lightIndex = 0;
    for (auto& l : *lights)
    {
        l_data[lightIndex++] = *(l->GetRSLightInfo());
    }
    STContext()->Unmap(mLightStructedBuffer, 0);

    STContext()->Map(mShadowStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    ShadowInfo* s_data = (ShadowInfo*)msr.pData;
    auto shadowLights = g_Root->LightsContainer()->
        GetShadowLights();
    UINT shadowSize = (UINT)shadowLights->size();
    for (UINT i = 0; i < shadowSize; i++)
    {
        auto lcam = (*shadowLights)[i]->GetRSLightCamera();
        mat = DirectX::XMLoadFloat4x4(
            &(lcam->GetRSCameraInfo()->mViewMat));
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&s_data[i].mShadowViewMat, mat);
        mat = DirectX::XMLoadFloat4x4(
            &(lcam->GetRSCameraInfo()->mProjMat));
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&s_data[i].mShadowProjMat, mat);
    }

    static DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);
    mat = DirectX::XMMatrixTranspose(
        DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat) *
        DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat) *
        T);
    DirectX::XMStoreFloat4x4(&s_data[0].mSSAOMat, mat);
    STContext()->Unmap(mShadowStructedBuffer, 0);

    static std::string depthSrvName = "mrt-depth";
    static auto depSrv = g_Root->ResourceManager()->
        GetResourceInfo(depthSrvName)->mSrv;
    static ID3D11ShaderResourceView* srvs[] =
    {
        mAmbientStructedBufferSrv,
        mLightInfoStructedBufferSrv,
        mLightStructedBufferSrv,
        mShadowStructedBufferSrv,
        mWorldPosSrv, mNormalSrv, mDiffuseSrv,
        mDiffuseAlbedoSrv, mFresenlShineseSrv,
        mSsaoSrv, mShadowDepthSrv,
        g_IblBrdfSrv, g_DiffMapSrv, g_SpecMapSrv,
        depSrv
    };
    STContext()->PSSetShaderResources(0, 15, srvs);

    static ID3D11SamplerState* samps[] =
    {
        mPointClampSampler,
        mLinearWrapSampler,
        mShadowTexSampler
    };
    STContext()->PSSetSamplers(0, 3, samps);

    STContext()->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    STContext()->IASetVertexBuffers(
        0, 1, &mVertexBuffer,
        &stride, &offset);
    STContext()->IASetIndexBuffer(
        mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    STContext()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    ID3D11RenderTargetView* rtvnull = nullptr;
    STContext()->OMSetRenderTargets(1, &rtvnull, nullptr);
    static ID3D11ShaderResourceView* nullsrvs[] =
    {
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr
    };
    STContext()->PSSetShaderResources(0, 15, nullsrvs);
}

bool RSPass_Defered::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\defered_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\defered_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Defered::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bufDesc = {};

    VertexType::TangentVertex v[4] = {};
    v[0].Position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
    v[1].Position = DirectX::XMFLOAT3(-1.0f, +1.0f, 0.0f);
    v[2].Position = DirectX::XMFLOAT3(+1.0f, +1.0f, 0.0f);
    v[3].Position = DirectX::XMFLOAT3(+1.0f, -1.0f, 0.0f);
    v[0].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    v[1].Normal = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    v[2].Normal = DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f);
    v[3].Normal = DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f);
    v[0].Tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    v[1].Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    v[2].Tangent = DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f);
    v[3].Tangent = DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f);
    v[0].TexCoord = DirectX::XMFLOAT2(0.0f, 1.0f);
    v[1].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
    v[2].TexCoord = DirectX::XMFLOAT2(1.0f, 0.0f);
    v[3].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);
    ZeroMemory(&bufDesc, sizeof(bufDesc));
    bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufDesc.ByteWidth = sizeof(VertexType::TangentVertex) * 4;
    bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = 0;
    bufDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = Device()->CreateBuffer(
        &bufDesc, &vinitData, &mVertexBuffer);
    if (FAILED(hr)) { return false; }

    UINT indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };
    ZeroMemory(&bufDesc, sizeof(bufDesc));
    bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufDesc.ByteWidth = sizeof(UINT) * 6;
    bufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.StructureByteStride = 0;
    bufDesc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    ZeroMemory(&iinitData, sizeof(iinitData));
    iinitData.pSysMem = indices;
    hr = Device()->CreateBuffer(
        &bufDesc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bufDesc, sizeof(bufDesc));
    bufDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufDesc.ByteWidth = MAX_LIGHT_SIZE * sizeof(RS_LIGHT_INFO);
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = sizeof(RS_LIGHT_INFO);
    hr = Device()->CreateBuffer(
        &bufDesc, nullptr, &mLightStructedBuffer);
    if (FAILED(hr)) { return false; }

    bufDesc.ByteWidth = sizeof(Ambient);
    bufDesc.StructureByteStride = sizeof(Ambient);
    hr = Device()->CreateBuffer(
        &bufDesc, nullptr, &mAmbientStructedBuffer);
    if (FAILED(hr)) { return false; }

    bufDesc.ByteWidth = sizeof(LightInfo);
    bufDesc.StructureByteStride = sizeof(LightInfo);
    hr = Device()->CreateBuffer(
        &bufDesc, nullptr, &mLightInfoStructedBuffer);
    if (FAILED(hr)) { return false; }

    bufDesc.ByteWidth = MAX_SHADOW_SIZE * sizeof(ShadowInfo);
    bufDesc.StructureByteStride = sizeof(ShadowInfo);
    hr = Device()->CreateBuffer(
        &bufDesc, nullptr, &mShadowStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Defered::CreateViews()
{
    mRenderTargetView = g_Root->Devices()->GetSwapChainRtv();

    std::string name = "mrt-worldpos";
    mWorldPosSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "mrt-normal";
    mNormalSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "mrt-diffuse";
    mDiffuseSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "mrt-diffuse-albedo";
    mDiffuseAlbedoSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "mrt-fresnel-shinese";
    mFresenlShineseSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "ssao-tex-compress-ssao";
    mSsaoSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    name = "light-depth-light-other";
    mShadowDepthSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;

    HRESULT hr = S_OK;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_LIGHT_SIZE;
    hr = Device()->CreateShaderResourceView(
        mLightStructedBuffer,
        &srvDesc, &mLightStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = 1;
    hr = Device()->CreateShaderResourceView(
        mLightInfoStructedBuffer,
        &srvDesc, &mLightInfoStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateShaderResourceView(
        mAmbientStructedBuffer,
        &srvDesc, &mAmbientStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = MAX_SHADOW_SIZE;
    hr = Device()->CreateShaderResourceView(
        mShadowStructedBuffer,
        &srvDesc, &mShadowStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Defered::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    auto filter = g_RenderEffectConfig.mSamplerLevel;
    switch (filter)
    {
    case SAMPLER_LEVEL::POINT:
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        break;
    case SAMPLER_LEVEL::BILINEAR:
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    case SAMPLER_LEVEL::ANISO_8X:
        sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampDesc.MaxAnisotropy = 8;
        break;
    case SAMPLER_LEVEL::ANISO_16X:
        sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampDesc.MaxAnisotropy = 16;
        break;
    default: return false;
    }
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &sampDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &sampDesc, &mPointClampSampler);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &sampDesc, &mShadowTexSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_SkyShpere::RSPass_SkyShpere(std::string& _name,
    PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mRasterizerState(nullptr), mDepthStencilState(nullptr),
    mRenderTargerView(nullptr), mDepthStencilView(nullptr),
    /*mSkyShpereSrv(nullptr), */mSkyShpereInfoStructedBuffer(nullptr),
    mSkyShpereInfoStructedBufferSrv(nullptr), mSkySphereMesh({}),
    mLinearWrapSampler(nullptr), mRSCameraInfo(nullptr)
{

}

RSPass_SkyShpere::RSPass_SkyShpere(
    const RSPass_SkyShpere& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mRasterizerState(_source.mRasterizerState),
    mDepthStencilState(_source.mDepthStencilState),
    mRenderTargerView(_source.mRenderTargerView),
    mDepthStencilView(_source.mDepthStencilView),
    //mSkyShpereSrv(_source.mSkyShpereSrv),
    mSkyShpereInfoStructedBuffer(_source.mSkyShpereInfoStructedBuffer),
    mSkyShpereInfoStructedBufferSrv(_source.mSkyShpereInfoStructedBufferSrv),
    mSkySphereMesh(_source.mSkySphereMesh),
    mLinearWrapSampler(_source.mLinearWrapSampler),
    mRSCameraInfo(_source.mRSCameraInfo)
{

}

RSPass_SkyShpere::~RSPass_SkyShpere()
{

}

RSPass_SkyShpere* RSPass_SkyShpere::ClonePass()
{
    return new RSPass_SkyShpere(*this);
}

bool RSPass_SkyShpere::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateStates()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mSkySphereMesh = g_Root->MeshHelper()->GeoGenerate()->
        CreateGeometrySphere(10.f, 0,
            LAYOUT_TYPE::NORMAL_TANGENT_TEX, false,
            {}, "room_env.dds");
    std::string resName = "room_env.dds";
    g_EnviMapSrv = g_Root->ResourceManager()->GetMeshSrv(resName);
    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        g_Root->Devices()->GetDevice(),
        L".\\Assets\\Textures\\room_diff.dds",
        nullptr, &g_DiffMapSrv);
    if (FAILED(hr)) { return false; }
    hr = DirectX::CreateDDSTextureFromFile(
        g_Root->Devices()->GetDevice(),
        L".\\Assets\\Textures\\room_spec.dds",
        nullptr, &g_SpecMapSrv);
    if (FAILED(hr)) { return false; }
    hr = DirectX::CreateDDSTextureFromFile(
        g_Root->Devices()->GetDevice(),
        L".\\Assets\\Textures\\ibl_brdf.dds",
        nullptr, &g_IblBrdfSrv);
    if (FAILED(hr)) { return false; }

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->CamerasContainer()->
        GetRSCameraInfo(name);

    mHasBeenInited = true;

    return true;
}

void RSPass_SkyShpere::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mRasterizerState);
    RS_RELEASE(mDepthStencilState);
    RS_RELEASE(mLinearWrapSampler);
    RS_RELEASE(mSkyShpereInfoStructedBuffer);
    //RS_RELEASE(mSkyShpereInfoStructedBufferSrv);

    g_Root->MeshHelper()->ReleaseSubMesh(mSkySphereMesh);
}

void RSPass_SkyShpere::ExecuatePass()
{
    ID3D11RenderTargetView* null = nullptr;
    STContext()->OMSetRenderTargets(1,
        &mRenderTargerView, mDepthStencilView);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);
    STContext()->RSSetState(mRasterizerState);
    STContext()->OMSetDepthStencilState(mDepthStencilState, 0);

    DirectX::XMMATRIX mat = {};
    DirectX::XMFLOAT4X4 flt44 = {};
    UINT stride = sizeof(VertexType::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};

    STContext()->Map(mSkyShpereInfoStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    SkyShpereInfo* sp_data = (SkyShpereInfo*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&sp_data[0].mViewMat, mat);

    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&sp_data[0].mProjMat, mat);

    sp_data[0].mEyePosition = mRSCameraInfo->mEyePosition;

    mat = DirectX::XMMatrixScaling(1000.f, 1000.f, 1000.f);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&sp_data[0].mWorldMat, mat);
    STContext()->Unmap(mSkyShpereInfoStructedBuffer, 0);

    STContext()->IASetInputLayout(mSkySphereMesh.mLayout);
    STContext()->IASetPrimitiveTopology(
        mSkySphereMesh.mTopologyType);
    STContext()->IASetVertexBuffers(
        0, 1, &mSkySphereMesh.mVertexBuffer,
        &stride, &offset);
    STContext()->IASetIndexBuffer(
        mSkySphereMesh.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    STContext()->VSSetShaderResources(
        0, 1, &mSkyShpereInfoStructedBufferSrv);
    static std::string tex = mSkySphereMesh.mTextures[0];
    static ID3D11ShaderResourceView* cube = nullptr;
    cube = g_Root->ResourceManager()->GetMeshSrv(tex);
    STContext()->PSSetShaderResources(
        0, 1, &cube);
    STContext()->PSSetSamplers(0, 1, &mLinearWrapSampler);

    STContext()->DrawIndexedInstanced(mSkySphereMesh.mIndexCount,
        1, 0, 0, 0);

    STContext()->OMSetRenderTargets(1, &null, nullptr);
    STContext()->RSSetState(nullptr);
    STContext()->OMSetDepthStencilState(nullptr, 0);
}

bool RSPass_SkyShpere::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\skysphere_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\skysphere_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_SkyShpere::CreateStates()
{
    HRESULT hr = S_OK;
    D3D11_RASTERIZER_DESC rasDesc = {};
    D3D11_DEPTH_STENCIL_DESC depDesc = {};
    ZeroMemory(&rasDesc, sizeof(rasDesc));
    ZeroMemory(&depDesc, sizeof(depDesc));

    rasDesc.CullMode = D3D11_CULL_NONE;
    rasDesc.FillMode = D3D11_FILL_SOLID;
    depDesc.DepthEnable = TRUE;
    depDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    hr = Device()->CreateRasterizerState(
        &rasDesc, &mRasterizerState);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateDepthStencilState(
        &depDesc, &mDepthStencilState);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_SkyShpere::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bdc = {};

    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_DYNAMIC;
    bdc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bdc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bdc.ByteWidth = sizeof(SkyShpereInfo);
    bdc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bdc.StructureByteStride = sizeof(SkyShpereInfo);
    hr = Device()->CreateBuffer(
        &bdc, nullptr, &mSkyShpereInfoStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_SkyShpere::CreateViews()
{
    mRenderTargerView = g_Root->Devices()->GetSwapChainRtv();
    std::string name = "mrt-depth";
    mDepthStencilView = g_Root->ResourceManager()->
        GetResourceInfo(name)->mDsv;

    D3D11_SHADER_RESOURCE_VIEW_DESC desSRV = {};
    HRESULT hr = S_OK;
    ZeroMemory(&desSRV, sizeof(desSRV));
    desSRV.Format = DXGI_FORMAT_UNKNOWN;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desSRV.Buffer.ElementWidth = 1;
    hr = Device()->CreateShaderResourceView(
        mSkyShpereInfoStructedBuffer,
        &desSRV, &mSkyShpereInfoStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_SkyShpere::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC samDesc = {};
    ZeroMemory(&samDesc, sizeof(samDesc));

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &samDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_Bloom::RSPass_Bloom(std::string& _name, PASS_TYPE _type,
    RSRoot_DX11* _root) :RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mDrawCallType(DRAWCALL_TYPE::LIGHT), mDrawCallPipe(nullptr),
    mViewProjStructedBuffer(nullptr),
    mViewProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
    mRtv(nullptr), mDepthDsv(nullptr), mRSCameraInfo(nullptr),
    mCompressVertexShader(nullptr), mCompressPixelShader(nullptr),
    mNotCompressSrv(nullptr), mCompressRtv(nullptr),
    mVertexBuffer(nullptr), mIndexBuffer(nullptr),
    mSampler(nullptr)
{

}

RSPass_Bloom::RSPass_Bloom(const RSPass_Bloom& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mDrawCallType(_source.mDrawCallType),
    mDrawCallPipe(_source.mDrawCallPipe),
    mViewProjStructedBuffer(_source.mViewProjStructedBuffer),
    mViewProjStructedBufferSrv(_source.mViewProjStructedBufferSrv),
    mInstanceStructedBuffer(_source.mInstanceStructedBuffer),
    mInstanceStructedBufferSrv(_source.mInstanceStructedBufferSrv),
    mRtv(_source.mRtv), mDepthDsv(_source.mDepthDsv),
    mRSCameraInfo(_source.mRSCameraInfo),
    mCompressVertexShader(_source.mCompressVertexShader),
    mCompressPixelShader(_source.mCompressPixelShader),
    mNotCompressSrv(_source.mNotCompressSrv),
    mCompressRtv(_source.mCompressRtv),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer),
    mSampler(_source.mSampler)
{

}

RSPass_Bloom::~RSPass_Bloom()
{

}

RSPass_Bloom* RSPass_Bloom::ClonePass()
{
    return new RSPass_Bloom(*this);
}

bool RSPass_Bloom::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallPipe = g_Root->DrawCallsPool()->
        GetDrawCallsPipe(mDrawCallType);

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->CamerasContainer()->
        GetRSCameraInfo(name);
    if (!mRSCameraInfo) { return false; }

    mHasBeenInited = true;

    return true;
}

void RSPass_Bloom::ReleasePass()
{
    std::string name = "bloom-light";
    g_Root->ResourceManager()->DeleteResource(name);
    name = "bloom-compress-light";
    g_Root->ResourceManager()->DeleteResource(name);

    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mViewProjStructedBufferSrv);
    RS_RELEASE(mInstanceStructedBufferSrv);
    RS_RELEASE(mViewProjStructedBuffer);
    RS_RELEASE(mVertexBuffer);
    RS_RELEASE(mIndexBuffer);
    RS_RELEASE(mSampler);
    RS_RELEASE(mCompressVertexShader);
    RS_RELEASE(mCompressPixelShader);
}

void RSPass_Bloom::ExecuatePass()
{
    STContext()->OMSetRenderTargets(1, &mRtv, mDepthDsv);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->ClearRenderTargetView(mRtv,
        DirectX::Colors::Transparent);
    STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);

    DirectX::XMMATRIX mat = {};
    DirectX::XMFLOAT4X4 flt44 = {};
    UINT stride = sizeof(VertexType::ColorVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    STContext()->Map(mViewProjStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    ViewProj* vp_data = (ViewProj*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mViewMat, mat);
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
    STContext()->Unmap(mViewProjStructedBuffer, 0);

    STContext()->VSSetShaderResources(
        0, 1, &mViewProjStructedBufferSrv);

    for (auto& call : mDrawCallPipe->mDatas)
    {
        auto vecPtr = call.mInstanceData.mDataPtr;
        auto size = vecPtr->size();
        STContext()->Map(mInstanceStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            mat = DirectX::XMLoadFloat4x4(
                &(*vecPtr)[i].mWorldMat);
            mat = DirectX::XMMatrixTranspose(mat);
            DirectX::XMStoreFloat4x4(&ins_data[i].mWorldMat, mat);
            ins_data[i].mMaterialData =
                (*vecPtr)[i].mMaterialData;
            ins_data[i].mCustomizedData1 =
                (*vecPtr)[i].mCustomizedData1;
            ins_data[i].mCustomizedData2 =
                (*vecPtr)[i].mCustomizedData2;
        }
        STContext()->Unmap(mInstanceStructedBuffer, 0);

        STContext()->IASetInputLayout(
            call.mMeshData.mLayout);
        STContext()->IASetPrimitiveTopology(
            call.mMeshData.mTopologyType);
        STContext()->IASetVertexBuffers(
            0, 1, &call.mMeshData.mVertexBuffer,
            &stride, &offset);
        STContext()->IASetIndexBuffer(
            call.mMeshData.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        STContext()->VSSetShaderResources(
            1, 1, &mInstanceStructedBufferSrv);

        STContext()->DrawIndexedInstanced(
            call.mMeshData.mIndexCount,
            (UINT)call.mInstanceData.mDataPtr->size(), 0, 0, 0);
    }

    stride = sizeof(VertexType::TangentVertex);
    STContext()->OMSetRenderTargets(1, &mCompressRtv, nullptr);
    STContext()->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    STContext()->IASetVertexBuffers(
        0, 1, &mVertexBuffer,
        &stride, &offset);
    STContext()->IASetIndexBuffer(
        mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->VSSetShader(mCompressVertexShader, nullptr, 0);
    STContext()->PSSetShader(mCompressPixelShader, nullptr, 0);
    STContext()->PSSetSamplers(0, 1, &mSampler);
    STContext()->PSSetShaderResources(
        0, 1, &mNotCompressSrv);

    STContext()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    static ID3D11RenderTargetView* nullrtv[] = { nullptr };
    STContext()->OMSetRenderTargets(1, nullrtv, nullptr);
}

bool RSPass_Bloom::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\bloom_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\bloom_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\compress_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mCompressVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\compress_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mCompressPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Bloom::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC samDesc = {};
    ZeroMemory(&samDesc, sizeof(samDesc));

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &samDesc, &mSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Bloom::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bdc = {};

    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_DYNAMIC;
    bdc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bdc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bdc.ByteWidth = MAX_INSTANCE_SIZE * sizeof(RS_INSTANCE_DATA);
    bdc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bdc.StructureByteStride = sizeof(RS_INSTANCE_DATA);
    hr = Device()->CreateBuffer(
        &bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = sizeof(ViewProj);
    bdc.StructureByteStride = sizeof(ViewProj);
    hr = Device()->CreateBuffer(
        &bdc, nullptr, &mViewProjStructedBuffer);
    if (FAILED(hr)) { return false; }

    VertexType::TangentVertex v[4] = {};
    v[0].Position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
    v[1].Position = DirectX::XMFLOAT3(-1.0f, +1.0f, 0.0f);
    v[2].Position = DirectX::XMFLOAT3(+1.0f, +1.0f, 0.0f);
    v[3].Position = DirectX::XMFLOAT3(+1.0f, -1.0f, 0.0f);
    v[0].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    v[1].Normal = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    v[2].Normal = DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f);
    v[3].Normal = DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f);
    v[0].Tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    v[1].Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    v[2].Tangent = DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f);
    v[3].Tangent = DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f);
    v[0].TexCoord = DirectX::XMFLOAT2(0.0f, 1.0f);
    v[1].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
    v[2].TexCoord = DirectX::XMFLOAT2(1.0f, 0.0f);
    v[3].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);
    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_IMMUTABLE;
    bdc.ByteWidth = sizeof(VertexType::TangentVertex) * 4;
    bdc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bdc.CPUAccessFlags = 0;
    bdc.MiscFlags = 0;
    bdc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = Device()->CreateBuffer(
        &bdc, &vinitData, &mVertexBuffer);
    if (FAILED(hr)) { return false; }

    UINT indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };
    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_IMMUTABLE;
    bdc.ByteWidth = sizeof(UINT) * 6;
    bdc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bdc.CPUAccessFlags = 0;
    bdc.StructureByteStride = 0;
    bdc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    ZeroMemory(&iinitData, sizeof(iinitData));
    iinitData.pSysMem = indices;
    hr = Device()->CreateBuffer(
        &bdc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Bloom::CreateViews()
{
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC texDesc = {};
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    ID3D11Texture2D* texture = nullptr;
    ID3D11ShaderResourceView* srv = nullptr;
    ID3D11UnorderedAccessView* uav = nullptr;
    std::string name = "";

    name = "mrt-depth";
    mDepthDsv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mDsv;
    if (!mDepthDsv) { return false; }

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
    hr = Device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &srvDesc, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = 1;
    hr = Device()->CreateShaderResourceView(
        mViewProjStructedBuffer,
        &srvDesc, &mViewProjStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&texDesc, sizeof(texDesc));
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    RS_RESOURCE_INFO dti = {};

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth();
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags =
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(
        &texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(
        texture, &rtvDesc, &mRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(texture,
        &srvDesc, &mNotCompressSrv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "bloom-light";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = mRtv;
    dti.mSrv = mNotCompressSrv;
    g_Root->ResourceManager()->AddResource(name, dti);

    texDesc.Width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth() / 2;
    texDesc.Height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight() / 2;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags =
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS |
        D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(
        &texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(
        texture, &rtvDesc, &mCompressRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = Device()->CreateShaderResourceView(texture,
        &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    hr = Device()->CreateUnorderedAccessView(
        texture, &uavDesc, &uav);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "bloom-compress-light";
    dti.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.mResource.mTexture2D = texture;
    dti.mRtv = mCompressRtv;
    dti.mSrv = srv;
    dti.mUav = uav;
    g_Root->ResourceManager()->AddResource(name, dti);

    return true;
}

RSPass_BloomOn::RSPass_BloomOn(std::string& _name, PASS_TYPE _type,
    RSRoot_DX11* _root) :RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mRtv(nullptr), mBloomTexSrv(nullptr), mDepthState(nullptr),
    mBlendState(nullptr), mSampler(nullptr),
    mVertexBuffer(nullptr), mIndexBuffer(nullptr)
{

}

RSPass_BloomOn::RSPass_BloomOn(const RSPass_BloomOn& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mRtv(_source.mRtv),
    mBloomTexSrv(_source.mBloomTexSrv),
    mBlendState(_source.mBlendState),
    mDepthState(_source.mDepthState),
    mSampler(_source.mSampler),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer)
{

}

RSPass_BloomOn::~RSPass_BloomOn()
{

}

RSPass_BloomOn* RSPass_BloomOn::ClonePass()
{
    return new RSPass_BloomOn(*this);
}

bool RSPass_BloomOn::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateStates()) { return false; }
    if (!CreateSamplers()) { return false; }

    mHasBeenInited = true;

    return true;
}

void RSPass_BloomOn::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mBlendState);
    RS_RELEASE(mDepthState);
    RS_RELEASE(mSampler);
    RS_RELEASE(mVertexBuffer);
    RS_RELEASE(mIndexBuffer);
}

void RSPass_BloomOn::ExecuatePass()
{
    ID3D11RenderTargetView* rtvnull = nullptr;
    STContext()->OMSetRenderTargets(1, &mRtv, nullptr);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->OMSetDepthStencilState(mDepthState, 0);
    static float factor[4] = { 0.f,0.f,0.f,0.f };
    STContext()->OMSetBlendState(mBlendState, factor, 0xFFFFFFFF);
    STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);
    STContext()->PSSetSamplers(0, 1, &mSampler);
    STContext()->PSSetShaderResources(0, 1, &mBloomTexSrv);

    UINT stride = sizeof(VertexType::TangentVertex);
    UINT offset = 0;
    STContext()->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    STContext()->IASetVertexBuffers(
        0, 1, &mVertexBuffer,
        &stride, &offset);
    STContext()->IASetIndexBuffer(
        mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    STContext()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    STContext()->OMSetRenderTargets(1, &rtvnull, nullptr);
    STContext()->OMSetDepthStencilState(nullptr, 0);
    STContext()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

bool RSPass_BloomOn::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\bloomon_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\bloomon_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_BloomOn::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bufDesc = {};

    VertexType::TangentVertex v[4] = {};
    v[0].Position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
    v[1].Position = DirectX::XMFLOAT3(-1.0f, +1.0f, 0.0f);
    v[2].Position = DirectX::XMFLOAT3(+1.0f, +1.0f, 0.0f);
    v[3].Position = DirectX::XMFLOAT3(+1.0f, -1.0f, 0.0f);
    v[0].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    v[1].Normal = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    v[2].Normal = DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f);
    v[3].Normal = DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f);
    v[0].Tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
    v[1].Tangent = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);
    v[2].Tangent = DirectX::XMFLOAT3(2.0f, 0.0f, 0.0f);
    v[3].Tangent = DirectX::XMFLOAT3(3.0f, 0.0f, 0.0f);
    v[0].TexCoord = DirectX::XMFLOAT2(0.0f, 1.0f);
    v[1].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
    v[2].TexCoord = DirectX::XMFLOAT2(1.0f, 0.0f);
    v[3].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);
    ZeroMemory(&bufDesc, sizeof(bufDesc));
    bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufDesc.ByteWidth = sizeof(VertexType::TangentVertex) * 4;
    bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = 0;
    bufDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = Device()->CreateBuffer(
        &bufDesc, &vinitData, &mVertexBuffer);
    if (FAILED(hr)) { return false; }

    UINT indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };
    ZeroMemory(&bufDesc, sizeof(bufDesc));
    bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufDesc.ByteWidth = sizeof(UINT) * 6;
    bufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.StructureByteStride = 0;
    bufDesc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    ZeroMemory(&iinitData, sizeof(iinitData));
    iinitData.pSysMem = indices;
    hr = Device()->CreateBuffer(
        &bufDesc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_BloomOn::CreateStates()
{
    HRESULT hr = S_OK;

    D3D11_DEPTH_STENCIL_DESC depDesc = {};
    depDesc.DepthEnable = FALSE;
    depDesc.StencilEnable = FALSE;
    hr = Device()->CreateDepthStencilState(
        &depDesc, &mDepthState);
    if (FAILED(hr)) { return false; }

    D3D11_BLEND_DESC bldDesc = {};
    bldDesc.AlphaToCoverageEnable = FALSE;
    bldDesc.IndependentBlendEnable = FALSE;
    bldDesc.RenderTarget[0].BlendEnable = TRUE;
    bldDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D11_COLOR_WRITE_ENABLE_ALL;
    bldDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    bldDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bldDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bldDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bldDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bldDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    hr = Device()->CreateBlendState(&bldDesc, &mBlendState);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_BloomOn::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = Device()->CreateSamplerState(
        &sampDesc, &mSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_BloomOn::CreateViews()
{
    mRtv = g_Root->Devices()->GetSwapChainRtv();

    std::string name = "bloom-compress-light";
    mBloomTexSrv = g_Root->ResourceManager()->
        GetResourceInfo(name)->mSrv;
    if (!mBloomTexSrv) { return false; }

    return true;
}

RSPass_Blur::RSPass_Blur(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mHoriBlurShader(nullptr), mVertBlurShader(nullptr),
    mLightTexUav(nullptr)
{

}

RSPass_Blur::RSPass_Blur(const RSPass_Blur& _source) :
    RSPass_Base(_source),
    mHoriBlurShader(_source.mHoriBlurShader),
    mVertBlurShader(_source.mVertBlurShader),
    mLightTexUav(_source.mLightTexUav)
{

}

RSPass_Blur::~RSPass_Blur()
{

}

RSPass_Blur* RSPass_Blur::ClonePass()
{
    return new RSPass_Blur(*this);
}

bool RSPass_Blur::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }

    mHasBeenInited = true;

    return true;
}

void RSPass_Blur::ReleasePass()
{
    RS_RELEASE(mHoriBlurShader);
    RS_RELEASE(mVertBlurShader);
}

void RSPass_Blur::ExecuatePass()
{
    static ID3D11UnorderedAccessView* nullUav = nullptr;

    static UINT loopCount = 1;
    static UINT width = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndWidth() / 2;
    static UINT height = GetRSRoot_DX11_Singleton()->Devices()->
        GetCurrWndHeight() / 2;
    UINT dispatchVert = Tool::Align(width, 256) / 256;
    UINT dispatchHori = Tool::Align(height, 256) / 256;

    for (UINT i = 0; i < loopCount; i++)
    {
        STContext()->CSSetShader(mHoriBlurShader, nullptr, 0);
        STContext()->CSSetUnorderedAccessViews(0, 1,
            &mLightTexUav, nullptr);
        STContext()->Dispatch(dispatchVert, height, 1);
        STContext()->CSSetUnorderedAccessViews(0, 1,
            &nullUav, nullptr);

        STContext()->CSSetShader(mVertBlurShader, nullptr, 0);
        STContext()->CSSetUnorderedAccessViews(0, 1,
            &mLightTexUav, nullptr);
        STContext()->Dispatch(width, dispatchHori, 1);
        STContext()->CSSetUnorderedAccessViews(0, 1,
            &nullUav, nullptr);
    }
}

bool RSPass_Blur::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\bloom_compute.hlsl",
        "HMain", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mHoriBlurShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\bloom_compute.hlsl",
        "VMain", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertBlurShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Blur::CreateViews()
{
    std::string name = "bloom-compress-light";
    mLightTexUav = g_Root->ResourceManager()->
        GetResourceInfo(name)->mUav;
    if (!mLightTexUav) { return false; }

    return true;
}

RSPass_PriticleSetUp::RSPass_PriticleSetUp(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mTilingConstant({}),
    mParticleRenderBuffer(nullptr),
    mParticleRender_Srv(nullptr), mParticleRender_Uav(nullptr),
    mParticleRandomTexture(nullptr), mParticleRandom_Srv(nullptr),
    mParticlePartA(nullptr), mPartA_Srv(nullptr), mPartA_Uav(nullptr),
    mParticlePartB(nullptr), mPartB_Uav(nullptr),
    mViewspacePosBuffer(nullptr),
    mViewSpacePos_Srv(nullptr), mViewSpacePos_Uav(nullptr),
    mMaxRadiusBuffer(nullptr),
    mMaxRadius_Srv(nullptr), mMaxRadius_Uav(nullptr),
    mStridedCoarseCullBuffer(nullptr),
    mStridedCoarseCull_Srv(nullptr), mStridedCoarseCull_Uav(nullptr),
    mStridedCoarseCullCounterBuffer(nullptr),
    mStridedCoarseCullCounter_Srv(nullptr),
    mStridedCoarseCullCounter_Uav(nullptr),
    mTiledIndexBuffer(nullptr),
    mTiledIndex_Srv(nullptr),
    mTiledIndex_Uav(nullptr),
    mDeadListBuffer(nullptr), mDeadList_Uav(nullptr),
    mAliveIndexBuffer(nullptr),
    mAliveIndex_Srv(nullptr), mAliveIndex_Uav(nullptr),
    mDeadListConstantBuffer(nullptr),
    mActiveListConstantBuffer(nullptr),
    mEmitterConstantBuffer(nullptr),
    mCameraConstantBuffer(nullptr),
    mTilingConstantBuffer(nullptr),
    mDebugCounterBuffer(nullptr),
    mSimulEmitterStructedBuffer(nullptr),
    mSimulEmitterStructedBuffer_Srv(nullptr),
    mTimeConstantBuffer(nullptr)
{
    g_ParticleSetUpPass = this;
}

RSPass_PriticleSetUp::RSPass_PriticleSetUp(
    const RSPass_PriticleSetUp& _source) :
    RSPass_Base(_source),
    mTilingConstant(_source.mTilingConstant),
    mParticleRenderBuffer(_source.mParticleRenderBuffer),
    mParticleRender_Srv(_source.mParticleRender_Srv),
    mParticleRender_Uav(_source.mParticleRender_Uav),
    mParticleRandomTexture(_source.mParticleRandomTexture),
    mParticleRandom_Srv(_source.mParticleRandom_Srv),
    mParticlePartA(_source.mParticlePartA),
    mPartA_Srv(_source.mPartA_Srv),
    mPartA_Uav(_source.mPartA_Uav),
    mParticlePartB(_source.mParticlePartB),
    mPartB_Uav(_source.mPartB_Uav),
    mViewspacePosBuffer(_source.mViewspacePosBuffer),
    mViewSpacePos_Srv(_source.mViewSpacePos_Srv),
    mViewSpacePos_Uav(_source.mViewSpacePos_Uav),
    mMaxRadiusBuffer(_source.mMaxRadiusBuffer),
    mMaxRadius_Srv(_source.mMaxRadius_Srv),
    mMaxRadius_Uav(_source.mMaxRadius_Uav),
    mStridedCoarseCullBuffer(_source.mStridedCoarseCullBuffer),
    mStridedCoarseCull_Srv(_source.mStridedCoarseCull_Srv),
    mStridedCoarseCull_Uav(_source.mStridedCoarseCull_Uav),
    mStridedCoarseCullCounterBuffer(_source.mStridedCoarseCullCounterBuffer),
    mStridedCoarseCullCounter_Srv(_source.mStridedCoarseCullCounter_Srv),
    mStridedCoarseCullCounter_Uav(_source.mStridedCoarseCullCounter_Uav),
    mTiledIndexBuffer(_source.mTiledIndexBuffer),
    mTiledIndex_Srv(_source.mTiledIndex_Srv),
    mTiledIndex_Uav(_source.mTiledIndex_Uav),
    mDeadListBuffer(_source.mDeadListBuffer),
    mDeadList_Uav(_source.mDeadList_Uav),
    mAliveIndexBuffer(_source.mAliveIndexBuffer),
    mAliveIndex_Srv(_source.mAliveIndex_Srv),
    mAliveIndex_Uav(_source.mAliveIndex_Uav),
    mDeadListConstantBuffer(_source.mDeadListConstantBuffer),
    mActiveListConstantBuffer(_source.mActiveListConstantBuffer),
    mEmitterConstantBuffer(_source.mEmitterConstantBuffer),
    mCameraConstantBuffer(_source.mCameraConstantBuffer),
    mTilingConstantBuffer(_source.mTilingConstantBuffer),
    mDebugCounterBuffer(_source.mDebugCounterBuffer),
    mSimulEmitterStructedBuffer(_source.mSimulEmitterStructedBuffer),
    mSimulEmitterStructedBuffer_Srv(_source.mSimulEmitterStructedBuffer_Srv),
    mTimeConstantBuffer(_source.mTimeConstantBuffer)
{
    g_ParticleSetUpPass = this;
}

RSPass_PriticleSetUp::~RSPass_PriticleSetUp()
{

}

const RS_TILING_CONSTANT& RSPass_PriticleSetUp::GetTilingConstantInfo() const
{
    return mTilingConstant;
}

RSPass_PriticleSetUp* RSPass_PriticleSetUp::ClonePass()
{
    return new RSPass_PriticleSetUp(*this);
}

bool RSPass_PriticleSetUp::InitPass()
{
    if (mHasBeenInited) { return true; }

    int width = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndWidth();
    int height = GetRSRoot_DX11_Singleton()->Devices()->GetCurrWndHeight();

    mTilingConstant.mNumTilesX =
        Tool::Align(width, PTC_TILE_X_SIZE) / PTC_TILE_X_SIZE;
    mTilingConstant.mNumTilesY =
        Tool::Align(height, PTC_TILE_Y_SIZE) / PTC_TILE_Y_SIZE;
    mTilingConstant.mNumCoarseCullingTilesX = PTC_MAX_COARSE_CULL_TILE_X;
    mTilingConstant.mNumCoarseCullingTilesY = PTC_MAX_COARSE_CULL_TILE_Y;
    mTilingConstant.mNumCullingTilesPerCoarseTileX =
        Tool::Align(
            mTilingConstant.mNumTilesX,
            mTilingConstant.mNumCoarseCullingTilesX) /
        mTilingConstant.mNumCoarseCullingTilesX;
    mTilingConstant.mNumCullingTilesPerCoarseTileY =
        Tool::Align(
            mTilingConstant.mNumTilesY,
            mTilingConstant.mNumCoarseCullingTilesY) /
        mTilingConstant.mNumCoarseCullingTilesY;

    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }

    auto resManager = GetRSRoot_DX11_Singleton()->ResourceManager();
    RS_RESOURCE_INFO res;
    std::string name = "";

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mParticleRenderBuffer;
    res.mSrv = mParticleRender_Srv;
    res.mUav = mParticleRender_Uav;
    name = PTC_RENDER_BUFFER_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mParticlePartA;
    res.mSrv = mPartA_Srv;
    res.mUav = mPartA_Uav;
    name = PTC_A_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mParticlePartB;
    res.mUav = mPartB_Uav;
    name = PTC_B_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mViewspacePosBuffer;
    res.mSrv = mViewSpacePos_Srv;
    res.mUav = mViewSpacePos_Uav;
    name = PTC_VIEW_SPCACE_POS_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mMaxRadiusBuffer;
    res.mSrv = mMaxRadius_Srv;
    res.mUav = mMaxRadius_Uav;
    name = PTC_MAX_RADIUS_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mStridedCoarseCullBuffer;
    res.mSrv = mStridedCoarseCull_Srv;
    res.mUav = mStridedCoarseCull_Uav;
    name = PTC_COARSE_CULL_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mStridedCoarseCullCounterBuffer;
    res.mSrv = mStridedCoarseCullCounter_Srv;
    res.mUav = mStridedCoarseCullCounter_Uav;
    name = PTC_COARSE_CULL_COUNTER_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mTiledIndexBuffer;
    res.mSrv = mTiledIndex_Srv;
    res.mUav = mTiledIndex_Uav;
    name = PTC_TILED_INDEX_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mDeadListBuffer;
    res.mUav = mDeadList_Uav;
    name = PTC_DEAD_LIST_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mAliveIndexBuffer;
    res.mSrv = mAliveIndex_Srv;
    res.mUav = mAliveIndex_Uav;
    name = PTC_ALIVE_INDEX_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mDeadListConstantBuffer;
    name = PTC_DEAD_LIST_CONSTANT_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mActiveListConstantBuffer;
    name = PTC_ALIVE_LIST_CONSTANT_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mEmitterConstantBuffer;
    name = PTC_EMITTER_CONSTANT_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mCameraConstantBuffer;
    name = PTC_CAMERA_CONSTANT_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mTilingConstantBuffer;
    name = PTC_TILING_CONSTANT_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mDebugCounterBuffer;
    name = PTC_DEBUG_COUNTER_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::TEXTURE2D;
    res.mResource.mTexture2D = mParticleRandomTexture;
    res.mSrv = mParticleRandom_Srv;
    name = PTC_RAMDOM_TEXTURE_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mSimulEmitterStructedBuffer;
    res.mSrv = mSimulEmitterStructedBuffer_Srv;
    name = PTC_SIMU_EMITTER_STRU_NAME;
    resManager->AddResource(name, res);

    res = {};
    res.mType = RS_RESOURCE_TYPE::BUFFER;
    res.mResource.mBuffer = mTimeConstantBuffer;
    name = PTC_TIME_CONSTANT_NAME;
    resManager->AddResource(name, res);

    mHasBeenInited = true;

    return true;
}

void RSPass_PriticleSetUp::ReleasePass()
{
    auto resManager = GetRSRoot_DX11_Singleton()->ResourceManager();
    std::string name = PTC_RENDER_BUFFER_NAME;
    resManager->DeleteResource(name);
    name = PTC_A_NAME;
    resManager->DeleteResource(name);
    name = PTC_B_NAME;
    resManager->DeleteResource(name);
    name = PTC_VIEW_SPCACE_POS_NAME;
    resManager->DeleteResource(name);
    name = PTC_MAX_RADIUS_NAME;
    resManager->DeleteResource(name);
    name = PTC_COARSE_CULL_NAME;
    resManager->DeleteResource(name);
    name = PTC_COARSE_CULL_COUNTER_NAME;
    resManager->DeleteResource(name);
    name = PTC_TILED_INDEX_NAME;
    resManager->DeleteResource(name);
    name = PTC_DEAD_LIST_NAME;
    resManager->DeleteResource(name);
    name = PTC_ALIVE_INDEX_NAME;
    resManager->DeleteResource(name);
    name = PTC_DEAD_LIST_CONSTANT_NAME;
    resManager->DeleteResource(name);
    name = PTC_ALIVE_LIST_CONSTANT_NAME;
    resManager->DeleteResource(name);
    name = PTC_EMITTER_CONSTANT_NAME;
    resManager->DeleteResource(name);
    name = PTC_CAMERA_CONSTANT_NAME;
    resManager->DeleteResource(name);
    name = PTC_TILING_CONSTANT_NAME;
    resManager->DeleteResource(name);
    name = PTC_DEBUG_COUNTER_NAME;
    resManager->DeleteResource(name);
    name = PTC_RAMDOM_TEXTURE_NAME;
    resManager->DeleteResource(name);
    name = PTC_SIMU_EMITTER_STRU_NAME;
    resManager->DeleteResource(name);
    name = PTC_TIME_CONSTANT_NAME;
    resManager->DeleteResource(name);
}

void RSPass_PriticleSetUp::ExecuatePass()
{

}

bool RSPass_PriticleSetUp::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bfrDesc = {};
    D3D11_TEXTURE2D_DESC texDesc = {};
    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    ZeroMemory(&texDesc, sizeof(texDesc));

    bfrDesc.ByteWidth = sizeof(RS_PARTICLE_PART_A) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.Usage = D3D11_USAGE_DEFAULT;
    bfrDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bfrDesc.CPUAccessFlags = 0;
    bfrDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bfrDesc.StructureByteStride = sizeof(RS_PARTICLE_PART_A);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mParticlePartA);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(RS_PARTICLE_PART_B) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(RS_PARTICLE_PART_B);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mParticlePartB);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(DirectX::XMFLOAT4);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mViewspacePosBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(float) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(float);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mMaxRadiusBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(UINT) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(UINT);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mDeadListBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth =
        sizeof(RS_ALIVE_INDEX_BUFFER_ELEMENT) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(RS_ALIVE_INDEX_BUFFER_ELEMENT);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mAliveIndexBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.StructureByteStride = 0;
    bfrDesc.MiscFlags = 0;
    bfrDesc.ByteWidth =
        sizeof(UINT) * PTC_MAX_PARTICLE_SIZE * PTC_MAX_COARSE_CULL_TILE_SIZE;
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mStridedCoarseCullBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(UINT) * PTC_MAX_COARSE_CULL_TILE_SIZE;
    hr = Device()->CreateBuffer(&bfrDesc, nullptr,
        &mStridedCoarseCullCounterBuffer);
    if (FAILED(hr)) { return false; }

    UINT numElements = mTilingConstant.mNumTilesX * mTilingConstant.mNumTilesY *
        PTC_TILE_BUFFER_SIZE;
    bfrDesc.ByteWidth = sizeof(UINT) * numElements;
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mTiledIndexBuffer);
    if (FAILED(hr)) { return false; }

    numElements =
        mTilingConstant.mNumTilesX * mTilingConstant.mNumTilesY *
        PTC_TILE_X_SIZE * PTC_TILE_Y_SIZE;
    bfrDesc.ByteWidth = 8 * numElements;    // DXGI_FORMAT_R16G16B16A16_FLOAT
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mParticleRenderBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_DEFAULT;
    bfrDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bfrDesc.CPUAccessFlags = 0;
    bfrDesc.ByteWidth = 4 * sizeof(UINT);   // one for record and three for pad
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mDeadListConstantBuffer);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mActiveListConstantBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_DYNAMIC;
    bfrDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bfrDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bfrDesc.ByteWidth = sizeof(RS_PARTICLE_EMITTER_INFO);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mEmitterConstantBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(CAMERA_STATUS);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mCameraConstantBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(RS_TILING_CONSTANT);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mTilingConstantBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(PTC_TIME_CONSTANT);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mTimeConstantBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_STAGING;
    bfrDesc.BindFlags = 0;
    bfrDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    bfrDesc.ByteWidth = sizeof(UINT);
    hr = Device()->CreateBuffer(&bfrDesc, nullptr, &mDebugCounterBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_DYNAMIC;
    bfrDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bfrDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bfrDesc.ByteWidth = MAX_PARTICLE_EMITTER_SIZE *
        sizeof(SIMULATE_EMITTER_INFO);
    bfrDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bfrDesc.StructureByteStride = sizeof(SIMULATE_EMITTER_INFO);
    hr = Device()->CreateBuffer(
        &bfrDesc, nullptr, &mSimulEmitterStructedBuffer);
    if (FAILED(hr)) { return false; }

    texDesc.Width = 1024;
    texDesc.Height = 1024;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.MipLevels = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;

    float* values = new float[texDesc.Width * texDesc.Height * 4];
    float* ptr = values;
    for (UINT i = 0; i < texDesc.Width * texDesc.Height; i++)
    {
        ptr[0] = Tool::RandomVariance(0.0f, 1.0f);
        ptr[1] = Tool::RandomVariance(0.0f, 1.0f);
        ptr[2] = Tool::RandomVariance(0.0f, 1.0f);
        ptr[3] = Tool::RandomVariance(0.0f, 1.0f);
        ptr += 4;
    }

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = values;
    data.SysMemPitch = texDesc.Width * 16;
    data.SysMemSlicePitch = 0;

    hr = Device()->CreateTexture2D(&texDesc, &data, &mParticleRandomTexture);
    delete[] values;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleSetUp::CreateViews()
{
    HRESULT hr = S_OK;
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    ZeroMemory(&srvDesc, sizeof(srvDesc));

    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.ElementWidth = PTC_MAX_PARTICLE_SIZE;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = PTC_MAX_PARTICLE_SIZE;
    uavDesc.Buffer.Flags = 0;
    hr = Device()->CreateShaderResourceView(
        mParticlePartA, &srvDesc, &mPartA_Srv);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mParticlePartA, &uavDesc, &mPartA_Uav);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mParticlePartB, &uavDesc, &mPartB_Uav);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateShaderResourceView(
        mViewspacePosBuffer, &srvDesc, &mViewSpacePos_Srv);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mViewspacePosBuffer, &uavDesc, &mViewSpacePos_Uav);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateShaderResourceView(
        mMaxRadiusBuffer, &srvDesc, &mMaxRadius_Srv);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mMaxRadiusBuffer, &uavDesc, &mMaxRadius_Uav);
    if (FAILED(hr)) { return false; }

    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    hr = Device()->CreateUnorderedAccessView(
        mDeadListBuffer, &uavDesc, &mDeadList_Uav);
    if (FAILED(hr)) { return false; }

    uavDesc.Format = DXGI_FORMAT_R32_UINT;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements =
        PTC_MAX_PARTICLE_SIZE * PTC_MAX_COARSE_CULL_TILE_SIZE;
    srvDesc.Format = DXGI_FORMAT_R32_UINT;
    srvDesc.Buffer.NumElements =
        PTC_MAX_PARTICLE_SIZE * PTC_MAX_COARSE_CULL_TILE_SIZE;
    hr = Device()->CreateShaderResourceView(
        mStridedCoarseCullBuffer, &srvDesc, &mStridedCoarseCull_Srv);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mStridedCoarseCullBuffer, &uavDesc, &mStridedCoarseCull_Uav);
    if (FAILED(hr)) { return false; }

    uavDesc.Buffer.NumElements = PTC_MAX_COARSE_CULL_TILE_SIZE;
    srvDesc.Buffer.NumElements = PTC_MAX_COARSE_CULL_TILE_SIZE;
    hr = Device()->CreateShaderResourceView(
        mStridedCoarseCullCounterBuffer, &srvDesc,
        &mStridedCoarseCullCounter_Srv);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mStridedCoarseCullCounterBuffer, &uavDesc,
        &mStridedCoarseCullCounter_Uav);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.ElementWidth = PTC_MAX_PARTICLE_SIZE;
    uavDesc.Buffer.NumElements = PTC_MAX_PARTICLE_SIZE;
    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    hr = Device()->CreateShaderResourceView(
        mAliveIndexBuffer, &srvDesc, &mAliveIndex_Srv);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mAliveIndexBuffer, &uavDesc, &mAliveIndex_Uav);
    if (FAILED(hr)) { return false; }

    UINT numElements = mTilingConstant.mNumTilesX * mTilingConstant.mNumTilesY *
        PTC_TILE_BUFFER_SIZE;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Format = DXGI_FORMAT_R32_UINT;
    srvDesc.Buffer.ElementWidth = numElements;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Format = DXGI_FORMAT_R32_UINT;
    uavDesc.Buffer.NumElements = numElements;
    hr = Device()->CreateShaderResourceView(
        mTiledIndexBuffer, &srvDesc, &mTiledIndex_Srv);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mTiledIndexBuffer, &uavDesc, &mTiledIndex_Uav);
    if (FAILED(hr)) { return false; }

    numElements =
        mTilingConstant.mNumTilesX * mTilingConstant.mNumTilesY *
        PTC_TILE_X_SIZE * PTC_TILE_Y_SIZE;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    ZeroMemory(&uavDesc, sizeof(uavDesc));
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.Buffer.ElementWidth = numElements;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    uavDesc.Buffer.NumElements = numElements;
    hr = Device()->CreateShaderResourceView(
        mParticleRenderBuffer, &srvDesc, &mParticleRender_Srv);
    if (FAILED(hr)) { return false; }
    hr = Device()->CreateUnorderedAccessView(
        mParticleRenderBuffer, &uavDesc, &mParticleRender_Uav);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    hr = Device()->CreateShaderResourceView(
        mParticleRandomTexture, &srvDesc, &mParticleRandom_Srv);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_PARTICLE_EMITTER_SIZE;
    hr = Device()->CreateShaderResourceView(
        mSimulEmitterStructedBuffer,
        &srvDesc, &mSimulEmitterStructedBuffer_Srv);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_PriticleEmitSimulate::RSPass_PriticleEmitSimulate(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mRSParticleContainerPtr(nullptr),
    mInitDeadListShader(nullptr), mResetParticlesShader(nullptr),
    mEmitParticleShader(nullptr), mSimulateShader(nullptr),
    mDeadList_Uav(nullptr), mPartA_Uav(nullptr), mPartB_Uav(nullptr),
    mRandomTex_Srv(nullptr), mEmitterConstantBuffer(nullptr),
    mDeadListConstantBuffer(nullptr),
    mLinearWrapSampler(nullptr),
    mDepthTex_Srv(nullptr), mSimulEmitterStructedBuffer_Srv(nullptr),
    mAliveIndex_Uav(nullptr), mViewSpacePos_Uav(nullptr),
    mMaxRadius_Uav(nullptr), mSimulEmitterStructedBuffer(nullptr),
    mRSCameraInfo(nullptr), mCameraConstantBuffer(nullptr),
    mTimeConstantBuffer(nullptr)
{

}

RSPass_PriticleEmitSimulate::RSPass_PriticleEmitSimulate(
    const RSPass_PriticleEmitSimulate& _source) :
    RSPass_Base(_source),
    mRSParticleContainerPtr(_source.mRSParticleContainerPtr),
    mInitDeadListShader(_source.mInitDeadListShader),
    mResetParticlesShader(_source.mResetParticlesShader),
    mEmitParticleShader(_source.mEmitParticleShader),
    mSimulateShader(_source.mSimulateShader),
    mDeadList_Uav(_source.mDeadList_Uav),
    mPartA_Uav(_source.mPartA_Uav), mPartB_Uav(_source.mPartB_Uav),
    mRandomTex_Srv(_source.mRandomTex_Srv),
    mEmitterConstantBuffer(_source.mEmitterConstantBuffer),
    mDeadListConstantBuffer(_source.mDeadListConstantBuffer),
    mLinearWrapSampler(_source.mLinearWrapSampler),
    mDepthTex_Srv(_source.mDepthTex_Srv),
    mSimulEmitterStructedBuffer_Srv(_source.mSimulEmitterStructedBuffer_Srv),
    mAliveIndex_Uav(_source.mAliveIndex_Uav),
    mViewSpacePos_Uav(_source.mViewSpacePos_Uav),
    mMaxRadius_Uav(_source.mMaxRadius_Uav),
    mSimulEmitterStructedBuffer(_source.mSimulEmitterStructedBuffer),
    mRSCameraInfo(_source.mRSCameraInfo),
    mCameraConstantBuffer(_source.mCameraConstantBuffer),
    mTimeConstantBuffer(_source.mTimeConstantBuffer)
{

}

RSPass_PriticleEmitSimulate::~RSPass_PriticleEmitSimulate()
{

}

RSPass_PriticleEmitSimulate* RSPass_PriticleEmitSimulate::ClonePass()
{
    return new RSPass_PriticleEmitSimulate(*this);
}

bool RSPass_PriticleEmitSimulate::InitPass()
{
    if (mHasBeenInited) { return true; }

    mRSParticleContainerPtr = GetRSRoot_DX11_Singleton()->ParticlesContainer();
    if (!mRSParticleContainerPtr) { return false; }

    std::string name = "temp-cam";
    mRSCameraInfo = GetRSRoot_DX11_Singleton()->CamerasContainer()->
        GetRSCameraInfo(name);
    if (!mRSCameraInfo) { return false; }

    if (!CreateShaders()) { return false; }
    if (!CreateSampler()) { return false; }
    if (!CheckResources()) { return false; }

    mRSParticleContainerPtr->ResetRSParticleSystem();

    mHasBeenInited = true;

    return true;
}

void RSPass_PriticleEmitSimulate::ReleasePass()
{
    RS_RELEASE(mSimulateShader);
    RS_RELEASE(mEmitParticleShader);
    RS_RELEASE(mResetParticlesShader);
    RS_RELEASE(mInitDeadListShader);
}

void RSPass_PriticleEmitSimulate::ExecuatePass()
{
    if (!mRSParticleContainerPtr->GetAllParticleEmitters()->size())
    {
        return;
    }

    if (mRSParticleContainerPtr->GetResetFlg())
    {
        {
            STContext()->CSSetShader(mInitDeadListShader,
                nullptr, 0);
            ID3D11UnorderedAccessView* uav[] = { mDeadList_Uav };
            UINT initialCount[] = { 0 };
            STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
                uav, initialCount);

            STContext()->Dispatch(
                Tool::Align(PTC_MAX_PARTICLE_SIZE, 256) / 256, 1, 1);

            ZeroMemory(uav, sizeof(uav));
            STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
                uav, nullptr);
        }

        {
            STContext()->CSSetShader(mResetParticlesShader,
                nullptr, 0);
            ID3D11UnorderedAccessView* uav[] = { mPartA_Uav,mPartB_Uav };
            UINT initialCount[] = { (UINT)-1,(UINT)-1 };
            STContext()->CSSetUnorderedAccessViews(0,
                ARRAYSIZE(uav), uav, initialCount);

            STContext()->Dispatch(
                Tool::Align(PTC_MAX_PARTICLE_SIZE, 256) / 256, 1, 1);

            ZeroMemory(uav, sizeof(uav));
            STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
                uav, nullptr);
        }

        mRSParticleContainerPtr->FinishResetRSParticleSystem();
    }

    {
        STContext()->CSSetShader(mEmitParticleShader, nullptr, 0);
        ID3D11UnorderedAccessView* uav[] =
        { mPartA_Uav,mPartB_Uav,mDeadList_Uav };
        ID3D11ShaderResourceView* srv[] = { mRandomTex_Srv };
        ID3D11Buffer* cbuffer[] =
        { mEmitterConstantBuffer,mDeadListConstantBuffer,mTimeConstantBuffer };
        ID3D11SamplerState* sam[] = { mLinearWrapSampler };
        UINT initialCount[] = { (UINT)-1,(UINT)-1,(UINT)-1 };
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
            uav, initialCount);
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->CSSetConstantBuffers(0, ARRAYSIZE(cbuffer), cbuffer);
        STContext()->CSSetSamplers(0, ARRAYSIZE(sam), sam);

        auto emitters = mRSParticleContainerPtr->
            GetAllParticleEmitters();
        D3D11_MAPPED_SUBRESOURCE msr = {};
        STContext()->Map(mTimeConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        PTC_TIME_CONSTANT* time = (PTC_TIME_CONSTANT*)msr.pData;
        static float timer = 0.f;
        time->mDeltaTime = g_DeltaTimeInSecond;
        timer += g_DeltaTimeInSecond;
        time->mTotalTime = timer;
        STContext()->Unmap(mTimeConstantBuffer, 0);
        for (auto& emitter : *emitters)
        {
            auto& rsinfo = emitter->GetRSParticleEmitterInfo();
            rsinfo.mAccumulation += rsinfo.mEmitNumPerSecond *
                g_DeltaTimeInSecond;
            if (rsinfo.mAccumulation > 1.f)
            {
                float integerPart = 0.0f;
                float fraction = modf(rsinfo.mAccumulation,
                    &integerPart);
                rsinfo.mNumToEmit = (int)integerPart;
                rsinfo.mAccumulation = fraction;
            }

            STContext()->Map(mEmitterConstantBuffer, 0,
                D3D11_MAP_WRITE_DISCARD, 0, &msr);
            RS_PARTICLE_EMITTER_INFO* emitterCon =
                (RS_PARTICLE_EMITTER_INFO*)msr.pData;
            emitterCon->mEmitterIndex = rsinfo.mEmitterIndex;
            emitterCon->mEmitNumPerSecond = rsinfo.mEmitNumPerSecond;
            emitterCon->mNumToEmit = rsinfo.mNumToEmit;
            emitterCon->mAccumulation = rsinfo.mAccumulation;
            emitterCon->mPosition = rsinfo.mPosition;
            emitterCon->mVelocity = rsinfo.mVelocity;
            emitterCon->mPosVariance = rsinfo.mPosVariance;
            emitterCon->mVelVariance = rsinfo.mVelVariance;
            emitterCon->mAcceleration = rsinfo.mAcceleration;
            emitterCon->mParticleMass = rsinfo.mParticleMass;
            emitterCon->mLifeSpan = rsinfo.mLifeSpan;
            emitterCon->mOffsetStartSize = rsinfo.mOffsetStartSize;
            emitterCon->mOffsetEndSize = rsinfo.mOffsetEndSize;
            emitterCon->mOffsetStartColor = rsinfo.mOffsetStartColor;
            emitterCon->mOffsetEndColor = rsinfo.mOffsetEndColor;
            emitterCon->mTextureID = rsinfo.mTextureID;
            emitterCon->mStreakFlg = rsinfo.mStreakFlg;
            emitterCon->mMiscFlg = rsinfo.mMiscFlg;
            STContext()->Unmap(mEmitterConstantBuffer, 0);
            STContext()->CopyStructureCount(mDeadListConstantBuffer,
                0, mDeadList_Uav);

            int threadGroupNum = Tool::Align(
                rsinfo.mNumToEmit, 1024) / 1024;
            STContext()->Dispatch(threadGroupNum, 1, 1);
        }

        ZeroMemory(uav, sizeof(uav));
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
        ZeroMemory(srv, sizeof(srv));
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
    }

    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        DirectX::XMMATRIX mat = {};

        STContext()->Map(mCameraConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        CAMERA_STATUS* camStatus =
            (CAMERA_STATUS*)msr.pData;
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvViewMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mViewProj), mat);
        camStatus->mEyePosition = mRSCameraInfo->mEyePosition;
        STContext()->Unmap(mCameraConstantBuffer, 0);

        static auto emitterVec = mRSParticleContainerPtr->
            GetAllParticleEmitters();
        auto size = emitterVec->size();
        STContext()->Map(mSimulEmitterStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        SIMULATE_EMITTER_INFO* emitter =
            (SIMULATE_EMITTER_INFO*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            emitter[i].mWorldPosition = (*(*emitterVec)[i]).
                GetRSParticleEmitterInfo().mPosition;
        }
        STContext()->Unmap(mSimulEmitterStructedBuffer, 0);

        ID3D11Buffer* cb[] = { mCameraConstantBuffer };
        ID3D11ShaderResourceView* srv[] =
        { mDepthTex_Srv,mSimulEmitterStructedBuffer_Srv };
        ID3D11UnorderedAccessView* uav[] =
        { mPartA_Uav,mPartB_Uav,mDeadList_Uav,mAliveIndex_Uav,
        mViewSpacePos_Uav,mMaxRadius_Uav };
        UINT initialCount[] =
        { (UINT)-1,(UINT)-1,(UINT)-1,0,(UINT)-1,(UINT)-1 };

        STContext()->CSSetShader(mSimulateShader, nullptr, 0);
        STContext()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
            uav, initialCount);
        static int threadGroupNum = Tool::Align(
            PTC_MAX_PARTICLE_SIZE, 256) / 256;
        STContext()->Dispatch(threadGroupNum, 1, 1);

        ZeroMemory(uav, sizeof(uav));
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
        ZeroMemory(srv, sizeof(srv));
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
    }
}

bool RSPass_PriticleEmitSimulate::CreateSampler()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = Device()->CreateSamplerState(&sampDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleEmitSimulate::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_init_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mInitDeadListShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_reset_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mResetParticlesShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_emit_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mEmitParticleShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_simulate_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mSimulateShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleEmitSimulate::CheckResources()
{
    auto resManager = GetRSRoot_DX11_Singleton()->ResourceManager();
    if (!resManager) { return false; }

    std::string name = PTC_DEAD_LIST_NAME;
    mDeadList_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mDeadList_Uav) { return false; }

    name = PTC_A_NAME;
    mPartA_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mPartA_Uav) { return false; }

    name = PTC_B_NAME;
    mPartB_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mPartB_Uav) { return false; }

    name = PTC_RAMDOM_TEXTURE_NAME;
    mRandomTex_Srv = resManager->GetResourceInfo(name)->mSrv;
    if (!mRandomTex_Srv) { return false; }

    name = PTC_EMITTER_CONSTANT_NAME;
    mEmitterConstantBuffer = resManager->GetResourceInfo(name)->
        mResource.mBuffer;
    if (!mEmitterConstantBuffer) { return false; }

    name = PTC_DEAD_LIST_CONSTANT_NAME;
    mDeadListConstantBuffer = resManager->GetResourceInfo(name)->
        mResource.mBuffer;
    if (!mDeadListConstantBuffer) { return false; }

    name = PTC_CAMERA_CONSTANT_NAME;
    mCameraConstantBuffer = resManager->GetResourceInfo(name)->
        mResource.mBuffer;
    if (!mCameraConstantBuffer) { return false; }

    name = PTC_SIMU_EMITTER_STRU_NAME;
    mSimulEmitterStructedBuffer_Srv = resManager->GetResourceInfo(name)->mSrv;
    mSimulEmitterStructedBuffer = resManager->GetResourceInfo(name)->
        mResource.mBuffer;
    if (!mSimulEmitterStructedBuffer_Srv || !mSimulEmitterStructedBuffer) { return false; }

    name = PTC_ALIVE_INDEX_NAME;
    mAliveIndex_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mAliveIndex_Uav) { return false; }

    name = PTC_VIEW_SPCACE_POS_NAME;
    mViewSpacePos_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mViewSpacePos_Uav) { return false; }

    name = PTC_MAX_RADIUS_NAME;
    mMaxRadius_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mMaxRadius_Uav) { return false; }

    name = PTC_TIME_CONSTANT_NAME;
    mTimeConstantBuffer = resManager->GetResourceInfo(name)->
        mResource.mBuffer;
    if (!mTimeConstantBuffer) { return false; }

    name = "mrt-depth";
    mDepthTex_Srv = resManager->GetResourceInfo(name)->mSrv;
    if (!mDepthTex_Srv) { return false; }

    return true;
}

RSPass_PriticleTileRender::RSPass_PriticleTileRender(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mCoarseCullingShader(nullptr), mTileCullingShader(nullptr),
    mTileRenderShader(nullptr), mAliveIndex_Uav(nullptr),
    mCameraConstantBuffer(nullptr), mTilingConstantBuffer(nullptr),
    mActiveListConstantBuffer(nullptr), mDepthTex_Srv(nullptr),
    mViewSpacePos_Srv(nullptr), mMaxRadius_Srv(nullptr),
    mAliveIndex_Srv(nullptr), mPartA_Srv(nullptr),
    mCoarseTileIndex_Srv(nullptr), mCoarseTileIndex_Uav(nullptr),
    mCoarseTileIndexCounter_Srv(nullptr), mCoarseTileIndexCounter_Uav(nullptr),
    mTiledIndex_Srv(nullptr), mTiledIndex_Uav(nullptr),
    mParticleRender_Srv(nullptr), mParticleRender_Uav(nullptr),
    mLinearClampSampler(nullptr), mParticleTex_Srv(nullptr),
    mRSCameraInfo(nullptr), mParticleBlendState(nullptr),
    mBlendVertexShader(nullptr), mBlendPixelShader(nullptr)
{

}

RSPass_PriticleTileRender::RSPass_PriticleTileRender(
    const RSPass_PriticleTileRender& _source) :
    RSPass_Base(_source),
    mCoarseCullingShader(_source.mCoarseCullingShader),
    mTileCullingShader(_source.mTileCullingShader),
    mTileRenderShader(_source.mTileRenderShader),
    mCameraConstantBuffer(_source.mCameraConstantBuffer),
    mTilingConstantBuffer(_source.mTilingConstantBuffer),
    mActiveListConstantBuffer(_source.mActiveListConstantBuffer),
    mDepthTex_Srv(_source.mDepthTex_Srv),
    mViewSpacePos_Srv(_source.mViewSpacePos_Srv),
    mMaxRadius_Srv(_source.mMaxRadius_Srv),
    mAliveIndex_Srv(_source.mAliveIndex_Srv),
    mPartA_Srv(_source.mPartA_Srv),
    mCoarseTileIndex_Srv(_source.mCoarseTileIndex_Srv),
    mCoarseTileIndex_Uav(_source.mCoarseTileIndex_Uav),
    mCoarseTileIndexCounter_Srv(_source.mCoarseTileIndexCounter_Srv),
    mCoarseTileIndexCounter_Uav(_source.mCoarseTileIndexCounter_Uav),
    mTiledIndex_Srv(_source.mTiledIndex_Srv),
    mTiledIndex_Uav(_source.mTiledIndex_Uav),
    mParticleRender_Srv(_source.mParticleRender_Srv),
    mParticleRender_Uav(_source.mParticleRender_Uav),
    mLinearClampSampler(_source.mLinearClampSampler),
    mParticleTex_Srv(_source.mParticleTex_Srv),
    mAliveIndex_Uav(_source.mAliveIndex_Uav),
    mRSCameraInfo(_source.mRSCameraInfo),
    mBlendVertexShader(_source.mBlendVertexShader),
    mBlendPixelShader(_source.mBlendPixelShader),
    mParticleBlendState(_source.mParticleBlendState)
{

}

RSPass_PriticleTileRender::~RSPass_PriticleTileRender()
{

}

RSPass_PriticleTileRender* RSPass_PriticleTileRender::ClonePass()
{
    return new RSPass_PriticleTileRender(*this);
}

bool RSPass_PriticleTileRender::InitPass()
{
    if (mHasBeenInited) { return true; }

    std::string name = "temp-cam";
    mRSCameraInfo = GetRSRoot_DX11_Singleton()->CamerasContainer()->
        GetRSCameraInfo(name);
    if (!mRSCameraInfo) { return false; }

    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSampler()) { return false; }
    if (!CreateBlend()) { return false; }
    if (!CheckResources()) { return false; }

    mHasBeenInited = true;

    return true;
}

void RSPass_PriticleTileRender::ReleasePass()
{
    RS_RELEASE(mCoarseCullingShader);
    RS_RELEASE(mTileCullingShader);
    RS_RELEASE(mTileRenderShader);
    RS_RELEASE(mBlendVertexShader);
    RS_RELEASE(mBlendPixelShader);
    RS_RELEASE(mParticleBlendState);

    RS_RELEASE(mLinearClampSampler);

    RS_RELEASE(mParticleTex_Srv);
}

void RSPass_PriticleTileRender::ExecuatePass()
{
    if (!g_Root->ParticlesContainer()->GetAllParticleEmitters()->size())
    {
        return;
    }

    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        DirectX::XMMATRIX mat = {};
        STContext()->Map(mCameraConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        CAMERA_STATUS* camStatus = (CAMERA_STATUS*)msr.pData;
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvViewMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mViewProj), mat);
        camStatus->mEyePosition = mRSCameraInfo->mEyePosition;
        STContext()->Unmap(mCameraConstantBuffer, 0);
        STContext()->Map(mTilingConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_TILING_CONSTANT* tiling = (RS_TILING_CONSTANT*)msr.pData;
        tiling->mNumTilesX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumTilesX;
        tiling->mNumTilesY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumTilesY;
        tiling->mNumCoarseCullingTilesX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCoarseCullingTilesX;
        tiling->mNumCoarseCullingTilesY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCoarseCullingTilesY;
        tiling->mNumCullingTilesPerCoarseTileX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCullingTilesPerCoarseTileX;
        tiling->mNumCullingTilesPerCoarseTileY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCullingTilesPerCoarseTileY;
        STContext()->Unmap(mTilingConstantBuffer, 0);
        STContext()->CopyStructureCount(mActiveListConstantBuffer, 0,
            mAliveIndex_Uav);

        ID3D11Buffer* cb[] =
        { mCameraConstantBuffer,mTilingConstantBuffer,mActiveListConstantBuffer };
        ID3D11ShaderResourceView* srv[] =
        { mViewSpacePos_Srv,mMaxRadius_Srv,mAliveIndex_Srv };
        ID3D11UnorderedAccessView* uav[] =
        { mCoarseTileIndex_Uav,mCoarseTileIndexCounter_Uav };
        UINT initial[] = { (UINT)-1,(UINT)-1 };

        STContext()->CSSetShader(mCoarseCullingShader, nullptr, 0);
        STContext()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, initial);

        static int threadGroupNum = Tool::Align(PTC_MAX_PARTICLE_SIZE,
            PTC_COARSE_CULLING_THREADS) / PTC_COARSE_CULLING_THREADS;
        STContext()->Dispatch(threadGroupNum, 1, 1);

        ZeroMemory(cb, sizeof(cb));
        ZeroMemory(srv, sizeof(srv));
        ZeroMemory(uav, sizeof(uav));
        STContext()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
    }

    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        DirectX::XMMATRIX mat = {};
        STContext()->Map(mCameraConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        CAMERA_STATUS* camStatus = (CAMERA_STATUS*)msr.pData;
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvViewMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mViewProj), mat);
        camStatus->mEyePosition = mRSCameraInfo->mEyePosition;
        STContext()->Unmap(mCameraConstantBuffer, 0);
        STContext()->Map(mTilingConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_TILING_CONSTANT* tiling = (RS_TILING_CONSTANT*)msr.pData;
        tiling->mNumTilesX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumTilesX;
        tiling->mNumTilesY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumTilesY;
        tiling->mNumCoarseCullingTilesX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCoarseCullingTilesX;
        tiling->mNumCoarseCullingTilesY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCoarseCullingTilesY;
        tiling->mNumCullingTilesPerCoarseTileX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCullingTilesPerCoarseTileX;
        tiling->mNumCullingTilesPerCoarseTileY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCullingTilesPerCoarseTileY;
        STContext()->Unmap(mTilingConstantBuffer, 0);

        ID3D11Buffer* cb[] =
        { mCameraConstantBuffer,mTilingConstantBuffer,mActiveListConstantBuffer };
        ID3D11ShaderResourceView* srv[] =
        { mViewSpacePos_Srv,mMaxRadius_Srv,mAliveIndex_Srv,
        mDepthTex_Srv,mCoarseTileIndex_Srv,mCoarseTileIndexCounter_Srv };
        ID3D11UnorderedAccessView* uav[] = { mTiledIndex_Uav };
        UINT initial[] = { (UINT)-1 };

        STContext()->CSSetShader(mTileCullingShader, nullptr, 0);
        STContext()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, initial);

        STContext()->Dispatch(
            g_ParticleSetUpPass->GetTilingConstantInfo().mNumTilesX,
            g_ParticleSetUpPass->GetTilingConstantInfo().mNumTilesY, 1);

        ZeroMemory(cb, sizeof(cb));
        ZeroMemory(srv, sizeof(srv));
        ZeroMemory(uav, sizeof(uav));
        STContext()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
    }

    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        DirectX::XMMATRIX mat = {};
        STContext()->Map(mCameraConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        CAMERA_STATUS* camStatus = (CAMERA_STATUS*)msr.pData;
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvViewMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mInvProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mViewProjMat);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mViewProj), mat);
        camStatus->mEyePosition = mRSCameraInfo->mEyePosition;
        STContext()->Unmap(mCameraConstantBuffer, 0);
        STContext()->Map(mTilingConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_TILING_CONSTANT* tiling = (RS_TILING_CONSTANT*)msr.pData;
        tiling->mNumTilesX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumTilesX;
        tiling->mNumTilesY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumTilesY;
        tiling->mNumCoarseCullingTilesX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCoarseCullingTilesX;
        tiling->mNumCoarseCullingTilesY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCoarseCullingTilesY;
        tiling->mNumCullingTilesPerCoarseTileX = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCullingTilesPerCoarseTileX;
        tiling->mNumCullingTilesPerCoarseTileY = g_ParticleSetUpPass->
            GetTilingConstantInfo().mNumCullingTilesPerCoarseTileY;
        STContext()->Unmap(mTilingConstantBuffer, 0);

        ID3D11Buffer* cb[] =
        { mCameraConstantBuffer,mTilingConstantBuffer };
        ID3D11ShaderResourceView* srv[] =
        { mPartA_Srv,mViewSpacePos_Srv,mDepthTex_Srv,mTiledIndex_Srv,
        mCoarseTileIndexCounter_Srv,mParticleTex_Srv };
        ID3D11UnorderedAccessView* uav[] = { mParticleRender_Uav };
        UINT initial[] = { (UINT)-1 };
        ID3D11SamplerState* sam[] = { mLinearClampSampler };

        STContext()->CSSetShader(mTileRenderShader, nullptr, 0);
        STContext()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, initial);
        STContext()->CSSetSamplers(0, ARRAYSIZE(sam), sam);

        STContext()->Dispatch(
            g_ParticleSetUpPass->GetTilingConstantInfo().mNumTilesX,
            g_ParticleSetUpPass->GetTilingConstantInfo().mNumTilesY, 1);

        ZeroMemory(cb, sizeof(cb));
        ZeroMemory(srv, sizeof(srv));
        ZeroMemory(uav, sizeof(uav));
        STContext()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        STContext()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
        STContext()->CSSetShader(nullptr, nullptr, 0);
    }

    {
        static auto rtv = GetRSRoot_DX11_Singleton()->Devices()->GetSwapChainRtv();
        static D3D11_VIEWPORT vp = {};
        vp.Width = 1280.f; vp.Height = 720.f; vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f; vp.TopLeftX = 0.f; vp.TopLeftY = 0.f;
        STContext()->VSSetShader(mBlendVertexShader, nullptr, 0);
        STContext()->PSSetShader(mBlendPixelShader, nullptr, 0);
        STContext()->OMSetBlendState(mParticleBlendState, nullptr, 0xFFFFFFFF);
        STContext()->OMSetRenderTargets(1, &rtv, nullptr);
        STContext()->RSSetViewports(1, &vp);
        STContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        STContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ID3D11ShaderResourceView* srv[] = { mParticleRender_Srv };
        STContext()->PSSetShaderResources(0, ARRAYSIZE(srv), srv);

        STContext()->Draw(3, 0);

        ZeroMemory(srv, sizeof(srv));
        STContext()->PSSetShaderResources(0, ARRAYSIZE(srv), srv);
        STContext()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    }
}

bool RSPass_PriticleTileRender::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_coarse_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mCoarseCullingShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_cull_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mTileCullingShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_render_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mTileRenderShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_blend_vertex.hlsl",
        "Main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mBlendVertexShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(L".\\Assets\\Shaders\\ptc_blend_pixel.hlsl",
        "Main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mBlendPixelShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleTileRender::CreateViews()
{
    HRESULT hr = S_OK;

    std::wstring path = L".\\Assets\\Textures\\particle_atlas.dds";
    hr = DirectX::CreateDDSTextureFromFile(Device(), path.c_str(),
        nullptr, &mParticleTex_Srv);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleTileRender::CreateSampler()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = Device()->CreateSamplerState(&sampDesc, &mLinearClampSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleTileRender::CreateBlend()
{
    HRESULT hr = S_OK;

    D3D11_BLEND_DESC bldDesc = {};
    ZeroMemory(&bldDesc, sizeof(D3D11_BLEND_DESC));
    bldDesc.AlphaToCoverageEnable = false;
    bldDesc.IndependentBlendEnable = false;
    bldDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    bldDesc.RenderTarget[0].BlendEnable = true;
    bldDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    bldDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bldDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bldDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    bldDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bldDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    hr = Device()->CreateBlendState(&bldDesc, &mParticleBlendState);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleTileRender::CheckResources()
{
    auto resManager = GetRSRoot_DX11_Singleton()->ResourceManager();
    if (!resManager) { return false; }

    std::string name = PTC_CAMERA_CONSTANT_NAME;
    mCameraConstantBuffer = resManager->GetResourceInfo(name)->
        mResource.mBuffer;
    if (!mCameraConstantBuffer) { return false; }

    name = PTC_TILING_CONSTANT_NAME;
    mTilingConstantBuffer = resManager->GetResourceInfo(name)->
        mResource.mBuffer;
    if (!mTilingConstantBuffer) { return false; }

    name = PTC_ALIVE_LIST_CONSTANT_NAME;
    mActiveListConstantBuffer = resManager->GetResourceInfo(name)->
        mResource.mBuffer;
    if (!mActiveListConstantBuffer) { return false; }

    name = "mrt-depth";
    mDepthTex_Srv = resManager->GetResourceInfo(name)->mSrv;
    if (!mDepthTex_Srv) { return false; }

    name = PTC_VIEW_SPCACE_POS_NAME;
    mViewSpacePos_Srv = resManager->GetResourceInfo(name)->mSrv;
    if (!mViewSpacePos_Srv) { return false; }

    name = PTC_MAX_RADIUS_NAME;
    mMaxRadius_Srv = resManager->GetResourceInfo(name)->mSrv;
    if (!mMaxRadius_Srv) { return false; }

    name = PTC_ALIVE_INDEX_NAME;
    mAliveIndex_Srv = resManager->GetResourceInfo(name)->mSrv;
    mAliveIndex_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mAliveIndex_Srv) { return false; }
    if (!mAliveIndex_Uav) { return false; }

    name = PTC_A_NAME;
    mPartA_Srv = resManager->GetResourceInfo(name)->mSrv;
    if (!mPartA_Srv) { return false; }

    name = PTC_COARSE_CULL_NAME;
    mCoarseTileIndex_Srv = resManager->GetResourceInfo(name)->mSrv;
    mCoarseTileIndex_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mCoarseTileIndex_Srv) { return false; }
    if (!mCoarseTileIndex_Uav) { return false; }

    name = PTC_COARSE_CULL_COUNTER_NAME;
    mCoarseTileIndexCounter_Srv = resManager->GetResourceInfo(name)->mSrv;
    mCoarseTileIndexCounter_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mCoarseTileIndexCounter_Srv) { return false; }
    if (!mCoarseTileIndexCounter_Uav) { return false; }

    name = PTC_TILED_INDEX_NAME;
    mTiledIndex_Srv = resManager->GetResourceInfo(name)->mSrv;
    mTiledIndex_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mTiledIndex_Srv) { return false; }
    if (!mTiledIndex_Uav) { return false; }

    name = PTC_RENDER_BUFFER_NAME;
    mParticleRender_Srv = resManager->GetResourceInfo(name)->mSrv;
    mParticleRender_Uav = resManager->GetResourceInfo(name)->mUav;
    if (!mParticleRender_Srv) { return false; }
    if (!mParticleRender_Uav) { return false; }

    return true;
}

RSPass_Sprite::RSPass_Sprite(std::string& _name,
    PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mDepthStencilState(nullptr), mRenderTargetView(nullptr),
    mProjStructedBuffer(nullptr), mProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
    mLinearSampler(nullptr), mBlendState(nullptr),
    mDrawCallType(DRAWCALL_TYPE::MAX), mDrawCallPipe(nullptr),
    mRSCameraInfo(nullptr)
{

}

RSPass_Sprite::RSPass_Sprite(
    const RSPass_Sprite& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mDepthStencilState(_source.mDepthStencilState),
    mBlendState(_source.mBlendState),
    mRenderTargetView(_source.mRenderTargetView),
    mDrawCallType(_source.mDrawCallType),
    mDrawCallPipe(_source.mDrawCallPipe),
    mProjStructedBuffer(_source.mProjStructedBuffer),
    mProjStructedBufferSrv(_source.mProjStructedBufferSrv),
    mInstanceStructedBuffer(_source.mInstanceStructedBuffer),
    mInstanceStructedBufferSrv(_source.mInstanceStructedBufferSrv),
    mLinearSampler(_source.mLinearSampler),
    mRSCameraInfo(_source.mRSCameraInfo)
{
    if (mHasBeenInited)
    {
        RS_ADD(mVertexShader);
        RS_ADD(mPixelShader);
        RS_ADD(mDepthStencilState);
        RS_ADD(mLinearSampler);
        RS_ADD(mProjStructedBufferSrv);
        RS_ADD(mProjStructedBuffer);
        RS_ADD(mInstanceStructedBufferSrv);
        RS_ADD(mInstanceStructedBuffer);
    }
}

RSPass_Sprite::~RSPass_Sprite()
{

}

RSPass_Sprite* RSPass_Sprite::ClonePass()
{
    return new RSPass_Sprite(*this);
}

bool RSPass_Sprite::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateStates()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallType = DRAWCALL_TYPE::UI_SPRITE;
    mDrawCallPipe = g_Root->DrawCallsPool()->
        GetDrawCallsPipe(mDrawCallType);

    std::string name = "temp-ui-cam";
    mRSCameraInfo = g_Root->CamerasContainer()->
        GetRSCameraInfo(name);

    mHasBeenInited = true;

    return true;
}

void RSPass_Sprite::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mDepthStencilState);
    RS_RELEASE(mLinearSampler);
    RS_RELEASE(mProjStructedBufferSrv);
    RS_RELEASE(mProjStructedBuffer);
    RS_RELEASE(mInstanceStructedBufferSrv);
    RS_RELEASE(mInstanceStructedBuffer);
}

void RSPass_Sprite::ExecuatePass()
{
    ID3D11RenderTargetView* rtvnull = nullptr;
    STContext()->OMSetRenderTargets(1,
        &mRenderTargetView, nullptr);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->OMSetDepthStencilState(mDepthStencilState, 0);
    static float factor[4] = { 0.f,0.f,0.f,0.f };
    STContext()->OMSetBlendState(mBlendState, factor, 0xFFFFFFFF);
    STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);
    STContext()->PSSetSamplers(0, 1, &mLinearSampler);

    DirectX::XMMATRIX mat = {};
    DirectX::XMFLOAT4X4 flt44 = {};
    UINT stride = sizeof(VertexType::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    STContext()->Map(mProjStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    OnlyProj* vp_data = (OnlyProj*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->mProjMat);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
    STContext()->Unmap(mProjStructedBuffer, 0);

    STContext()->VSSetShaderResources(
        0, 1, &mProjStructedBufferSrv);

    for (auto& call : mDrawCallPipe->mDatas)
    {
        auto vecPtr = call.mInstanceData.mDataPtr;
        auto size = vecPtr->size();
        STContext()->Map(mInstanceStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            mat = DirectX::XMLoadFloat4x4(
                &(*vecPtr)[i].mWorldMat);
            mat = DirectX::XMMatrixTranspose(mat);
            DirectX::XMStoreFloat4x4(&ins_data[i].mWorldMat, mat);
            ins_data[i].mMaterialData =
                (*vecPtr)[i].mMaterialData;
            ins_data[i].mCustomizedData1 =
                (*vecPtr)[i].mCustomizedData1;
            ins_data[i].mCustomizedData2 =
                (*vecPtr)[i].mCustomizedData2;
        }
        STContext()->Unmap(mInstanceStructedBuffer, 0);

        STContext()->IASetInputLayout(
            call.mMeshData.mLayout);
        STContext()->IASetPrimitiveTopology(
            call.mMeshData.mTopologyType);
        STContext()->IASetVertexBuffers(
            0, 1, &call.mMeshData.mVertexBuffer,
            &stride, &offset);
        STContext()->IASetIndexBuffer(
            call.mMeshData.mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        STContext()->VSSetShaderResources(
            1, 1, &mInstanceStructedBufferSrv);
        STContext()->PSSetShaderResources(
            0, 1, &(call.mTextureDatas[0].mSrv));

        STContext()->DrawIndexedInstanced(
            call.mMeshData.mIndexCount,
            (UINT)call.mInstanceData.mDataPtr->size(), 0, 0, 0);
    }

    STContext()->OMSetRenderTargets(1, &rtvnull, nullptr);
    STContext()->OMSetDepthStencilState(nullptr, 0);
    STContext()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

bool RSPass_Sprite::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\sprite_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\sprite_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Sprite::CreateStates()
{
    HRESULT hr = S_OK;

    D3D11_DEPTH_STENCIL_DESC depDesc = {};
    depDesc.DepthEnable = FALSE;
    depDesc.StencilEnable = FALSE;
    hr = Device()->CreateDepthStencilState(
        &depDesc, &mDepthStencilState);
    if (FAILED(hr)) { return false; }

    D3D11_BLEND_DESC bldDesc = {};
    bldDesc.AlphaToCoverageEnable = FALSE;
    bldDesc.IndependentBlendEnable = FALSE;
    bldDesc.RenderTarget[0].BlendEnable = TRUE;
    bldDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D11_COLOR_WRITE_ENABLE_ALL;
    bldDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bldDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bldDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bldDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bldDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bldDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    hr = Device()->CreateBlendState(&bldDesc, &mBlendState);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Sprite::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bdc = {};

    ZeroMemory(&bdc, sizeof(bdc));
    bdc.Usage = D3D11_USAGE_DYNAMIC;
    bdc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bdc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bdc.ByteWidth = MAX_INSTANCE_SIZE * sizeof(RS_INSTANCE_DATA);
    bdc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bdc.StructureByteStride = sizeof(RS_INSTANCE_DATA);
    hr = Device()->CreateBuffer(
        &bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = sizeof(OnlyProj);
    bdc.StructureByteStride = sizeof(OnlyProj);
    hr = Device()->CreateBuffer(
        &bdc, nullptr, &mProjStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Sprite::CreateViews()
{
    mRenderTargetView = g_Root->Devices()->GetSwapChainRtv();

    D3D11_SHADER_RESOURCE_VIEW_DESC desSRV = {};
    HRESULT hr = S_OK;
    ZeroMemory(&desSRV, sizeof(desSRV));
    desSRV.Format = DXGI_FORMAT_UNKNOWN;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desSRV.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
    hr = Device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &desSRV, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    desSRV.Buffer.ElementWidth = 1;
    hr = Device()->CreateShaderResourceView(
        mProjStructedBuffer,
        &desSRV, &mProjStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Sprite::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    auto filter = g_RenderEffectConfig.mSamplerLevel;
    switch (filter)
    {
    case SAMPLER_LEVEL::POINT:
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        break;
    default:
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    }
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = Device()->CreateSamplerState(
        &sampDesc, &mLinearSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_SimpleLight::RSPass_SimpleLight(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mLinearWrapSampler(nullptr), mRenderTargetView(nullptr),
    mVertexBuffer(nullptr), mIndexBuffer(nullptr),
    mSsaoSrv(nullptr), mDiffuseSrv(nullptr), mDiffuseAlbedoSrv(nullptr)
{

}

RSPass_SimpleLight::RSPass_SimpleLight(const RSPass_SimpleLight& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mRenderTargetView(_source.mRenderTargetView),
    mLinearWrapSampler(_source.mLinearWrapSampler),
    mSsaoSrv(_source.mSsaoSrv),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer),
    mDiffuseSrv(_source.mDiffuseSrv),
    mDiffuseAlbedoSrv(_source.mDiffuseAlbedoSrv)
{

}

RSPass_SimpleLight::~RSPass_SimpleLight()
{

}

RSPass_SimpleLight* RSPass_SimpleLight::ClonePass()
{
    return new RSPass_SimpleLight(*this);
}

bool RSPass_SimpleLight::InitPass()
{
    if (mHasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mHasBeenInited = true;

    return true;
}

void RSPass_SimpleLight::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mLinearWrapSampler);
    RS_RELEASE(mVertexBuffer);
    RS_RELEASE(mIndexBuffer);
}

void RSPass_SimpleLight::ExecuatePass()
{
    STContext()->OMSetRenderTargets(1, &mRenderTargetView, nullptr);
    STContext()->RSSetViewports(1, &g_ViewPort);
    STContext()->ClearRenderTargetView(
        mRenderTargetView, DirectX::Colors::DarkGreen);
    STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);

    UINT stride = sizeof(VertexType::TangentVertex);
    UINT offset = 0;

    static ID3D11ShaderResourceView* srvs[] =
    {
        mDiffuseSrv, mDiffuseAlbedoSrv, mSsaoSrv
    };
    STContext()->PSSetShaderResources(0, 3, srvs);

    static ID3D11SamplerState* samps[] =
    {
        mLinearWrapSampler,
    };
    STContext()->PSSetSamplers(0, 1, samps);

    STContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    STContext()->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    STContext()->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    STContext()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    ID3D11RenderTargetView* rtvnull = nullptr;
    STContext()->OMSetRenderTargets(1, &rtvnull, nullptr);
    static ID3D11ShaderResourceView* nullsrvs[] =
    {
        nullptr, nullptr, nullptr
    };
    STContext()->PSSetShaderResources(0, 3, nullsrvs);
}

bool RSPass_SimpleLight::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\simplylit_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = Tool::CompileShaderFromFile(
        L".\\Assets\\Shaders\\simplylit_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = Device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_SimpleLight::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bufDesc = {};

    VertexType::TangentVertex v[4] = {};
    v[0].Position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
    v[1].Position = DirectX::XMFLOAT3(-1.0f, +1.0f, 0.0f);
    v[2].Position = DirectX::XMFLOAT3(+1.0f, +1.0f, 0.0f);
    v[3].Position = DirectX::XMFLOAT3(+1.0f, -1.0f, 0.0f);
    v[0].TexCoord = DirectX::XMFLOAT2(0.0f, 1.0f);
    v[1].TexCoord = DirectX::XMFLOAT2(0.0f, 0.0f);
    v[2].TexCoord = DirectX::XMFLOAT2(1.0f, 0.0f);
    v[3].TexCoord = DirectX::XMFLOAT2(1.0f, 1.0f);
    ZeroMemory(&bufDesc, sizeof(bufDesc));
    bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufDesc.ByteWidth = sizeof(VertexType::TangentVertex) * 4;
    bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = 0;
    bufDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = Device()->CreateBuffer(&bufDesc, &vinitData, &mVertexBuffer);
    if (FAILED(hr)) { return false; }

    UINT indices[6] =
    {
        0, 1, 2,
        0, 2, 3
    };
    ZeroMemory(&bufDesc, sizeof(bufDesc));
    bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufDesc.ByteWidth = sizeof(UINT) * 6;
    bufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.StructureByteStride = 0;
    bufDesc.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData = {};
    ZeroMemory(&iinitData, sizeof(iinitData));
    iinitData.pSysMem = indices;
    hr = Device()->CreateBuffer(&bufDesc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_SimpleLight::CreateViews()
{
    mRenderTargetView = g_Root->Devices()->GetSwapChainRtv();

    std::string name = "mrt-diffuse";
    mDiffuseSrv = g_Root->ResourceManager()->GetResourceInfo(name)->mSrv;
    name = "mrt-diffuse-albedo";
    mDiffuseAlbedoSrv = g_Root->ResourceManager()->GetResourceInfo(name)->mSrv;
    name = "ssao-tex-compress-ssao";
    mSsaoSrv = g_Root->ResourceManager()->GetResourceInfo(name)->mSrv;

    return true;
}

bool RSPass_SimpleLight::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    auto filter = g_RenderEffectConfig.mSamplerLevel;
    switch (filter)
    {
    case SAMPLER_LEVEL::POINT:
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        break;
    case SAMPLER_LEVEL::BILINEAR:
        sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        break;
    case SAMPLER_LEVEL::ANISO_8X:
        sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampDesc.MaxAnisotropy = 8;
        break;
    case SAMPLER_LEVEL::ANISO_16X:
        sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
        sampDesc.MaxAnisotropy = 16;
        break;
    default: return false;
    }
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = Device()->CreateSamplerState(
        &sampDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    return true;
}
