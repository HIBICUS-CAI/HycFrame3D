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
#include <TextUtility.h>

#define RS_RELEASE(p) { if (p) { (p)->Release(); (p)=nullptr; } }
#define RS_ADDREF(p) { if(p) { p->AddRef(); } }
static RSRoot_DX11* g_Root = nullptr;
static RSPass_PriticleSetUp* g_ParticleSetUpPass = nullptr;
static RSPipeline* g_BasicPipeline = nullptr;
static RSPipeline* g_SimplePipeline = nullptr;
static D3D11_VIEWPORT g_ViewPort = {};

static float g_DeltaTimeInSecond = 0.f;

static ID3D11ShaderResourceView* g_IblBrdfSrv = nullptr;
static ID3D11ShaderResourceView* g_EnviMapSrv = nullptr;
static ID3D11ShaderResourceView* g_DiffMapSrv = nullptr;
static ID3D11ShaderResourceView* g_SpecMapSrv = nullptr;

enum class SAMPLER_LEVEL
{
    POINT = 0,
    BILINEAR = 1,
    ANISO_8X = 2,
    ANISO_16X = 3
};

struct RENDER_EFFECT_CONFIG
{
    bool mSimplyLitOn = true;
    std::string mLightModel = "brdf_disney";

    float mSsaoRadius = 0.5f;
    float mSsaoStart = 0.2f;
    float mSsaoEnd = 1.f;
    float mSsaoEpsilon = 0.05f;
    UINT mSsaoBlurCount = 4;

    SAMPLER_LEVEL mSamplerLevel = SAMPLER_LEVEL::ANISO_16X;

    bool mParticleOff = false;

    bool mBloomOff = false;
    float mBloomMinValue = 2.f;
    UINT mBloomDownSamplingCount = 7;
    UINT mBloomBlurCount = 2;
    UINT mBloomBlurKernel = 5;
    float mBloomBlurSigma = 1.f;
    float mBloomIntensityFactor = 1.f;
    float mBloomLightPixelFactor = 0.02f;

    bool mDynamicExpoOff = false;
    float mStaticExpo = 0.2f;
    float mExpoTransSpeed = 0.05f;
    float mExpoMin = 0.01f;
    float mExpoMax = 10.f;
    float mExpoInvFactor = 25.f;

    bool mFXAAOff = false;
    float mFXAAThreshould = 0.125f;
    float mFXAAMinThreshould = 0.0625f;
    UINT mFXAASearchStep = 10;
    UINT mFXAAGuess = 8;
};

static RENDER_EFFECT_CONFIG g_RenderEffectConfig = {};

void SetPipelineDeltaTime(float _deltaMilliSecond)
{
    g_DeltaTimeInSecond = _deltaMilliSecond / 1000.f;
}

void SetPipelineIBLTextures(ID3D11ShaderResourceView* _envSrv,
    ID3D11ShaderResourceView* _diffSrv,
    ID3D11ShaderResourceView* _specSrv)
{
    g_EnviMapSrv = _envSrv;
    g_DiffMapSrv = _diffSrv;
    g_SpecMapSrv = _specSrv;
}

bool CreateBasicPipeline()
{
    {
        using namespace hyc;
        using namespace hyc::text;
        TomlNode config = {};
        TomlNode node = {};
        std::string errorMess = "";
        if (!loadTomlAndParse(config,
            ".\\Assets\\Configs\\render-effect-config.toml",
            errorMess))
        {
            return false;
        }

        if (!getTomlNode(config, "pipeline", node))
        {
            return false;
        }
        g_RenderEffectConfig.mSimplyLitOn = getAs<bool>(node["simply-lit"]);
        g_RenderEffectConfig.mLightModel = getAs<std::string>(node["light-model"]);
        if (g_RenderEffectConfig.mLightModel != "brdf_disney" &&
            g_RenderEffectConfig.mLightModel != "blinn_phong")
        {
            g_RenderEffectConfig.mLightModel = "brdf_disney";
        }

        if (!getTomlNode(config, "ssao", node))
        {
            return false;
        }
        g_RenderEffectConfig.mSsaoRadius = getAs<float>(node["radius"]);
        g_RenderEffectConfig.mSsaoStart = getAs<float>(node["range-start"]);
        g_RenderEffectConfig.mSsaoEnd = getAs<float>(node["range-end"]);
        g_RenderEffectConfig.mSsaoEpsilon = getAs<float>(node["epsilon"]);
        g_RenderEffectConfig.mSsaoBlurCount = getAs<uint>(node["blur-count"]);

        if (!getTomlNode(config, "sampler", node))
        {
            return false;
        }
        g_RenderEffectConfig.mSamplerLevel =
            (SAMPLER_LEVEL)(getAs<uint>(node["filter-level"]));
        if ((UINT)g_RenderEffectConfig.mSamplerLevel < 0 ||
            (UINT)g_RenderEffectConfig.mSamplerLevel > 3)
        {
            return false;
        }

        if (!getTomlNode(config, "particle", node))
        {
            return false;
        }
        g_RenderEffectConfig.mParticleOff = getAs<bool>(node["disable-particle"]);

        if (!getTomlNode(config, "bloom", node))
        {
            return false;
        }
        g_RenderEffectConfig.mBloomOff = getAs<bool>(node["disable-bloom"]);
        if (g_RenderEffectConfig.mLightModel == "blinn_phong")
        {
            g_RenderEffectConfig.mBloomOff = true;
        }
        g_RenderEffectConfig.mBloomMinValue =
            getAs<float>(node["min-luminous"]);
        g_RenderEffectConfig.mBloomDownSamplingCount =
            getAs<uint>(node["downsampling-count"]);
        g_RenderEffectConfig.mBloomBlurCount =
            getAs<uint>(node["downsampling-blur-count"]);
        g_RenderEffectConfig.mBloomBlurKernel =
            getAs<uint>(node["gauss-kernel-size"]);
        g_RenderEffectConfig.mBloomBlurSigma =
            getAs<float>(node["gauss-sigma"]);
        g_RenderEffectConfig.mBloomIntensityFactor =
            getAs<float>(node["intensity-factor"]);
        g_RenderEffectConfig.mBloomLightPixelFactor =
            getAs<float>(node["light-source-factor"]);

        if (!getTomlNode(config, "exposure", node))
        {
            return false;
        }
        g_RenderEffectConfig.mDynamicExpoOff = getAs<bool>(node["disable-dynamic"]);
        g_RenderEffectConfig.mStaticExpo = getAs<float>(node["static-exposure"]);
        g_RenderEffectConfig.mExpoTransSpeed = getAs<float>(node["trans-speed"]);
        g_RenderEffectConfig.mExpoMin = getAs<float>(node["min-value"]);
        g_RenderEffectConfig.mExpoMax = getAs<float>(node["max-value"]);
        g_RenderEffectConfig.mExpoInvFactor = getAs<float>(node["inverse-factor"]);

        if (!getTomlNode(config, "fxaa", node))
        {
            return false;
        }
        g_RenderEffectConfig.mFXAAOff = getAs<bool>(node["disable-fxaa"]);
        g_RenderEffectConfig.mFXAAThreshould = getAs<float>(node["threshold"]);
        g_RenderEffectConfig.mFXAAMinThreshould = getAs<float>(node["min-threshold"]);
        g_RenderEffectConfig.mFXAASearchStep = getAs<uint>(node["search-step"]);
        g_RenderEffectConfig.mFXAAGuess = getAs<uint>(node["edge-guess"]);
        if (!g_RenderEffectConfig.mFXAASearchStep)
        {
            g_RenderEffectConfig.mFXAAOff = true;
        }
    }

    g_Root = getRSDX11RootInstance();
    std::string name = "";

    name = "mrt-pass";
    RSPass_MRT* mrt = new RSPass_MRT(name, PASS_TYPE::RENDER, g_Root);
    mrt->setExecuateOrder(1);

    name = "mrt-topic";
    RSTopic* mrt_topic = new RSTopic(name);
    mrt_topic->startAssembly();
    mrt_topic->insertPass(mrt);
    mrt_topic->setExecuateOrder(1);
    mrt_topic->finishAssembly();

    name = "basic-ssao";
    RSPass_Ssao* ssao = new RSPass_Ssao(
        name, PASS_TYPE::RENDER, g_Root);
    ssao->setExecuateOrder(2);

    name = "kbblur-ssao";
    RSPass_KBBlur* kbblur = new RSPass_KBBlur(
        name, PASS_TYPE::COMPUTE, g_Root);
    kbblur->setExecuateOrder(3);

    name = "ssao-topic";
    RSTopic* ssao_topic = new RSTopic(name);
    ssao_topic->startAssembly();
    ssao_topic->insertPass(ssao);
    ssao_topic->insertPass(kbblur);
    ssao_topic->setExecuateOrder(2);
    ssao_topic->finishAssembly();

    name = "basic-shadowmap";
    RSPass_Shadow* shadow = new RSPass_Shadow(
        name, PASS_TYPE::RENDER, g_Root);
    shadow->setExecuateOrder(1);

    name = "shadowmap-topic";
    RSTopic* shadow_topic = new RSTopic(name);
    shadow_topic->startAssembly();
    shadow_topic->insertPass(shadow);
    shadow_topic->setExecuateOrder(3);
    shadow_topic->finishAssembly();

    name = "defered-light";
    RSPass_Defered* defered = new RSPass_Defered(
        name, PASS_TYPE::RENDER, g_Root);
    defered->setExecuateOrder(1);

    name = "defered-light-topic";
    RSTopic* defered_topic = new RSTopic(name);
    defered_topic->startAssembly();
    defered_topic->insertPass(defered);
    defered_topic->setExecuateOrder(4);
    defered_topic->finishAssembly();

    name = "sky-skysphere";
    RSPass_SkyShpere* skysphere = new RSPass_SkyShpere(
        name, PASS_TYPE::RENDER, g_Root);
    skysphere->setExecuateOrder(1);

    name = "skysphere-topic";
    RSTopic* sky_topic = new RSTopic(name);
    sky_topic->startAssembly();
    sky_topic->insertPass(skysphere);
    sky_topic->setExecuateOrder(5);
    sky_topic->finishAssembly();

    name = "billboard-pass";
    RSPass_Billboard* billboard = new RSPass_Billboard(
        name, PASS_TYPE::RENDER, g_Root);
    billboard->setExecuateOrder(1);

    name = "billboard-topic";
    RSTopic* billboard_topic = new RSTopic(name);
    billboard_topic->startAssembly();
    billboard_topic->insertPass(billboard);
    billboard_topic->setExecuateOrder(6);
    billboard_topic->finishAssembly();

    name = "bloomdraw-pass";
    RSPass_Bloom* bloomdraw = new RSPass_Bloom(
        name, PASS_TYPE::RENDER, g_Root);
    bloomdraw->setExecuateOrder(1);

    name = "bloom-topic";
    RSTopic* bloom_topic = new RSTopic(name);
    bloom_topic->startAssembly();
    bloom_topic->insertPass(bloomdraw);
    bloom_topic->setExecuateOrder(7);
    bloom_topic->finishAssembly();

    RSTopic* particle_topic = nullptr;
    if (!g_RenderEffectConfig.mParticleOff)
    {
        name = "particle-setup-pass";
        RSPass_PriticleSetUp* ptcsetup = new RSPass_PriticleSetUp(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptcsetup->setExecuateOrder(1);

        name = "particle-emit-simulate-pass";
        RSPass_PriticleEmitSimulate* ptcemitsimul = new RSPass_PriticleEmitSimulate(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptcemitsimul->setExecuateOrder(2);

        name = "particle-tile-render-pass";
        RSPass_PriticleTileRender* ptctile = new RSPass_PriticleTileRender(
            name, PASS_TYPE::COMPUTE, g_Root);
        ptctile->setExecuateOrder(3);

        name = "paricle-topic";
        particle_topic = new RSTopic(name);
        particle_topic->startAssembly();
        particle_topic->insertPass(ptcsetup);
        particle_topic->insertPass(ptcemitsimul);
        particle_topic->insertPass(ptctile);
        particle_topic->setExecuateOrder(8);
        particle_topic->finishAssembly();
    }

    name = "sprite-ui";
    RSPass_Sprite* sprite = new RSPass_Sprite(
        name, PASS_TYPE::RENDER, g_Root);
    sprite->setExecuateOrder(1);

    name = "sprite-topic";
    RSTopic* sprite_topic = new RSTopic(name);
    sprite_topic->startAssembly();
    sprite_topic->insertPass(sprite);
    sprite_topic->setExecuateOrder(10);
    sprite_topic->finishAssembly();

    name = "tonemapping-pass";
    RSPass_Tonemapping* tonemap = new RSPass_Tonemapping(
        name, PASS_TYPE::COMPUTE, g_Root);
    tonemap->setExecuateOrder(2);

    RSPass_BloomHdr* bloomhdr = nullptr;
    if (!g_RenderEffectConfig.mBloomOff)
    {
        name = "bloom-hdr-pass";
        bloomhdr = new RSPass_BloomHdr(name, PASS_TYPE::COMPUTE, g_Root);
        bloomhdr->setExecuateOrder(1);
    }

    name = "fxaa-pass";
    RSPass_FXAA* fxaa = new RSPass_FXAA(name, PASS_TYPE::COMPUTE, g_Root);
    fxaa->setExecuateOrder(3);

    name = "to-swapchain-pass";
    RSPass_ToSwapChain* toswap = new RSPass_ToSwapChain(
        name, PASS_TYPE::RENDER, g_Root);
    toswap->setExecuateOrder(4);

    name = "post-processing-topic";
    RSTopic* post_procsssing_topic = new RSTopic(name);
    post_procsssing_topic->startAssembly();
    post_procsssing_topic->insertPass(tonemap);
    if (!g_RenderEffectConfig.mFXAAOff)
    {
        post_procsssing_topic->insertPass(fxaa);
    }
    post_procsssing_topic->insertPass(toswap);
    if (!g_RenderEffectConfig.mBloomOff)
    {
        post_procsssing_topic->insertPass(bloomhdr);
    }
    post_procsssing_topic->setExecuateOrder(9);
    post_procsssing_topic->finishAssembly();

    name = "light-pipeline";
    g_BasicPipeline = new RSPipeline(name);
    g_BasicPipeline->startAssembly();
    g_BasicPipeline->insertTopic(mrt_topic);
    g_BasicPipeline->insertTopic(ssao_topic);
    g_BasicPipeline->insertTopic(shadow_topic);
    g_BasicPipeline->insertTopic(defered_topic);
    g_BasicPipeline->insertTopic(sky_topic);
    g_BasicPipeline->insertTopic(bloom_topic);
    g_BasicPipeline->insertTopic(billboard_topic);
    if (!g_RenderEffectConfig.mParticleOff)
    {
        g_BasicPipeline->insertTopic(particle_topic);
    }
    g_BasicPipeline->insertTopic(post_procsssing_topic);
    g_BasicPipeline->insertTopic(sprite_topic);
    g_BasicPipeline->finishAssembly();

    if (!g_BasicPipeline->initAllTopics(g_Root->getDevices()))
    {
        return false;
    }

    name = g_BasicPipeline->getPipelineName();
    g_Root->getPipelinesManager()->addPipeline(name, g_BasicPipeline);
    g_Root->getPipelinesManager()->setPipeline(name);
    g_Root->getPipelinesManager()->useNextPipeline();

    name = "simp-mrt-pass";
    RSPass_MRT* simp_mrt = mrt->clonePass();
    simp_mrt->setExecuateOrder(1);

    name = "simp-mrt-topic";
    RSTopic* simp_mrt_topic = new RSTopic(name);
    simp_mrt_topic->startAssembly();
    simp_mrt_topic->insertPass(simp_mrt);
    simp_mrt_topic->setExecuateOrder(1);
    simp_mrt_topic->finishAssembly();

    name = "simp-basic-ssao";
    RSPass_Ssao* simp_ssao = ssao->clonePass();
    simp_ssao->setExecuateOrder(1);

    name = "simp-ssao-topic";
    RSTopic* simp_ssao_topic = new RSTopic(name);
    simp_ssao_topic->startAssembly();
    simp_ssao_topic->insertPass(simp_ssao);
    simp_ssao_topic->setExecuateOrder(2);
    simp_ssao_topic->finishAssembly();

    name = "simp-lit-shadowmap";
    RSPass_SimpleLight* simp_lit = new RSPass_SimpleLight(
        name, PASS_TYPE::RENDER, g_Root);
    simp_lit->setExecuateOrder(1);

    name = "simp-lit-topic";
    RSTopic* simp_lit_topic = new RSTopic(name);
    simp_lit_topic->startAssembly();
    simp_lit_topic->insertPass(simp_lit);
    simp_lit_topic->setExecuateOrder(3);
    simp_lit_topic->finishAssembly();

    name = "simp-billboard";
    RSPass_Billboard* simp_bill = billboard->clonePass();
    simp_bill->setExecuateOrder(1);

    name = "simp-billboard-topic";
    RSTopic* simp_bill_topic = new RSTopic(name);
    simp_bill_topic->startAssembly();
    simp_bill_topic->insertPass(simp_bill);
    simp_bill_topic->setExecuateOrder(4);
    simp_bill_topic->finishAssembly();

    name = "simp-sprite-ui";
    RSPass_Sprite* simp_sprite = sprite->clonePass();
    simp_sprite->setExecuateOrder(1);

    name = "simp-sprite-topic";
    RSTopic* simp_sprite_topic = new RSTopic(name);
    simp_sprite_topic->startAssembly();
    simp_sprite_topic->insertPass(simp_sprite);
    simp_sprite_topic->setExecuateOrder(5);
    simp_sprite_topic->finishAssembly();

    name = "simple-pipeline";
    g_SimplePipeline = new RSPipeline(name);
    g_SimplePipeline->startAssembly();
    g_SimplePipeline->insertTopic(simp_mrt_topic);
    g_SimplePipeline->insertTopic(simp_ssao_topic);
    g_SimplePipeline->insertTopic(simp_lit_topic);
    g_SimplePipeline->insertTopic(simp_bill_topic);
    g_SimplePipeline->insertTopic(simp_sprite_topic);
    g_SimplePipeline->finishAssembly();

    if (!g_SimplePipeline->initAllTopics(g_Root->getDevices()))
    {
        return false;
    }

    name = g_SimplePipeline->getPipelineName();
    g_Root->getPipelinesManager()->addPipeline(name, g_SimplePipeline);
    if (g_RenderEffectConfig.mSimplyLitOn)
    {
        g_Root->getPipelinesManager()->setPipeline(name);
        g_Root->getPipelinesManager()->useNextPipeline();
    }

    g_ViewPort.Width = (float)g_Root->getDevices()->getCurrWndWidth();
    g_ViewPort.Height = (float)g_Root->getDevices()->getCurrWndHeight();
    g_ViewPort.MinDepth = 0.f;
    g_ViewPort.MaxDepth = 1.f;
    g_ViewPort.TopLeftX = 0.f;
    g_ViewPort.TopLeftY = 0.f;

    return true;
}

RSPass_MRT::RSPass_MRT(std::string& _name, PASS_TYPE _type,
    RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr), mAniVertexShader(nullptr),
    mPixelShader(nullptr), mNDPixelShader(nullptr),
    mDrawCallType(DRAWCALL_TYPE::OPACITY), mDrawCallPipe(nullptr),
    mViewProjStructedBuffer(nullptr),
    mViewProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
    mBonesStructedBuffer(nullptr),
    mBonesStructedBufferSrv(nullptr),
    mLinearSampler(nullptr),
    mGeoBufferRtv(nullptr),
    mAnisotropicRtv(nullptr),
    mDepthDsv(nullptr),
    mRSCameraInfo(nullptr)
{

}

RSPass_MRT::RSPass_MRT(const RSPass_MRT& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mAniVertexShader(_source.mAniVertexShader),
    mPixelShader(_source.mPixelShader),
    mNDPixelShader(_source.mNDPixelShader),
    mDrawCallType(_source.mDrawCallType),
    mDrawCallPipe(_source.mDrawCallPipe),
    mViewProjStructedBuffer(_source.mViewProjStructedBuffer),
    mViewProjStructedBufferSrv(_source.mViewProjStructedBufferSrv),
    mInstanceStructedBuffer(_source.mInstanceStructedBuffer),
    mInstanceStructedBufferSrv(_source.mInstanceStructedBufferSrv),
    mBonesStructedBuffer(_source.mBonesStructedBuffer),
    mBonesStructedBufferSrv(_source.mBonesStructedBufferSrv),
    mLinearSampler(_source.mLinearSampler),
    mGeoBufferRtv(_source.mGeoBufferRtv),
    mAnisotropicRtv(_source.mAnisotropicRtv),
    mDepthDsv(_source.mDepthDsv),
    mRSCameraInfo(_source.mRSCameraInfo)
{
    if (HasBeenInited)
    {
        RS_ADDREF(mVertexShader);
        RS_ADDREF(mAniVertexShader);
        RS_ADDREF(mPixelShader);
        RS_ADDREF(mNDPixelShader);
        RS_ADDREF(mViewProjStructedBuffer);
        RS_ADDREF(mViewProjStructedBufferSrv);
        RS_ADDREF(mInstanceStructedBuffer);
        RS_ADDREF(mInstanceStructedBufferSrv);
        RS_ADDREF(mBonesStructedBuffer);
        RS_ADDREF(mBonesStructedBufferSrv);
        RS_ADDREF(mLinearSampler);
    }
}

RSPass_MRT::~RSPass_MRT()
{

}

RSPass_MRT* RSPass_MRT::clonePass()
{
    return new RSPass_MRT(*this);
}

bool RSPass_MRT::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallType = DRAWCALL_TYPE::OPACITY;
    mDrawCallPipe = g_Root->getDrawCallsPool()->getDrawCallsPipe(mDrawCallType);

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->getCamerasContainer()->getRSCameraInfo(name);

    HasBeenInited = true;

    return true;
}

void RSPass_MRT::releasePass()
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
    g_Root->getResourceManager()->deleteResource(name);
    name = "mrt-geo-buffer";
    g_Root->getResourceManager()->deleteResource(name);
    name = "mrt-anisotropic";
    g_Root->getResourceManager()->deleteResource(name);
}

void RSPass_MRT::execuatePass()
{
    ID3D11RenderTargetView* rtvnull = nullptr;
    static ID3D11RenderTargetView* mrt[] = { mGeoBufferRtv, mAnisotropicRtv };
    context()->OMSetRenderTargets(2, mrt, mDepthDsv);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->ClearRenderTargetView(
        mGeoBufferRtv, DirectX::Colors::Transparent);
    context()->ClearRenderTargetView(
        mAnisotropicRtv, DirectX::Colors::Transparent);
    context()->ClearDepthStencilView(mDepthDsv, D3D11_CLEAR_DEPTH, 1.f, 0);
    //STContext()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);

    context()->PSSetSamplers(0, 1, &mLinearSampler);

    DirectX::XMMATRIX mat = {};
    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT aniStride = sizeof(vertex_type::AnimationVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    context()->Map(mViewProjStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    ViewProj* vp_data = (ViewProj*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mViewMat, mat);
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
    context()->Unmap(mViewProjStructedBuffer, 0);

    context()->VSSetShaderResources(0, 1, &mViewProjStructedBufferSrv);

    static std::string A_NAME = "AnimationVertex";
    static const auto ANIMAT_LAYOUT =
        getRSDX11RootInstance()->getStaticResources()->
        getStaticInputLayout(A_NAME);

    for (auto& call : mDrawCallPipe->Data)
    {
        if (call.MeshData.InputLayout == ANIMAT_LAYOUT)
        {
            context()->VSSetShader(mAniVertexShader, nullptr, 0);
            context()->IASetVertexBuffers(
                0, 1, &call.MeshData.VertexBuffer, &aniStride, &offset);

            context()->Map(mBonesStructedBuffer, 0,
                D3D11_MAP_WRITE_DISCARD, 0, &msr);
            DirectX::XMFLOAT4X4* b_data = (DirectX::XMFLOAT4X4*)msr.pData;
            void* boneData = call.InstanceData.BonesArrayPtr;
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
                            &((*bones)[i][j].BoneTransform));
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
            context()->Unmap(mBonesStructedBuffer, 0);

            context()->VSSetShaderResources(2, 1,
                &mBonesStructedBufferSrv);
        }
        else
        {
            context()->VSSetShader(mVertexShader, nullptr, 0);
            context()->IASetVertexBuffers(
                0, 1, &call.MeshData.VertexBuffer, &stride, &offset);
        }

        auto vecPtr = call.InstanceData.DataArrayPtr;
        auto size = vecPtr->size();
        context()->Map(mInstanceStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            mat = DirectX::XMLoadFloat4x4(&(*vecPtr)[i].WorldMatrix);
            mat = DirectX::XMMatrixTranspose(mat);
            DirectX::XMStoreFloat4x4(&ins_data[i].WorldMatrix, mat);
            ins_data[i].MaterialData = (*vecPtr)[i].MaterialData;
            ins_data[i].CustomizedData1 = (*vecPtr)[i].CustomizedData1;
            ins_data[i].CustomizedData2 = (*vecPtr)[i].CustomizedData2;
        }
        context()->Unmap(mInstanceStructedBuffer, 0);

        context()->IASetInputLayout(call.MeshData.InputLayout);
        context()->IASetPrimitiveTopology(call.MeshData.TopologyType);
        /*STContext()->IASetVertexBuffers(
            0, 1, &call.mMeshData.mVertexBuffer, &stride, &offset);*/
        context()->IASetIndexBuffer(
            call.MeshData.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context()->VSSetShaderResources(1, 1, &mInstanceStructedBufferSrv);
        ID3D11ShaderResourceView* matSrv = g_Root->getStaticResources()->getMaterialSrv();
        context()->PSSetShaderResources(0, 1, &matSrv);
        for (UINT i = 0; i < (UINT)MESH_TEXTURE_TYPE::SIZE; i++)
        {
            if (call.TextureData[i].EnabledFlag)
            {
                context()->PSSetShaderResources(
                    i + 1, 1, &(call.TextureData[i].Srv));
            }
        }

        context()->DrawIndexedInstanced(
            call.MeshData.IndexSize,
            (UINT)call.InstanceData.DataArrayPtr->size(), 0, 0, 0);

        if (call.MeshData.InputLayout == ANIMAT_LAYOUT)
        {
            ID3D11ShaderResourceView* nullSRV = nullptr;
            context()->VSSetShaderResources(2, 1,
                &nullSRV);
        }
    }

    context()->OMSetRenderTargets(1, &rtvnull, nullptr);
}

bool RSPass_MRT::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\mrt_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    D3D_SHADER_MACRO macro[] =
    { { "ANIMATION_VERTEX", "1" }, { nullptr, nullptr } };
    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\mrt_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob, macro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mAniVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\mrt_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\mrt_nd_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
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
    hr = device()->CreateBuffer(&bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE *
        (UINT)sizeof(DirectX::XMFLOAT4X4);
    bdc.StructureByteStride = sizeof(DirectX::XMFLOAT4X4);
    hr = device()->CreateBuffer(&bdc, nullptr, &mBonesStructedBuffer);

    bdc.ByteWidth = sizeof(ViewProj);
    bdc.StructureByteStride = sizeof(ViewProj);
    hr = device()->CreateBuffer(&bdc, nullptr, &mViewProjStructedBuffer);
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
    hr = device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &srvDesc, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE;
    hr = device()->CreateShaderResourceView(
        mBonesStructedBuffer,
        &srvDesc, &mBonesStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = 1;
    hr = device()->CreateShaderResourceView(
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
    texDesc.Width = getRSDX11RootInstance()->getDevices()->
        getCurrWndWidth();
    texDesc.Height = getRSDX11RootInstance()->getDevices()->
        getCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    hr = device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    hr = device()->CreateDepthStencilView(
        texture, &dsvDesc, &mDepthDsv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device()->CreateShaderResourceView(
        texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-depth";
    dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.Resource.Texture2D = texture;
    dti.Dsv = mDepthDsv;
    dti.Srv = srv;
    g_Root->getResourceManager()->addResource(name, dti);

    texDesc.Width = getRSDX11RootInstance()->getDevices()->
        getCurrWndWidth();
    texDesc.Height = getRSDX11RootInstance()->getDevices()->
        getCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = device()->CreateRenderTargetView(texture, &rtvDesc, &mGeoBufferRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device()->CreateShaderResourceView(texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-geo-buffer";
    dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.Resource.Texture2D = texture;
    dti.Rtv = mGeoBufferRtv;
    dti.Srv = srv;
    g_Root->getResourceManager()->addResource(name, dti);

    texDesc.Width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
    texDesc.Height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = device()->CreateRenderTargetView(texture, &rtvDesc, &mAnisotropicRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device()->CreateShaderResourceView(texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "mrt-anisotropic";
    dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.Resource.Texture2D = texture;
    dti.Rtv = mAnisotropicRtv;
    dti.Srv = srv;
    g_Root->getResourceManager()->addResource(name, dti);

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

    hr = device()->CreateSamplerState(&sampDesc, &mLinearSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_Ssao::RSPass_Ssao(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mPixelShader(nullptr),
    mCompressVertexShader(nullptr),
    mCompressPixelShader(nullptr),
    mRenderTargetView(nullptr),
    mNotCompressSrv(nullptr),
    mCompressRtv(nullptr),
    mSamplePointClamp(nullptr),
    mSampleLinearClamp(nullptr),
    mSampleDepthMap(nullptr),
    mSampleLinearWrap(nullptr),
    mSsaoInfoStructedBuffer(nullptr),
    mSsaoInfoStructedBufferSrv(nullptr),
    mGeoBufferSrv(nullptr),
    mDepthMapSrv(nullptr),
    mRandomMapSrv(nullptr),
    mOffsetVec({ {} }),
    mVertexBuffer(nullptr),
    mIndexBuffer(nullptr),
    mRSCameraInfo(nullptr)
{

}

RSPass_Ssao::RSPass_Ssao(const RSPass_Ssao& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mCompressVertexShader(_source.mCompressVertexShader),
    mCompressPixelShader(_source.mCompressPixelShader),
    mRenderTargetView(_source.mRenderTargetView),
    mNotCompressSrv(_source.mNotCompressSrv),
    mCompressRtv(_source.mCompressRtv),
    mSamplePointClamp(_source.mSamplePointClamp),
    mSampleLinearClamp(_source.mSampleLinearClamp),
    mSampleDepthMap(_source.mSampleDepthMap),
    mSampleLinearWrap(_source.mSampleLinearWrap),
    mSsaoInfoStructedBuffer(_source.mSsaoInfoStructedBuffer),
    mSsaoInfoStructedBufferSrv(_source.mSsaoInfoStructedBufferSrv),
    mGeoBufferSrv(_source.mGeoBufferSrv),
    mDepthMapSrv(_source.mDepthMapSrv),
    mRandomMapSrv(_source.mRandomMapSrv),
    mOffsetVec(_source.mOffsetVec),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer),
    mRSCameraInfo(_source.mRSCameraInfo)
{
    if (HasBeenInited)
    {
        RS_ADDREF(mVertexShader);
        RS_ADDREF(mPixelShader);
        RS_ADDREF(mCompressVertexShader);
        RS_ADDREF(mCompressPixelShader);
        RS_ADDREF(mSamplePointClamp);
        RS_ADDREF(mSampleLinearClamp);
        RS_ADDREF(mSampleDepthMap);
        RS_ADDREF(mSampleLinearWrap);
        RS_ADDREF(mSsaoInfoStructedBuffer);
        RS_ADDREF(mSsaoInfoStructedBufferSrv);
        RS_ADDREF(mVertexBuffer);
        RS_ADDREF(mIndexBuffer);
    }
}

RSPass_Ssao::~RSPass_Ssao()
{

}

RSPass_Ssao* RSPass_Ssao::clonePass()
{
    return new RSPass_Ssao(*this);
}

bool RSPass_Ssao::initPass()
{
    if (HasBeenInited) { return true; }

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
    mRSCameraInfo = g_Root->getCamerasContainer()->
        getRSCameraInfo(name);

    HasBeenInited = true;

    return true;
}

void RSPass_Ssao::releasePass()
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
    g_Root->getResourceManager()->deleteResource(name);
    name = "ssao-tex-ssao";
    g_Root->getResourceManager()->deleteResource(name);
    name = "ssao-tex-compress-ssao";
    g_Root->getResourceManager()->deleteResource(name);
}

void RSPass_Ssao::execuatePass()
{
    ID3D11RenderTargetView* null = nullptr;
    ID3D11ShaderResourceView* srvnull = nullptr;
    context()->OMSetRenderTargets(1, &mRenderTargetView, nullptr);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);
    context()->RSSetState(nullptr);

    DirectX::XMMATRIX mat = {};
    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};

    context()->Map(mSsaoInfoStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    SsaoInfo* ss_data = (SsaoInfo*)msr.pData;

    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&ss_data[0].mProj, mat);

    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&ss_data[0].mView, mat);

    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvProjMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&ss_data[0].mInvProj, mat);

    static DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);
    mat = DirectX::XMMatrixTranspose(
        DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix) * T);
    DirectX::XMStoreFloat4x4(&ss_data[0].mTexProj, mat);

    for (UINT i = 0; i < 14; i++)
    {
        ss_data[0].mOffsetVec[i] = mOffsetVec[i];
    }

    ss_data[0].mOcclusionRadius = g_RenderEffectConfig.mSsaoRadius;
    ss_data[0].mOcclusionFadeStart = g_RenderEffectConfig.mSsaoStart;
    ss_data[0].mOcclusionFadeEnd = g_RenderEffectConfig.mSsaoEnd;
    ss_data[0].mSurfaceEpsilon = g_RenderEffectConfig.mSsaoEpsilon;
    context()->Unmap(mSsaoInfoStructedBuffer, 0);

    context()->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context()->IASetVertexBuffers(
        0, 1, &mVertexBuffer,
        &stride, &offset);
    context()->IASetIndexBuffer(
        mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context()->VSSetShaderResources(
        0, 1, &mSsaoInfoStructedBufferSrv);
    context()->PSSetShaderResources(
        0, 1, &mSsaoInfoStructedBufferSrv);
    context()->PSSetShaderResources(
        1, 1, &mGeoBufferSrv);
    context()->PSSetShaderResources(
        2, 1, &mDepthMapSrv);
    context()->PSSetShaderResources(
        3, 1, &mRandomMapSrv);

    context()->PSSetSamplers(0, 1, &mSamplePointClamp);
    context()->PSSetSamplers(1, 1, &mSampleLinearClamp);
    context()->PSSetSamplers(2, 1, &mSampleDepthMap);
    context()->PSSetSamplers(3, 1, &mSampleLinearWrap);

    context()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    context()->OMSetRenderTargets(1, &mCompressRtv, nullptr);
    context()->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context()->IASetVertexBuffers(
        0, 1, &mVertexBuffer,
        &stride, &offset);
    context()->IASetIndexBuffer(
        mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->VSSetShader(mCompressVertexShader, nullptr, 0);
    context()->PSSetShader(mCompressPixelShader, nullptr, 0);
    context()->PSSetSamplers(0, 1, &mSampleLinearWrap);
    context()->PSSetShaderResources(
        0, 1, &mNotCompressSrv);

    context()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    context()->OMSetRenderTargets(1, &null, nullptr);
    context()->RSSetState(nullptr);
    context()->PSSetShaderResources(
        1, 1, &srvnull);
    context()->PSSetShaderResources(
        2, 1, &srvnull);
    context()->PSSetShaderResources(
        3, 1, &srvnull);
}

bool RSPass_Ssao::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\ssao_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\ssao_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\compress_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mCompressVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\compress_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
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
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mSsaoInfoStructedBuffer);
    if (FAILED(hr)) { return false; }

    vertex_type::TangentVertex v[4] = {};
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
    bdc.ByteWidth = sizeof(vertex_type::TangentVertex) * 4;
    bdc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bdc.CPUAccessFlags = 0;
    bdc.MiscFlags = 0;
    bdc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = device()->CreateBuffer(
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
    hr = device()->CreateBuffer(
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
    hr = device()->CreateTexture2D(
        &texDesc, &iniData, &texture);

    delete[] random;
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device()->CreateShaderResourceView(texture,
        &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "random-tex-ssao";
    dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.Resource.Texture2D = texture;
    dti.Srv = srv;
    g_Root->getResourceManager()->addResource(name, dti);

    texDesc.Width = getRSDX11RootInstance()->getDevices()->
        getCurrWndWidth();
    texDesc.Height = getRSDX11RootInstance()->getDevices()->
        getCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.BindFlags =
        D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = device()->CreateTexture2D(
        &texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = device()->CreateRenderTargetView(
        texture, &rtvDesc, &rtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device()->CreateShaderResourceView(
        texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "ssao-tex-ssao";
    dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.Resource.Texture2D = texture;
    dti.Rtv = rtv;
    dti.Srv = srv;
    g_Root->getResourceManager()->addResource(name, dti);

    texDesc.Width = getRSDX11RootInstance()->getDevices()->
        getCurrWndWidth() / 2;
    texDesc.Height = getRSDX11RootInstance()->getDevices()->
        getCurrWndHeight() / 2;
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
    hr = device()->CreateTexture2D(
        &texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = device()->CreateRenderTargetView(
        texture, &rtvDesc, &rtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device()->CreateShaderResourceView(
        texture, &srvDesc, &srv);
    if (FAILED(hr)) { return false; }

    uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;
    hr = device()->CreateUnorderedAccessView(
        texture, &uavDesc, &uav);
    if (FAILED(hr)) { return false; }

    dti = {};
    name = "ssao-tex-compress-ssao";
    dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.Resource.Texture2D = texture;
    dti.Rtv = rtv;
    dti.Srv = srv;
    dti.Uav = uav;
    g_Root->getResourceManager()->addResource(name, dti);

    return true;
}

bool RSPass_Ssao::CreateViews()
{
    std::string name = "random-tex-ssao";
    mRandomMapSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "mrt-geo-buffer";
    mGeoBufferSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "mrt-depth";
    mDepthMapSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "ssao-tex-ssao";
    mRenderTargetView = g_Root->getResourceManager()->
        getResource(name)->Rtv;
    mNotCompressSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "ssao-tex-compress-ssao";
    mCompressRtv = g_Root->getResourceManager()->
        getResource(name)->Rtv;

    D3D11_SHADER_RESOURCE_VIEW_DESC desSRV = {};
    HRESULT hr = S_OK;
    ZeroMemory(&desSRV, sizeof(desSRV));
    desSRV.Format = DXGI_FORMAT_UNKNOWN;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desSRV.Buffer.ElementWidth = 1;
    hr = device()->CreateShaderResourceView(
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
    hr = device()->CreateSamplerState(
        &samDesc, &mSamplePointClamp);
    if (FAILED(hr)) { return false; }

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device()->CreateSamplerState(
        &samDesc, &mSampleLinearClamp);
    if (FAILED(hr)) { return false; }

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    samDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device()->CreateSamplerState(
        &samDesc, &mSampleDepthMap);
    if (FAILED(hr)) { return false; }

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device()->CreateSamplerState(
        &samDesc, &mSampleLinearWrap);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_KBBlur::RSPass_KBBlur(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mHoriBlurShader(nullptr),
    mVertBlurShader(nullptr),
    mSsaoTexUav(nullptr),
    mGeoBufferSrv(nullptr),
    mDepthMapSrv(nullptr)
{

}

RSPass_KBBlur::RSPass_KBBlur(const RSPass_KBBlur& _source) :
    RSPass_Base(_source),
    mHoriBlurShader(_source.mHoriBlurShader),
    mVertBlurShader(_source.mVertBlurShader),
    mSsaoTexUav(_source.mSsaoTexUav),
    mGeoBufferSrv(_source.mGeoBufferSrv),
    mDepthMapSrv(_source.mDepthMapSrv)
{
    if (HasBeenInited)
    {
        RS_ADDREF(mHoriBlurShader);
        RS_ADDREF(mVertBlurShader);
    }
}

RSPass_KBBlur::~RSPass_KBBlur()
{

}

RSPass_KBBlur* RSPass_KBBlur::clonePass()
{
    return new RSPass_KBBlur(*this);
}

bool RSPass_KBBlur::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }

    HasBeenInited = true;

    return true;
}

void RSPass_KBBlur::releasePass()
{
    RS_RELEASE(mHoriBlurShader);
    RS_RELEASE(mVertBlurShader);
}

void RSPass_KBBlur::execuatePass()
{
    ID3D11ShaderResourceView* srv[] =
    {
        mGeoBufferSrv, mDepthMapSrv
    };
    static ID3D11UnorderedAccessView* nullUav = nullptr;
    static ID3D11ShaderResourceView* nullSrv[] =
    {
        nullptr, nullptr
    };

    static const UINT loopCount = g_RenderEffectConfig.mSsaoBlurCount;
    static UINT width = getRSDX11RootInstance()->getDevices()->
        getCurrWndWidth() / 2;
    static UINT height = getRSDX11RootInstance()->getDevices()->
        getCurrWndHeight() / 2;
    UINT dispatchVert = rs_tool::align(width, 256) / 256;
    UINT dispatchHori = rs_tool::align(height, 256) / 256;

    for (UINT i = 0; i < loopCount; i++)
    {
        context()->CSSetShader(mHoriBlurShader, nullptr, 0);
        context()->CSSetUnorderedAccessViews(0, 1,
            &mSsaoTexUav, nullptr);
        context()->CSSetShaderResources(0, 2, srv);
        context()->Dispatch(dispatchVert, height, 1);
        context()->CSSetUnorderedAccessViews(0, 1,
            &nullUav, nullptr);
        context()->CSSetShaderResources(0, 2, nullSrv);

        context()->CSSetShader(mVertBlurShader, nullptr, 0);
        context()->CSSetUnorderedAccessViews(0, 1,
            &mSsaoTexUav, nullptr);
        context()->CSSetShaderResources(0, 2, srv);
        context()->Dispatch(width, dispatchHori, 1);
        context()->CSSetUnorderedAccessViews(0, 1,
            &nullUav, nullptr);
        context()->CSSetShaderResources(0, 2, nullSrv);
    }
}

bool RSPass_KBBlur::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\ssao_compute.hlsl",
        "HMain", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mHoriBlurShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\ssao_compute.hlsl",
        "VMain", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
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
    std::string name = "mrt-geo-buffer";
    mGeoBufferSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "mrt-depth";
    mDepthMapSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "ssao-tex-compress-ssao";
    mSsaoTexUav = g_Root->getResourceManager()->
        getResource(name)->Uav;

    return true;
}

RSPass_Shadow::RSPass_Shadow(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mAniVertexShader(nullptr),
    mRasterizerState(nullptr),
    mDepthStencilView({ nullptr }),
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

RSPass_Shadow* RSPass_Shadow::clonePass()
{
    return new RSPass_Shadow(*this);
}

bool RSPass_Shadow::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateStates()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallType = DRAWCALL_TYPE::OPACITY;
    mDrawCallPipe = g_Root->getDrawCallsPool()->
        getDrawCallsPipe(mDrawCallType);

    HasBeenInited = true;

    return true;
}

void RSPass_Shadow::releasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mAniVertexShader);
    RS_RELEASE(mRasterizerState);
    RS_RELEASE(mViewProjStructedBufferSrv);
    RS_RELEASE(mViewProjStructedBuffer);
    RS_RELEASE(mInstanceStructedBufferSrv);
    RS_RELEASE(mInstanceStructedBuffer);

    std::string name = "light-depth-light-other";
    g_Root->getResourceManager()->deleteResource(name);
    name = "light-depth-light-dep0";
    g_Root->getResourceManager()->deleteResource(name);
    name = "light-depth-light-dep1";
    g_Root->getResourceManager()->deleteResource(name);
    name = "light-depth-light-dep2";
    g_Root->getResourceManager()->deleteResource(name);
    name = "light-depth-light-dep3";
    g_Root->getResourceManager()->deleteResource(name);
}

void RSPass_Shadow::execuatePass()
{
    ID3D11RenderTargetView* null = nullptr;
    //STContext()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(nullptr, nullptr, 0);
    context()->RSSetState(mRasterizerState);

    DirectX::XMMATRIX mat = {};
    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT aniStride = sizeof(vertex_type::AnimationVertex);
    UINT offset = 0;
    auto shadowLights = g_Root->getLightsContainer()->
        getShadowLightsArray();
    UINT shadowSize = (UINT)shadowLights->size();
    D3D11_MAPPED_SUBRESOURCE msr = {};

    static std::string A_NAME = "AnimationVertex";
    static const auto ANIMAT_LAYOUT =
        getRSDX11RootInstance()->getStaticResources()->
        getStaticInputLayout(A_NAME);

    for (UINT i = 0; i < shadowSize; i++)
    {
        context()->OMSetRenderTargets(1,
            &null, mDepthStencilView[i]);
        context()->RSSetViewports(1, &g_ViewPort);
        context()->ClearDepthStencilView(
            mDepthStencilView[i], D3D11_CLEAR_DEPTH, 1.f, 0);

        context()->Map(mViewProjStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        ViewProj* vp_data = (ViewProj*)msr.pData;
        auto light = (*shadowLights)[i];
        auto lcam = light->getRSLightCamera();
        mat = DirectX::XMLoadFloat4x4(
            &(lcam->getRSCameraInfo()->ViewMatrix));
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&vp_data[0].mViewMat, mat);
        mat = DirectX::XMLoadFloat4x4(
            &(lcam->getRSCameraInfo()->ProjMatrix));
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
        context()->Unmap(mViewProjStructedBuffer, 0);

        context()->VSSetShaderResources(
            0, 1, &mViewProjStructedBufferSrv);

        for (auto& call : mDrawCallPipe->Data)
        {
            if (call.MeshData.InputLayout == ANIMAT_LAYOUT)
            {
                context()->VSSetShader(mAniVertexShader, nullptr, 0);
                context()->IASetVertexBuffers(
                    0, 1, &call.MeshData.VertexBuffer, &aniStride, &offset);

                context()->Map(mBonesStructedBuffer, 0,
                    D3D11_MAP_WRITE_DISCARD, 0, &msr);
                DirectX::XMFLOAT4X4* b_data = (DirectX::XMFLOAT4X4*)msr.pData;
                void* boneData = call.InstanceData.BonesArrayPtr;
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
                                &((*bones)[i][j].BoneTransform));
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
                context()->Unmap(mBonesStructedBuffer, 0);

                context()->VSSetShaderResources(3, 1,
                    &mBonesStructedBufferSrv);
            }
            else
            {
                context()->VSSetShader(mVertexShader, nullptr, 0);
                context()->IASetVertexBuffers(
                    0, 1, &call.MeshData.VertexBuffer, &stride, &offset);
            }

            auto vecPtr = call.InstanceData.DataArrayPtr;
            auto size = vecPtr->size();
            context()->Map(mInstanceStructedBuffer, 0,
                D3D11_MAP_WRITE_DISCARD, 0, &msr);
            RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
            for (size_t i = 0; i < size; i++)
            {
                mat = DirectX::XMLoadFloat4x4(
                    &(*vecPtr)[i].WorldMatrix);
                mat = DirectX::XMMatrixTranspose(mat);
                DirectX::XMStoreFloat4x4(&ins_data[i].WorldMatrix, mat);
                ins_data[i].MaterialData =
                    (*vecPtr)[i].MaterialData;
                ins_data[i].CustomizedData1 =
                    (*vecPtr)[i].CustomizedData1;
                ins_data[i].CustomizedData2 =
                    (*vecPtr)[i].CustomizedData2;
            }
            context()->Unmap(mInstanceStructedBuffer, 0);

            context()->IASetInputLayout(
                call.MeshData.InputLayout);
            context()->IASetPrimitiveTopology(
                call.MeshData.TopologyType);
            /*STContext()->IASetVertexBuffers(
                0, 1, &call.mMeshData.mVertexBuffer,
                &stride, &offset);*/
            context()->IASetIndexBuffer(
                call.MeshData.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
            context()->VSSetShaderResources(
                1, 1, &mInstanceStructedBufferSrv);

            context()->DrawIndexedInstanced(
                call.MeshData.IndexSize,
                (UINT)call.InstanceData.DataArrayPtr->size(), 0, 0, 0);

            if (call.MeshData.InputLayout == ANIMAT_LAYOUT)
            {
                ID3D11ShaderResourceView* nullSRV = nullptr;
                context()->VSSetShaderResources(3, 1,
                    &nullSRV);
            }
        }
    }

    context()->OMSetRenderTargets(1, &null, nullptr);
    context()->RSSetState(nullptr);
}

bool RSPass_Shadow::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\light_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    D3D_SHADER_MACRO macro[] =
    { { "ANIMATION_VERTEX", "1" }, { nullptr, nullptr } };
    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\light_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob, macro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
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

    hr = device()->CreateRasterizerState(&shadowRasterDesc,
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
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE * (UINT)sizeof(DirectX::XMFLOAT4X4);
    bdc.StructureByteStride = sizeof(DirectX::XMFLOAT4X4);
    hr = device()->CreateBuffer(&bdc, nullptr, &mBonesStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = sizeof(ViewProj);
    bdc.StructureByteStride = sizeof(ViewProj);
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mViewProjStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Shadow::CreateViews()
{
    HRESULT hr = S_OK;
    ID3D11Texture2D* depthTex = nullptr;
    D3D11_TEXTURE2D_DESC texDepSte = {};
    texDepSte.Width = getRSDX11RootInstance()->getDevices()->
        getCurrWndWidth();
    texDepSte.Height = getRSDX11RootInstance()->getDevices()->
        getCurrWndHeight();
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
    hr = device()->CreateTexture2D(
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
        hr = device()->CreateDepthStencilView(
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
    hr = device()->CreateShaderResourceView(
        depthTex, &desSRV, &srv);
    if (FAILED(hr)) { return false; }

    RS_RESOURCE_INFO dti = {};
    std::string name = "light-depth-light-other";
    dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
    dti.Resource.Texture2D = depthTex;
    dti.Srv = srv;
    g_Root->getResourceManager()->addResource(name, dti);

    dti = {};
    name = "light-depth-light-dep0";
    dti.Dsv = mDepthStencilView[0];
    g_Root->getResourceManager()->addResource(name, dti);
    name = "light-depth-light-dep1";
    dti.Dsv = mDepthStencilView[1];
    g_Root->getResourceManager()->addResource(name, dti);
    name = "light-depth-light-dep2";
    dti.Dsv = mDepthStencilView[2];
    g_Root->getResourceManager()->addResource(name, dti);
    name = "light-depth-light-dep3";
    dti.Dsv = mDepthStencilView[3];
    g_Root->getResourceManager()->addResource(name, dti);

    ZeroMemory(&desSRV, sizeof(desSRV));
    desSRV.Format = DXGI_FORMAT_UNKNOWN;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desSRV.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
    hr = device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &desSRV, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    desSRV.Buffer.ElementWidth = MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE;
    hr = device()->CreateShaderResourceView(
        mBonesStructedBuffer,
        &desSRV, &mBonesStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    desSRV.Buffer.ElementWidth = 1;
    hr = device()->CreateShaderResourceView(
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
    mVertexShader(nullptr),
    mPixelShader(nullptr),
    mRenderTargetView(nullptr),
    mLinearWrapSampler(nullptr),
    mPointClampSampler(nullptr),
    mShadowTexSampler(nullptr),
    mLightInfoStructedBuffer(nullptr),
    mLightInfoStructedBufferSrv(nullptr),
    mLightStructedBuffer(nullptr),
    mLightStructedBufferSrv(nullptr),
    mAmbientStructedBuffer(nullptr),
    mAmbientStructedBufferSrv(nullptr),
    mShadowStructedBuffer(nullptr),
    mShadowStructedBufferSrv(nullptr),
    mCameraStructedBuffer(nullptr),
    mCameraStructedBufferSrv(nullptr),
    mGeoBufferSrv(nullptr),
    mAnisotropicSrv(nullptr),
    mSsaoSrv(nullptr),
    mShadowDepthSrv(nullptr),
    mVertexBuffer(nullptr),
    mIndexBuffer(nullptr),
    mRSCameraInfo(nullptr)
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
    mLightInfoStructedBuffer(_source.mLightInfoStructedBuffer),
    mLightInfoStructedBufferSrv(_source.mLightInfoStructedBufferSrv),
    mLightStructedBuffer(_source.mLightStructedBuffer),
    mLightStructedBufferSrv(_source.mLightStructedBufferSrv),
    mAmbientStructedBuffer(_source.mAmbientStructedBuffer),
    mAmbientStructedBufferSrv(_source.mAmbientStructedBufferSrv),
    mShadowStructedBuffer(_source.mShadowStructedBuffer),
    mShadowStructedBufferSrv(_source.mShadowStructedBufferSrv),
    mCameraStructedBuffer(_source.mCameraStructedBuffer),
    mCameraStructedBufferSrv(_source.mCameraStructedBufferSrv),
    mGeoBufferSrv(_source.mGeoBufferSrv),
    mAnisotropicSrv(_source.mAnisotropicSrv),
    mSsaoSrv(_source.mSsaoSrv),
    mShadowDepthSrv(_source.mShadowDepthSrv),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer),
    mRSCameraInfo(_source.mRSCameraInfo)
{

}

RSPass_Defered::~RSPass_Defered()
{

}

RSPass_Defered* RSPass_Defered::clonePass()
{
    return new RSPass_Defered(*this);
}

bool RSPass_Defered::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->getCamerasContainer()->
        getRSCameraInfo(name);

    HasBeenInited = true;

    return true;
}

void RSPass_Defered::releasePass()
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

void RSPass_Defered::execuatePass()
{
    context()->OMSetRenderTargets(1,
        &mRenderTargetView, nullptr);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->ClearRenderTargetView(
        mRenderTargetView, DirectX::Colors::DarkGreen);
    context()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);

    DirectX::XMMATRIX mat = {};
    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    context()->Map(mAmbientStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    Ambient* amb_data = (Ambient*)msr.pData;
    DirectX::XMFLOAT4 ambientL = getRSDX11RootInstance()->
        getLightsContainer()->getCurrentAmbientLight();
    amb_data[0].mAmbient = ambientL;
    context()->Unmap(mAmbientStructedBuffer, 0);

    static auto lights = g_Root->getLightsContainer()->getLightsArray();
    context()->Map(mLightInfoStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    LightInfo* li_data = (LightInfo*)msr.pData;
    li_data[0].mCameraPos = mRSCameraInfo->EyePosition;
    UINT dNum = 0;
    UINT sNum = 0;
    UINT pNum = 0;
    for (auto& l : *lights)
    {
        auto type = l->getRSLightType();
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
    li_data[0].mShadowLightNum = (UINT)g_Root->getLightsContainer()->
        getShadowLightsArray()->size();
    li_data[0].mShadowLightIndex[0] = -1;
    li_data[0].mShadowLightIndex[1] = -1;
    li_data[0].mShadowLightIndex[2] = -1;
    li_data[0].mShadowLightIndex[3] = -1;
    auto shadowIndeices = g_Root->getLightsContainer()->
        getShadowLightIndeicesArray();
    for (UINT i = 0; i < li_data[0].mShadowLightNum; i++)
    {
        li_data[0].mShadowLightIndex[i] = (*shadowIndeices)[i];
        if (i >= 3)
        {
            break;
        }
    }
    context()->Unmap(mLightInfoStructedBuffer, 0);

    context()->Map(mLightStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    RS_LIGHT_INFO* l_data = (RS_LIGHT_INFO*)msr.pData;
    UINT lightIndex = 0;
    for (auto& l : *lights)
    {
        l_data[lightIndex++] = *(l->getRSLightInfo());
    }
    context()->Unmap(mLightStructedBuffer, 0);

    context()->Map(mShadowStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    ShadowInfo* s_data = (ShadowInfo*)msr.pData;
    auto shadowLights = g_Root->getLightsContainer()->
        getShadowLightsArray();
    UINT shadowSize = (UINT)shadowLights->size();
    for (UINT i = 0; i < shadowSize; i++)
    {
        auto lcam = (*shadowLights)[i]->getRSLightCamera();
        mat = DirectX::XMLoadFloat4x4(
            &(lcam->getRSCameraInfo()->ViewMatrix));
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&s_data[i].mShadowViewMat, mat);
        mat = DirectX::XMLoadFloat4x4(
            &(lcam->getRSCameraInfo()->ProjMatrix));
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&s_data[i].mShadowProjMat, mat);
    }

    static DirectX::XMMATRIX T(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, -0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f);
    mat = DirectX::XMMatrixTranspose(
        DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix) *
        DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix) *
        T);
    DirectX::XMStoreFloat4x4(&s_data[0].mSSAOMat, mat);
    context()->Unmap(mShadowStructedBuffer, 0);

    context()->Map(mCameraStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    ViewProj* vp_data = (ViewProj*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvViewMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mViewMat, mat);
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvProjMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
    context()->Unmap(mCameraStructedBuffer, 0);

    static std::string depthSrvName = "mrt-depth";
    static auto depSrv = g_Root->getResourceManager()->
        getResource(depthSrvName)->Srv;
    ID3D11ShaderResourceView* srvs[] =
    {
        mAmbientStructedBufferSrv,
        mLightInfoStructedBufferSrv,
        mLightStructedBufferSrv,
        mShadowStructedBufferSrv,
        mCameraStructedBufferSrv,
        g_Root->getStaticResources()->getMaterialSrv(),
        mGeoBufferSrv, mAnisotropicSrv,
        mSsaoSrv, mShadowDepthSrv,
        g_IblBrdfSrv, g_DiffMapSrv, g_SpecMapSrv,
        depSrv
    };
    context()->PSSetShaderResources(0, 14, srvs);

    static ID3D11SamplerState* samps[] =
    {
        mPointClampSampler,
        mLinearWrapSampler,
        mShadowTexSampler
    };
    context()->PSSetSamplers(0, 3, samps);

    context()->IASetPrimitiveTopology(
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context()->IASetVertexBuffers(
        0, 1, &mVertexBuffer,
        &stride, &offset);
    context()->IASetIndexBuffer(
        mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    context()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    ID3D11RenderTargetView* rtvnull = nullptr;
    context()->OMSetRenderTargets(1, &rtvnull, nullptr);
    static ID3D11ShaderResourceView* nullsrvs[] =
    {
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr
    };
    context()->PSSetShaderResources(0, 14, nullsrvs);
}

bool RSPass_Defered::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\defered_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    std::string modelName = g_RenderEffectConfig.mLightModel;
    std::transform(modelName.begin(), modelName.end(),
        modelName.begin(), std::toupper);
    D3D_SHADER_MACRO macro[] =
    {
        { modelName.c_str(), "1" },
        { nullptr, nullptr }
    };
    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\defered_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob, macro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
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

    vertex_type::TangentVertex v[4] = {};
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
    bufDesc.ByteWidth = sizeof(vertex_type::TangentVertex) * 4;
    bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = 0;
    bufDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = device()->CreateBuffer(
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
    hr = device()->CreateBuffer(
        &bufDesc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bufDesc, sizeof(bufDesc));
    bufDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufDesc.ByteWidth = MAX_LIGHT_SIZE * sizeof(RS_LIGHT_INFO);
    bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufDesc.StructureByteStride = sizeof(RS_LIGHT_INFO);
    hr = device()->CreateBuffer(
        &bufDesc, nullptr, &mLightStructedBuffer);
    if (FAILED(hr)) { return false; }

    bufDesc.ByteWidth = sizeof(Ambient);
    bufDesc.StructureByteStride = sizeof(Ambient);
    hr = device()->CreateBuffer(
        &bufDesc, nullptr, &mAmbientStructedBuffer);
    if (FAILED(hr)) { return false; }

    bufDesc.ByteWidth = sizeof(LightInfo);
    bufDesc.StructureByteStride = sizeof(LightInfo);
    hr = device()->CreateBuffer(
        &bufDesc, nullptr, &mLightInfoStructedBuffer);
    if (FAILED(hr)) { return false; }

    bufDesc.ByteWidth = MAX_SHADOW_SIZE * sizeof(ShadowInfo);
    bufDesc.StructureByteStride = sizeof(ShadowInfo);
    hr = device()->CreateBuffer(
        &bufDesc, nullptr, &mShadowStructedBuffer);
    if (FAILED(hr)) { return false; }

    bufDesc.ByteWidth = sizeof(ViewProj);
    bufDesc.StructureByteStride = sizeof(ViewProj);
    hr = device()->CreateBuffer(
        &bufDesc, nullptr, &mCameraStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Defered::CreateViews()
{
    mRenderTargetView = g_Root->getDevices()->getHighDynamicRtv();

    std::string name = "mrt-geo-buffer";
    mGeoBufferSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "mrt-anisotropic";
    mAnisotropicSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "ssao-tex-compress-ssao";
    mSsaoSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;
    name = "light-depth-light-other";
    mShadowDepthSrv = g_Root->getResourceManager()->
        getResource(name)->Srv;

    HRESULT hr = S_OK;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_LIGHT_SIZE;
    hr = device()->CreateShaderResourceView(
        mLightStructedBuffer,
        &srvDesc, &mLightStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = 1;
    hr = device()->CreateShaderResourceView(
        mLightInfoStructedBuffer,
        &srvDesc, &mLightInfoStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateShaderResourceView(
        mAmbientStructedBuffer,
        &srvDesc, &mAmbientStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateShaderResourceView(
        mCameraStructedBuffer,
        &srvDesc, &mCameraStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = MAX_SHADOW_SIZE;
    hr = device()->CreateShaderResourceView(
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
    hr = device()->CreateSamplerState(
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
    hr = device()->CreateSamplerState(
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
    hr = device()->CreateSamplerState(
        &sampDesc, &mShadowTexSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_SkyShpere::RSPass_SkyShpere(std::string& _name,
    PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mPixelShader(nullptr),
    mRasterizerState(nullptr),
    mDepthStencilState(nullptr),
    mLinearWrapSampler(nullptr),
    mRenderTargerView(nullptr),
    mDepthStencilView(nullptr),
    mSkyShpereInfoStructedBuffer(nullptr),
    mSkyShpereInfoStructedBufferSrv(nullptr),
    mSkySphereMesh({}),
    mRSCameraInfo(nullptr)
{

}

RSPass_SkyShpere::RSPass_SkyShpere(
    const RSPass_SkyShpere& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mRasterizerState(_source.mRasterizerState),
    mDepthStencilState(_source.mDepthStencilState),
    mLinearWrapSampler(_source.mLinearWrapSampler),
    mRenderTargerView(_source.mRenderTargerView),
    mDepthStencilView(_source.mDepthStencilView),
    mSkyShpereInfoStructedBuffer(_source.mSkyShpereInfoStructedBuffer),
    mSkyShpereInfoStructedBufferSrv(_source.mSkyShpereInfoStructedBufferSrv),
    mSkySphereMesh(_source.mSkySphereMesh),
    mRSCameraInfo(_source.mRSCameraInfo)
{

}

RSPass_SkyShpere::~RSPass_SkyShpere()
{

}

RSPass_SkyShpere* RSPass_SkyShpere::clonePass()
{
    return new RSPass_SkyShpere(*this);
}

bool RSPass_SkyShpere::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateStates()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mSkySphereMesh = g_Root->getMeshHelper()->getGeoGenerator()->
        createGeometrySphere(10.f, 0,
            LAYOUT_TYPE::NORMAL_TANGENT_TEX, false,
            {},
            "this is not a bug about loading skybox texture failed :)");
    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        g_Root->getDevices()->getDevice(),
        L".\\RenderSystem_StaticResources\\Textures\\ibl_brdf.dds",
        nullptr, &g_IblBrdfSrv);
    if (FAILED(hr)) { return false; }

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->getCamerasContainer()->
        getRSCameraInfo(name);

    HasBeenInited = true;

    return true;
}

void RSPass_SkyShpere::releasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mRasterizerState);
    RS_RELEASE(mDepthStencilState);
    RS_RELEASE(mLinearWrapSampler);
    RS_RELEASE(mSkyShpereInfoStructedBuffer);
    //RS_RELEASE(mSkyShpereInfoStructedBufferSrv);

    g_Root->getMeshHelper()->releaseSubMesh(mSkySphereMesh);
}

void RSPass_SkyShpere::execuatePass()
{
    ID3D11RenderTargetView* null = nullptr;
    context()->OMSetRenderTargets(1,
        &mRenderTargerView, mDepthStencilView);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);
    context()->RSSetState(mRasterizerState);
    context()->OMSetDepthStencilState(mDepthStencilState, 0);

    DirectX::XMMATRIX mat = {};
    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};

    context()->Map(mSkyShpereInfoStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    SkyShpereInfo* sp_data = (SkyShpereInfo*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&sp_data[0].mViewMat, mat);

    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&sp_data[0].mProjMat, mat);

    sp_data[0].mEyePosition = mRSCameraInfo->EyePosition;

    mat = DirectX::XMMatrixScaling(1000.f, 1000.f, 1000.f);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&sp_data[0].mWorldMat, mat);
    context()->Unmap(mSkyShpereInfoStructedBuffer, 0);

    context()->IASetInputLayout(mSkySphereMesh.InputLayout);
    context()->IASetPrimitiveTopology(
        mSkySphereMesh.TopologyType);
    context()->IASetVertexBuffers(
        0, 1, &mSkySphereMesh.VertexBuffer,
        &stride, &offset);
    context()->IASetIndexBuffer(
        mSkySphereMesh.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context()->VSSetShaderResources(
        0, 1, &mSkyShpereInfoStructedBufferSrv);
    context()->PSSetShaderResources(
        0, 1, &g_EnviMapSrv);
    context()->PSSetSamplers(0, 1, &mLinearWrapSampler);

    context()->DrawIndexedInstanced(mSkySphereMesh.IndexSize,
        1, 0, 0, 0);

    context()->OMSetRenderTargets(1, &null, nullptr);
    context()->RSSetState(nullptr);
    context()->OMSetDepthStencilState(nullptr, 0);
}

bool RSPass_SkyShpere::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\skysphere_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\skysphere_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
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

    hr = device()->CreateRasterizerState(
        &rasDesc, &mRasterizerState);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateDepthStencilState(
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
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mSkyShpereInfoStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_SkyShpere::CreateViews()
{
    mRenderTargerView = g_Root->getDevices()->getHighDynamicRtv();
    std::string name = "mrt-depth";
    mDepthStencilView = g_Root->getResourceManager()->
        getResource(name)->Dsv;

    D3D11_SHADER_RESOURCE_VIEW_DESC desSRV = {};
    HRESULT hr = S_OK;
    ZeroMemory(&desSRV, sizeof(desSRV));
    desSRV.Format = DXGI_FORMAT_UNKNOWN;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desSRV.Buffer.ElementWidth = 1;
    hr = device()->CreateShaderResourceView(
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
    hr = device()->CreateSamplerState(
        &samDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_Bloom::RSPass_Bloom(std::string& _name, PASS_TYPE _type,
    RSRoot_DX11* _root) :RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mPixelShader(nullptr),
    mDrawCallType(DRAWCALL_TYPE::LIGHT),
    mDrawCallPipe(nullptr),
    mViewProjStructedBuffer(nullptr),
    mViewProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
    mRtv(nullptr),
    mDepthDsv(nullptr),
    mRSCameraInfo(nullptr),
    mVertexBuffer(nullptr),
    mIndexBuffer(nullptr),
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
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer),
    mSampler(_source.mSampler)
{

}

RSPass_Bloom::~RSPass_Bloom()
{

}

RSPass_Bloom* RSPass_Bloom::clonePass()
{
    return new RSPass_Bloom(*this);
}

bool RSPass_Bloom::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallPipe = g_Root->getDrawCallsPool()->
        getDrawCallsPipe(mDrawCallType);

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->getCamerasContainer()->
        getRSCameraInfo(name);
    if (!mRSCameraInfo) { return false; }

    HasBeenInited = true;

    return true;
}

void RSPass_Bloom::releasePass()
{
    std::string name = "bloom-light";
    g_Root->getResourceManager()->deleteResource(name);
    name = "bloom-compress-light";
    g_Root->getResourceManager()->deleteResource(name);

    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mViewProjStructedBufferSrv);
    RS_RELEASE(mInstanceStructedBufferSrv);
    RS_RELEASE(mViewProjStructedBuffer);
    RS_RELEASE(mVertexBuffer);
    RS_RELEASE(mIndexBuffer);
    RS_RELEASE(mSampler);
}

void RSPass_Bloom::execuatePass()
{
    context()->OMSetRenderTargets(1, &mRtv, mDepthDsv);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);

    DirectX::XMMATRIX mat = {};
    UINT stride = sizeof(vertex_type::ColorVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    context()->Map(mViewProjStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    ViewProj* vp_data = (ViewProj*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mViewMat, mat);
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
    context()->Unmap(mViewProjStructedBuffer, 0);

    context()->VSSetShaderResources(
        0, 1, &mViewProjStructedBufferSrv);

    for (auto& call : mDrawCallPipe->Data)
    {
        auto vecPtr = call.InstanceData.DataArrayPtr;
        auto size = vecPtr->size();
        context()->Map(mInstanceStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            mat = DirectX::XMLoadFloat4x4(
                &(*vecPtr)[i].WorldMatrix);
            mat = DirectX::XMMatrixTranspose(mat);
            DirectX::XMStoreFloat4x4(&ins_data[i].WorldMatrix, mat);
            ins_data[i].MaterialData =
                (*vecPtr)[i].MaterialData;
            ins_data[i].CustomizedData1 =
                (*vecPtr)[i].CustomizedData1;
            ins_data[i].CustomizedData2 =
                (*vecPtr)[i].CustomizedData2;
        }
        context()->Unmap(mInstanceStructedBuffer, 0);

        context()->IASetInputLayout(
            call.MeshData.InputLayout);
        context()->IASetPrimitiveTopology(
            call.MeshData.TopologyType);
        context()->IASetVertexBuffers(
            0, 1, &call.MeshData.VertexBuffer,
            &stride, &offset);
        context()->IASetIndexBuffer(
            call.MeshData.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context()->VSSetShaderResources(
            1, 1, &mInstanceStructedBufferSrv);

        context()->DrawIndexedInstanced(
            call.MeshData.IndexSize,
            (UINT)call.InstanceData.DataArrayPtr->size(), 0, 0, 0);
    }

    static ID3D11RenderTargetView* nullrtv[] = { nullptr };
    static ID3D11ShaderResourceView* nullsrv[] = { nullptr };
    context()->OMSetRenderTargets(1, nullrtv, nullptr);
    context()->VSSetShaderResources(0, 1, nullsrv);
}

bool RSPass_Bloom::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\bloom_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    const std::string pixelFactor = std::to_string(
        g_RenderEffectConfig.mBloomLightPixelFactor);
    if (g_RenderEffectConfig.mBloomOff)
    {
        D3D_SHADER_MACRO macro[] =
        {
            { "PIXEL_FACTOR", pixelFactor.c_str() },
            { nullptr, nullptr }
        };
        hr = rs_tool::compileShaderFromFile(
            L".\\Assets\\Shaders\\bloom_pixel.hlsl",
            "main", "ps_5_0", &shaderBlob, macro);
        if (FAILED(hr)) { return false; }
    }
    else
    {
        D3D_SHADER_MACRO macro[] =
        {
            { "BLOOM_ON", "1" },
            { "PIXEL_FACTOR", pixelFactor.c_str() },
            { nullptr, nullptr }
        };
        hr = rs_tool::compileShaderFromFile(
            L".\\Assets\\Shaders\\bloom_pixel.hlsl",
            "main", "ps_5_0", &shaderBlob, macro);
        if (FAILED(hr)) { return false; }
    }

    hr = device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
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
    hr = device()->CreateSamplerState(
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
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = sizeof(ViewProj);
    bdc.StructureByteStride = sizeof(ViewProj);
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mViewProjStructedBuffer);
    if (FAILED(hr)) { return false; }

    vertex_type::TangentVertex v[4] = {};
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
    bdc.ByteWidth = sizeof(vertex_type::TangentVertex) * 4;
    bdc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bdc.CPUAccessFlags = 0;
    bdc.MiscFlags = 0;
    bdc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = device()->CreateBuffer(
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
    hr = device()->CreateBuffer(
        &bdc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Bloom::CreateViews()
{
    HRESULT hr = S_OK;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    std::string name = "";

    mRtv = g_Root->getDevices()->getHighDynamicRtv();

    name = "mrt-depth";
    mDepthDsv = g_Root->getResourceManager()->
        getResource(name)->Dsv;
    if (!mDepthDsv) { return false; }

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
    hr = device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &srvDesc, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = 1;
    hr = device()->CreateShaderResourceView(
        mViewProjStructedBuffer,
        &srvDesc, &mViewProjStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_PriticleSetUp::RSPass_PriticleSetUp(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mTilingConstant({}),
    mParticleRenderBuffer(nullptr),
    mParticleRender_Srv(nullptr),
    mParticleRender_Uav(nullptr),
    mParticlePartA(nullptr),
    mPartA_Srv(nullptr),
    mPartA_Uav(nullptr),
    mParticlePartB(nullptr),
    mPartB_Uav(nullptr),
    mViewspacePosBuffer(nullptr),
    mViewSpacePos_Srv(nullptr),
    mViewSpacePos_Uav(nullptr),
    mMaxRadiusBuffer(nullptr),
    mMaxRadius_Srv(nullptr),
    mMaxRadius_Uav(nullptr),
    mStridedCoarseCullBuffer(nullptr),
    mStridedCoarseCull_Srv(nullptr),
    mStridedCoarseCull_Uav(nullptr),
    mStridedCoarseCullCounterBuffer(nullptr),
    mStridedCoarseCullCounter_Srv(nullptr),
    mStridedCoarseCullCounter_Uav(nullptr),
    mTiledIndexBuffer(nullptr),
    mTiledIndex_Srv(nullptr),
    mTiledIndex_Uav(nullptr),
    mDeadListBuffer(nullptr),
    mDeadList_Uav(nullptr),
    mAliveIndexBuffer(nullptr),
    mAliveIndex_Srv(nullptr),
    mAliveIndex_Uav(nullptr),
    mDeadListConstantBuffer(nullptr),
    mActiveListConstantBuffer(nullptr),
    mEmitterConstantBuffer(nullptr),
    mCameraConstantBuffer(nullptr),
    mTilingConstantBuffer(nullptr),
    mTimeConstantBuffer(nullptr),
    mDebugCounterBuffer(nullptr),
    mParticleRandomTexture(nullptr),
    mParticleRandom_Srv(nullptr),
    mSimulEmitterStructedBuffer(nullptr),
    mSimulEmitterStructedBuffer_Srv(nullptr)
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
    mTimeConstantBuffer(_source.mTimeConstantBuffer),
    mDebugCounterBuffer(_source.mDebugCounterBuffer),
    mParticleRandomTexture(_source.mParticleRandomTexture),
    mParticleRandom_Srv(_source.mParticleRandom_Srv),
    mSimulEmitterStructedBuffer(_source.mSimulEmitterStructedBuffer),
    mSimulEmitterStructedBuffer_Srv(_source.mSimulEmitterStructedBuffer_Srv)
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

RSPass_PriticleSetUp* RSPass_PriticleSetUp::clonePass()
{
    return new RSPass_PriticleSetUp(*this);
}

bool RSPass_PriticleSetUp::initPass()
{
    if (HasBeenInited) { return true; }

    int width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
    int height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();

    mTilingConstant.mNumTilesX =
        rs_tool::align(width, PTC_TILE_X_SIZE) / PTC_TILE_X_SIZE;
    mTilingConstant.mNumTilesY =
        rs_tool::align(height, PTC_TILE_Y_SIZE) / PTC_TILE_Y_SIZE;
    mTilingConstant.mNumCoarseCullingTilesX = PTC_MAX_COARSE_CULL_TILE_X;
    mTilingConstant.mNumCoarseCullingTilesY = PTC_MAX_COARSE_CULL_TILE_Y;
    mTilingConstant.mNumCullingTilesPerCoarseTileX =
        rs_tool::align(
            mTilingConstant.mNumTilesX,
            mTilingConstant.mNumCoarseCullingTilesX) /
        mTilingConstant.mNumCoarseCullingTilesX;
    mTilingConstant.mNumCullingTilesPerCoarseTileY =
        rs_tool::align(
            mTilingConstant.mNumTilesY,
            mTilingConstant.mNumCoarseCullingTilesY) /
        mTilingConstant.mNumCoarseCullingTilesY;

    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }

    auto resManager = getRSDX11RootInstance()->getResourceManager();
    RS_RESOURCE_INFO res;
    std::string name = "";

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mParticleRenderBuffer;
    res.Srv = mParticleRender_Srv;
    res.Uav = mParticleRender_Uav;
    name = PTC_RENDER_BUFFER_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mParticlePartA;
    res.Srv = mPartA_Srv;
    res.Uav = mPartA_Uav;
    name = PTC_A_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mParticlePartB;
    res.Uav = mPartB_Uav;
    name = PTC_B_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mViewspacePosBuffer;
    res.Srv = mViewSpacePos_Srv;
    res.Uav = mViewSpacePos_Uav;
    name = PTC_VIEW_SPCACE_POS_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mMaxRadiusBuffer;
    res.Srv = mMaxRadius_Srv;
    res.Uav = mMaxRadius_Uav;
    name = PTC_MAX_RADIUS_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mStridedCoarseCullBuffer;
    res.Srv = mStridedCoarseCull_Srv;
    res.Uav = mStridedCoarseCull_Uav;
    name = PTC_COARSE_CULL_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mStridedCoarseCullCounterBuffer;
    res.Srv = mStridedCoarseCullCounter_Srv;
    res.Uav = mStridedCoarseCullCounter_Uav;
    name = PTC_COARSE_CULL_COUNTER_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mTiledIndexBuffer;
    res.Srv = mTiledIndex_Srv;
    res.Uav = mTiledIndex_Uav;
    name = PTC_TILED_INDEX_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mDeadListBuffer;
    res.Uav = mDeadList_Uav;
    name = PTC_DEAD_LIST_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mAliveIndexBuffer;
    res.Srv = mAliveIndex_Srv;
    res.Uav = mAliveIndex_Uav;
    name = PTC_ALIVE_INDEX_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mDeadListConstantBuffer;
    name = PTC_DEAD_LIST_CONSTANT_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mActiveListConstantBuffer;
    name = PTC_ALIVE_LIST_CONSTANT_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mEmitterConstantBuffer;
    name = PTC_EMITTER_CONSTANT_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mCameraConstantBuffer;
    name = PTC_CAMERA_CONSTANT_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mTilingConstantBuffer;
    name = PTC_TILING_CONSTANT_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mDebugCounterBuffer;
    name = PTC_DEBUG_COUNTER_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::TEXTURE2D;
    res.Resource.Texture2D = mParticleRandomTexture;
    res.Srv = mParticleRandom_Srv;
    name = PTC_RAMDOM_TEXTURE_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mSimulEmitterStructedBuffer;
    res.Srv = mSimulEmitterStructedBuffer_Srv;
    name = PTC_SIMU_EMITTER_STRU_NAME;
    resManager->addResource(name, res);

    res = {};
    res.Type = RS_RESOURCE_TYPE::BUFFER;
    res.Resource.Buffer = mTimeConstantBuffer;
    name = PTC_TIME_CONSTANT_NAME;
    resManager->addResource(name, res);

    HasBeenInited = true;

    return true;
}

void RSPass_PriticleSetUp::releasePass()
{
    auto resManager = getRSDX11RootInstance()->getResourceManager();
    std::string name = PTC_RENDER_BUFFER_NAME;
    resManager->deleteResource(name);
    name = PTC_A_NAME;
    resManager->deleteResource(name);
    name = PTC_B_NAME;
    resManager->deleteResource(name);
    name = PTC_VIEW_SPCACE_POS_NAME;
    resManager->deleteResource(name);
    name = PTC_MAX_RADIUS_NAME;
    resManager->deleteResource(name);
    name = PTC_COARSE_CULL_NAME;
    resManager->deleteResource(name);
    name = PTC_COARSE_CULL_COUNTER_NAME;
    resManager->deleteResource(name);
    name = PTC_TILED_INDEX_NAME;
    resManager->deleteResource(name);
    name = PTC_DEAD_LIST_NAME;
    resManager->deleteResource(name);
    name = PTC_ALIVE_INDEX_NAME;
    resManager->deleteResource(name);
    name = PTC_DEAD_LIST_CONSTANT_NAME;
    resManager->deleteResource(name);
    name = PTC_ALIVE_LIST_CONSTANT_NAME;
    resManager->deleteResource(name);
    name = PTC_EMITTER_CONSTANT_NAME;
    resManager->deleteResource(name);
    name = PTC_CAMERA_CONSTANT_NAME;
    resManager->deleteResource(name);
    name = PTC_TILING_CONSTANT_NAME;
    resManager->deleteResource(name);
    name = PTC_DEBUG_COUNTER_NAME;
    resManager->deleteResource(name);
    name = PTC_RAMDOM_TEXTURE_NAME;
    resManager->deleteResource(name);
    name = PTC_SIMU_EMITTER_STRU_NAME;
    resManager->deleteResource(name);
    name = PTC_TIME_CONSTANT_NAME;
    resManager->deleteResource(name);
}

void RSPass_PriticleSetUp::execuatePass()
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
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mParticlePartA);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(RS_PARTICLE_PART_B) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(RS_PARTICLE_PART_B);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mParticlePartB);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(DirectX::XMFLOAT4) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(DirectX::XMFLOAT4);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mViewspacePosBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(float) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(float);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mMaxRadiusBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(UINT) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(UINT);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mDeadListBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth =
        sizeof(RS_ALIVE_INDEX_BUFFER_ELEMENT) * PTC_MAX_PARTICLE_SIZE;
    bfrDesc.StructureByteStride = sizeof(RS_ALIVE_INDEX_BUFFER_ELEMENT);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mAliveIndexBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.StructureByteStride = 0;
    bfrDesc.MiscFlags = 0;
    bfrDesc.ByteWidth =
        sizeof(UINT) * PTC_MAX_PARTICLE_SIZE * PTC_MAX_COARSE_CULL_TILE_SIZE;
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mStridedCoarseCullBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(UINT) * PTC_MAX_COARSE_CULL_TILE_SIZE;
    hr = device()->CreateBuffer(&bfrDesc, nullptr,
        &mStridedCoarseCullCounterBuffer);
    if (FAILED(hr)) { return false; }

    UINT numElements = mTilingConstant.mNumTilesX * mTilingConstant.mNumTilesY *
        PTC_TILE_BUFFER_SIZE;
    bfrDesc.ByteWidth = sizeof(UINT) * numElements;
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mTiledIndexBuffer);
    if (FAILED(hr)) { return false; }

    numElements =
        mTilingConstant.mNumTilesX * mTilingConstant.mNumTilesY *
        PTC_TILE_X_SIZE * PTC_TILE_Y_SIZE;
    bfrDesc.ByteWidth = 8 * numElements;    // DXGI_FORMAT_R16G16B16A16_FLOAT
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mParticleRenderBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_DEFAULT;
    bfrDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bfrDesc.CPUAccessFlags = 0;
    bfrDesc.ByteWidth = 4 * sizeof(UINT);   // one for record and three for pad
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mDeadListConstantBuffer);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mActiveListConstantBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_DYNAMIC;
    bfrDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bfrDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bfrDesc.ByteWidth = sizeof(RS_PARTICLE_EMITTER_INFO);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mEmitterConstantBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(CAMERA_STATUS);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mCameraConstantBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(RS_TILING_CONSTANT);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mTilingConstantBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(PTC_TIME_CONSTANT);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mTimeConstantBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_STAGING;
    bfrDesc.BindFlags = 0;
    bfrDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    bfrDesc.ByteWidth = sizeof(UINT);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mDebugCounterBuffer);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_DYNAMIC;
    bfrDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bfrDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bfrDesc.ByteWidth = MAX_PARTICLE_EMITTER_SIZE *
        sizeof(SIMULATE_EMITTER_INFO);
    bfrDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bfrDesc.StructureByteStride = sizeof(SIMULATE_EMITTER_INFO);
    hr = device()->CreateBuffer(
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
        ptr[0] = rs_tool::randomVariance(0.0f, 1.0f);
        ptr[1] = rs_tool::randomVariance(0.0f, 1.0f);
        ptr[2] = rs_tool::randomVariance(0.0f, 1.0f);
        ptr[3] = rs_tool::randomVariance(0.0f, 1.0f);
        ptr += 4;
    }

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = values;
    data.SysMemPitch = texDesc.Width * 16;
    data.SysMemSlicePitch = 0;

    hr = device()->CreateTexture2D(&texDesc, &data, &mParticleRandomTexture);
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
    hr = device()->CreateShaderResourceView(
        mParticlePartA, &srvDesc, &mPartA_Srv);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
        mParticlePartA, &uavDesc, &mPartA_Uav);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
        mParticlePartB, &uavDesc, &mPartB_Uav);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateShaderResourceView(
        mViewspacePosBuffer, &srvDesc, &mViewSpacePos_Srv);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
        mViewspacePosBuffer, &uavDesc, &mViewSpacePos_Uav);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateShaderResourceView(
        mMaxRadiusBuffer, &srvDesc, &mMaxRadius_Srv);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
        mMaxRadiusBuffer, &uavDesc, &mMaxRadius_Uav);
    if (FAILED(hr)) { return false; }

    uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_APPEND;
    hr = device()->CreateUnorderedAccessView(
        mDeadListBuffer, &uavDesc, &mDeadList_Uav);
    if (FAILED(hr)) { return false; }

    uavDesc.Format = DXGI_FORMAT_R32_UINT;
    uavDesc.Buffer.Flags = 0;
    uavDesc.Buffer.NumElements =
        PTC_MAX_PARTICLE_SIZE * PTC_MAX_COARSE_CULL_TILE_SIZE;
    srvDesc.Format = DXGI_FORMAT_R32_UINT;
    srvDesc.Buffer.NumElements =
        PTC_MAX_PARTICLE_SIZE * PTC_MAX_COARSE_CULL_TILE_SIZE;
    hr = device()->CreateShaderResourceView(
        mStridedCoarseCullBuffer, &srvDesc, &mStridedCoarseCull_Srv);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
        mStridedCoarseCullBuffer, &uavDesc, &mStridedCoarseCull_Uav);
    if (FAILED(hr)) { return false; }

    uavDesc.Buffer.NumElements = PTC_MAX_COARSE_CULL_TILE_SIZE;
    srvDesc.Buffer.NumElements = PTC_MAX_COARSE_CULL_TILE_SIZE;
    hr = device()->CreateShaderResourceView(
        mStridedCoarseCullCounterBuffer, &srvDesc,
        &mStridedCoarseCullCounter_Srv);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
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
    hr = device()->CreateShaderResourceView(
        mAliveIndexBuffer, &srvDesc, &mAliveIndex_Srv);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
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
    hr = device()->CreateShaderResourceView(
        mTiledIndexBuffer, &srvDesc, &mTiledIndex_Srv);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
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
    hr = device()->CreateShaderResourceView(
        mParticleRenderBuffer, &srvDesc, &mParticleRender_Srv);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(
        mParticleRenderBuffer, &uavDesc, &mParticleRender_Uav);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    hr = device()->CreateShaderResourceView(
        mParticleRandomTexture, &srvDesc, &mParticleRandom_Srv);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_PARTICLE_EMITTER_SIZE;
    hr = device()->CreateShaderResourceView(
        mSimulEmitterStructedBuffer,
        &srvDesc, &mSimulEmitterStructedBuffer_Srv);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_PriticleEmitSimulate::RSPass_PriticleEmitSimulate(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mRSParticleContainerPtr(nullptr),
    mInitDeadListShader(nullptr),
    mResetParticlesShader(nullptr),
    mEmitParticleShader(nullptr),
    mSimulateShader(nullptr),
    mDepthTex_Srv(nullptr),
    mRandomTex_Srv(nullptr),
    mSimulEmitterStructedBuffer_Srv(nullptr),
    mDeadList_Uav(nullptr),
    mPartA_Uav(nullptr),
    mPartB_Uav(nullptr),
    mAliveIndex_Uav(nullptr),
    mViewSpacePos_Uav(nullptr),
    mMaxRadius_Uav(nullptr),
    mEmitterConstantBuffer(nullptr),
    mCameraConstantBuffer(nullptr),
    mDeadListConstantBuffer(nullptr),
    mSimulEmitterStructedBuffer(nullptr),
    mTimeConstantBuffer(nullptr),
    mLinearWrapSampler(nullptr),
    mRSCameraInfo(nullptr)
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
    mDepthTex_Srv(_source.mDepthTex_Srv),
    mRandomTex_Srv(_source.mRandomTex_Srv),
    mSimulEmitterStructedBuffer_Srv(_source.mSimulEmitterStructedBuffer_Srv),
    mDeadList_Uav(_source.mDeadList_Uav),
    mPartA_Uav(_source.mPartA_Uav),
    mPartB_Uav(_source.mPartB_Uav),
    mAliveIndex_Uav(_source.mAliveIndex_Uav),
    mViewSpacePos_Uav(_source.mViewSpacePos_Uav),
    mMaxRadius_Uav(_source.mMaxRadius_Uav),
    mEmitterConstantBuffer(_source.mEmitterConstantBuffer),
    mCameraConstantBuffer(_source.mCameraConstantBuffer),
    mDeadListConstantBuffer(_source.mDeadListConstantBuffer),
    mSimulEmitterStructedBuffer(_source.mSimulEmitterStructedBuffer),
    mTimeConstantBuffer(_source.mTimeConstantBuffer),
    mLinearWrapSampler(_source.mLinearWrapSampler),
    mRSCameraInfo(_source.mRSCameraInfo)
{

}

RSPass_PriticleEmitSimulate::~RSPass_PriticleEmitSimulate()
{

}

RSPass_PriticleEmitSimulate* RSPass_PriticleEmitSimulate::clonePass()
{
    return new RSPass_PriticleEmitSimulate(*this);
}

bool RSPass_PriticleEmitSimulate::initPass()
{
    if (HasBeenInited) { return true; }

    mRSParticleContainerPtr = getRSDX11RootInstance()->getParticlesContainer();
    if (!mRSParticleContainerPtr) { return false; }

    std::string name = "temp-cam";
    mRSCameraInfo = getRSDX11RootInstance()->getCamerasContainer()->
        getRSCameraInfo(name);
    if (!mRSCameraInfo) { return false; }

    if (!CreateShaders()) { return false; }
    if (!CreateSampler()) { return false; }
    if (!CheckResources()) { return false; }

    mRSParticleContainerPtr->resetRSParticleSystem();

    HasBeenInited = true;

    return true;
}

void RSPass_PriticleEmitSimulate::releasePass()
{
    RS_RELEASE(mSimulateShader);
    RS_RELEASE(mEmitParticleShader);
    RS_RELEASE(mResetParticlesShader);
    RS_RELEASE(mInitDeadListShader);
}

void RSPass_PriticleEmitSimulate::execuatePass()
{
    if (!mRSParticleContainerPtr->getAllParticleEmitters()->size())
    {
        return;
    }

    if (mRSParticleContainerPtr->getResetFlg())
    {
        {
            context()->CSSetShader(mInitDeadListShader,
                nullptr, 0);
            ID3D11UnorderedAccessView* uav[] = { mDeadList_Uav };
            UINT initialCount[] = { 0 };
            context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
                uav, initialCount);

            context()->Dispatch(
                rs_tool::align(PTC_MAX_PARTICLE_SIZE, 256) / 256, 1, 1);

            ZeroMemory(uav, sizeof(uav));
            context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
                uav, nullptr);
        }

        {
            context()->CSSetShader(mResetParticlesShader,
                nullptr, 0);
            ID3D11UnorderedAccessView* uav[] = { mPartA_Uav,mPartB_Uav };
            UINT initialCount[] = { (UINT)-1,(UINT)-1 };
            context()->CSSetUnorderedAccessViews(0,
                ARRAYSIZE(uav), uav, initialCount);

            context()->Dispatch(
                rs_tool::align(PTC_MAX_PARTICLE_SIZE, 256) / 256, 1, 1);

            ZeroMemory(uav, sizeof(uav));
            context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
                uav, nullptr);
        }

        mRSParticleContainerPtr->finishResetRSParticleSystem();
    }

    {
        context()->CSSetShader(mEmitParticleShader, nullptr, 0);
        ID3D11UnorderedAccessView* uav[] =
        { mPartA_Uav,mPartB_Uav,mDeadList_Uav };
        ID3D11ShaderResourceView* srv[] = { mRandomTex_Srv };
        ID3D11Buffer* cbuffer[] =
        { mEmitterConstantBuffer,mDeadListConstantBuffer,mTimeConstantBuffer };
        ID3D11SamplerState* sam[] = { mLinearWrapSampler };
        UINT initialCount[] = { (UINT)-1,(UINT)-1,(UINT)-1 };
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
            uav, initialCount);
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->CSSetConstantBuffers(0, ARRAYSIZE(cbuffer), cbuffer);
        context()->CSSetSamplers(0, ARRAYSIZE(sam), sam);

        auto emitters = mRSParticleContainerPtr->
            getAllParticleEmitters();
        D3D11_MAPPED_SUBRESOURCE msr = {};
        context()->Map(mTimeConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        PTC_TIME_CONSTANT* time = (PTC_TIME_CONSTANT*)msr.pData;
        static float timer = 0.f;
        time->mDeltaTime = g_DeltaTimeInSecond;
        timer += g_DeltaTimeInSecond;
        time->mTotalTime = timer;
        context()->Unmap(mTimeConstantBuffer, 0);
        for (auto& emitter : *emitters)
        {
            auto& rsinfo = emitter->getRSParticleEmitterInfo();
            rsinfo.Accumulation += rsinfo.EmitNumPerSecond *
                g_DeltaTimeInSecond;
            if (rsinfo.Accumulation > 1.f)
            {
                float integerPart = 0.0f;
                float fraction = modf(rsinfo.Accumulation,
                    &integerPart);
                rsinfo.EmitSize = (int)integerPart;
                rsinfo.Accumulation = fraction;
            }

            context()->Map(mEmitterConstantBuffer, 0,
                D3D11_MAP_WRITE_DISCARD, 0, &msr);
            RS_PARTICLE_EMITTER_INFO* emitterCon =
                (RS_PARTICLE_EMITTER_INFO*)msr.pData;
            emitterCon->EmitterIndex = rsinfo.EmitterIndex;
            emitterCon->EmitNumPerSecond = rsinfo.EmitNumPerSecond;
            emitterCon->EmitSize = rsinfo.EmitSize;
            emitterCon->Accumulation = rsinfo.Accumulation;
            emitterCon->Position = rsinfo.Position;
            emitterCon->Velocity = rsinfo.Velocity;
            emitterCon->PosVariance = rsinfo.PosVariance;
            emitterCon->VelVariance = rsinfo.VelVariance;
            emitterCon->Acceleration = rsinfo.Acceleration;
            emitterCon->ParticleMass = rsinfo.ParticleMass;
            emitterCon->LifeSpan = rsinfo.LifeSpan;
            emitterCon->StartSize = rsinfo.StartSize;
            emitterCon->EndSize = rsinfo.EndSize;
            emitterCon->StartColor = rsinfo.StartColor;
            emitterCon->EndColor = rsinfo.EndColor;
            emitterCon->TextureID = rsinfo.TextureID;
            emitterCon->StreakFlag = rsinfo.StreakFlag;
            emitterCon->MiscFlag = rsinfo.MiscFlag;
            context()->Unmap(mEmitterConstantBuffer, 0);
            context()->CopyStructureCount(mDeadListConstantBuffer,
                0, mDeadList_Uav);

            int threadGroupNum = rs_tool::align(
                rsinfo.EmitSize, 1024) / 1024;
            context()->Dispatch(threadGroupNum, 1, 1);
        }

        ZeroMemory(uav, sizeof(uav));
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
        ZeroMemory(srv, sizeof(srv));
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
    }

    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        DirectX::XMMATRIX mat = {};

        context()->Map(mCameraConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        CAMERA_STATUS* camStatus =
            (CAMERA_STATUS*)msr.pData;
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvViewMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mViewProj), mat);
        camStatus->mEyePosition = mRSCameraInfo->EyePosition;
        context()->Unmap(mCameraConstantBuffer, 0);

        static auto emitterVec = mRSParticleContainerPtr->
            getAllParticleEmitters();
        auto size = emitterVec->size();
        context()->Map(mSimulEmitterStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        SIMULATE_EMITTER_INFO* emitter =
            (SIMULATE_EMITTER_INFO*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            emitter[i].mWorldPosition = (*(*emitterVec)[i]).
                getRSParticleEmitterInfo().Position;
        }
        context()->Unmap(mSimulEmitterStructedBuffer, 0);

        ID3D11Buffer* cb[] = { mCameraConstantBuffer,mTimeConstantBuffer };
        ID3D11ShaderResourceView* srv[] =
        { mDepthTex_Srv,mSimulEmitterStructedBuffer_Srv };
        ID3D11UnorderedAccessView* uav[] =
        { mPartA_Uav,mPartB_Uav,mDeadList_Uav,mAliveIndex_Uav,
        mViewSpacePos_Uav,mMaxRadius_Uav };
        UINT initialCount[] =
        { (UINT)-1,(UINT)-1,(UINT)-1,0,(UINT)-1,(UINT)-1 };

        context()->CSSetShader(mSimulateShader, nullptr, 0);
        context()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav),
            uav, initialCount);
        static int threadGroupNum = rs_tool::align(
            PTC_MAX_PARTICLE_SIZE, 256) / 256;
        context()->Dispatch(threadGroupNum, 1, 1);

        ZeroMemory(uav, sizeof(uav));
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
        ZeroMemory(srv, sizeof(srv));
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
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

    hr = device()->CreateSamplerState(&sampDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleEmitSimulate::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_init_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mInitDeadListShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_reset_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mResetParticlesShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_emit_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mEmitParticleShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_simulate_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mSimulateShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleEmitSimulate::CheckResources()
{
    auto resManager = getRSDX11RootInstance()->getResourceManager();
    if (!resManager) { return false; }

    std::string name = PTC_DEAD_LIST_NAME;
    mDeadList_Uav = resManager->getResource(name)->Uav;
    if (!mDeadList_Uav) { return false; }

    name = PTC_A_NAME;
    mPartA_Uav = resManager->getResource(name)->Uav;
    if (!mPartA_Uav) { return false; }

    name = PTC_B_NAME;
    mPartB_Uav = resManager->getResource(name)->Uav;
    if (!mPartB_Uav) { return false; }

    name = PTC_RAMDOM_TEXTURE_NAME;
    mRandomTex_Srv = resManager->getResource(name)->Srv;
    if (!mRandomTex_Srv) { return false; }

    name = PTC_EMITTER_CONSTANT_NAME;
    mEmitterConstantBuffer = resManager->getResource(name)->
        Resource.Buffer;
    if (!mEmitterConstantBuffer) { return false; }

    name = PTC_DEAD_LIST_CONSTANT_NAME;
    mDeadListConstantBuffer = resManager->getResource(name)->
        Resource.Buffer;
    if (!mDeadListConstantBuffer) { return false; }

    name = PTC_CAMERA_CONSTANT_NAME;
    mCameraConstantBuffer = resManager->getResource(name)->
        Resource.Buffer;
    if (!mCameraConstantBuffer) { return false; }

    name = PTC_SIMU_EMITTER_STRU_NAME;
    mSimulEmitterStructedBuffer_Srv = resManager->getResource(name)->Srv;
    mSimulEmitterStructedBuffer = resManager->getResource(name)->
        Resource.Buffer;
    if (!mSimulEmitterStructedBuffer_Srv || !mSimulEmitterStructedBuffer) { return false; }

    name = PTC_ALIVE_INDEX_NAME;
    mAliveIndex_Uav = resManager->getResource(name)->Uav;
    if (!mAliveIndex_Uav) { return false; }

    name = PTC_VIEW_SPCACE_POS_NAME;
    mViewSpacePos_Uav = resManager->getResource(name)->Uav;
    if (!mViewSpacePos_Uav) { return false; }

    name = PTC_MAX_RADIUS_NAME;
    mMaxRadius_Uav = resManager->getResource(name)->Uav;
    if (!mMaxRadius_Uav) { return false; }

    name = PTC_TIME_CONSTANT_NAME;
    mTimeConstantBuffer = resManager->getResource(name)->
        Resource.Buffer;
    if (!mTimeConstantBuffer) { return false; }

    name = "mrt-depth";
    mDepthTex_Srv = resManager->getResource(name)->Srv;
    if (!mDepthTex_Srv) { return false; }

    return true;
}

RSPass_PriticleTileRender::RSPass_PriticleTileRender(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mCoarseCullingShader(nullptr),
    mTileCullingShader(nullptr),
    mTileRenderShader(nullptr),
    mBlendVertexShader(nullptr),
    mBlendPixelShader(nullptr),
    mCameraConstantBuffer(nullptr),
    mTilingConstantBuffer(nullptr),
    mActiveListConstantBuffer(nullptr),
    mDepthTex_Srv(nullptr),
    mViewSpacePos_Srv(nullptr),
    mMaxRadius_Srv(nullptr),
    mPartA_Srv(nullptr),
    mAliveIndex_Srv(nullptr),
    mAliveIndex_Uav(nullptr),
    mCoarseTileIndex_Srv(nullptr),
    mCoarseTileIndex_Uav(nullptr),
    mCoarseTileIndexCounter_Srv(nullptr),
    mCoarseTileIndexCounter_Uav(nullptr),
    mTiledIndex_Srv(nullptr),
    mTiledIndex_Uav(nullptr),
    mParticleRender_Srv(nullptr),
    mParticleRender_Uav(nullptr),
    mLinearClampSampler(nullptr),
    mParticleBlendState(nullptr),
    mParticleTex_Srv(nullptr),
    mRSCameraInfo(nullptr)
{

}

RSPass_PriticleTileRender::RSPass_PriticleTileRender(
    const RSPass_PriticleTileRender& _source) :
    RSPass_Base(_source),
    mCoarseCullingShader(_source.mCoarseCullingShader),
    mTileCullingShader(_source.mTileCullingShader),
    mTileRenderShader(_source.mTileRenderShader),
    mBlendVertexShader(_source.mBlendVertexShader),
    mBlendPixelShader(_source.mBlendPixelShader),
    mCameraConstantBuffer(_source.mCameraConstantBuffer),
    mTilingConstantBuffer(_source.mTilingConstantBuffer),
    mActiveListConstantBuffer(_source.mActiveListConstantBuffer),
    mDepthTex_Srv(_source.mDepthTex_Srv),
    mViewSpacePos_Srv(_source.mViewSpacePos_Srv),
    mMaxRadius_Srv(_source.mMaxRadius_Srv),
    mPartA_Srv(_source.mPartA_Srv),
    mAliveIndex_Srv(_source.mAliveIndex_Srv),
    mAliveIndex_Uav(_source.mAliveIndex_Uav),
    mCoarseTileIndex_Srv(_source.mCoarseTileIndex_Srv),
    mCoarseTileIndex_Uav(_source.mCoarseTileIndex_Uav),
    mCoarseTileIndexCounter_Srv(_source.mCoarseTileIndexCounter_Srv),
    mCoarseTileIndexCounter_Uav(_source.mCoarseTileIndexCounter_Uav),
    mTiledIndex_Srv(_source.mTiledIndex_Srv),
    mTiledIndex_Uav(_source.mTiledIndex_Uav),
    mParticleRender_Srv(_source.mParticleRender_Srv),
    mParticleRender_Uav(_source.mParticleRender_Uav),
    mLinearClampSampler(_source.mLinearClampSampler),
    mParticleBlendState(_source.mParticleBlendState),
    mParticleTex_Srv(_source.mParticleTex_Srv),
    mRSCameraInfo(_source.mRSCameraInfo)
{

}

RSPass_PriticleTileRender::~RSPass_PriticleTileRender()
{

}

RSPass_PriticleTileRender* RSPass_PriticleTileRender::clonePass()
{
    return new RSPass_PriticleTileRender(*this);
}

bool RSPass_PriticleTileRender::initPass()
{
    if (HasBeenInited) { return true; }

    std::string name = "temp-cam";
    mRSCameraInfo = getRSDX11RootInstance()->getCamerasContainer()->
        getRSCameraInfo(name);
    if (!mRSCameraInfo) { return false; }

    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSampler()) { return false; }
    if (!CreateBlend()) { return false; }
    if (!CheckResources()) { return false; }

    HasBeenInited = true;

    return true;
}

void RSPass_PriticleTileRender::releasePass()
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

void RSPass_PriticleTileRender::execuatePass()
{
    if (!g_Root->getParticlesContainer()->getAllParticleEmitters()->size())
    {
        return;
    }

    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        DirectX::XMMATRIX mat = {};
        context()->Map(mCameraConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        CAMERA_STATUS* camStatus = (CAMERA_STATUS*)msr.pData;
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvViewMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mViewProj), mat);
        camStatus->mEyePosition = mRSCameraInfo->EyePosition;
        context()->Unmap(mCameraConstantBuffer, 0);
        context()->Map(mTilingConstantBuffer, 0,
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
        context()->Unmap(mTilingConstantBuffer, 0);
        context()->CopyStructureCount(mActiveListConstantBuffer, 0,
            mAliveIndex_Uav);

        ID3D11Buffer* cb[] =
        { mCameraConstantBuffer,mTilingConstantBuffer,mActiveListConstantBuffer };
        ID3D11ShaderResourceView* srv[] =
        { mViewSpacePos_Srv,mMaxRadius_Srv,mAliveIndex_Srv };
        ID3D11UnorderedAccessView* uav[] =
        { mCoarseTileIndex_Uav,mCoarseTileIndexCounter_Uav };
        UINT initial[] = { (UINT)-1,(UINT)-1 };

        context()->CSSetShader(mCoarseCullingShader, nullptr, 0);
        context()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, initial);

        static int threadGroupNum = rs_tool::align(PTC_MAX_PARTICLE_SIZE,
            PTC_COARSE_CULLING_THREADS) / PTC_COARSE_CULLING_THREADS;
        context()->Dispatch(threadGroupNum, 1, 1);

        ZeroMemory(cb, sizeof(cb));
        ZeroMemory(srv, sizeof(srv));
        ZeroMemory(uav, sizeof(uav));
        context()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
    }

    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        DirectX::XMMATRIX mat = {};
        context()->Map(mCameraConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        CAMERA_STATUS* camStatus = (CAMERA_STATUS*)msr.pData;
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvViewMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mViewProj), mat);
        camStatus->mEyePosition = mRSCameraInfo->EyePosition;
        context()->Unmap(mCameraConstantBuffer, 0);
        context()->Map(mTilingConstantBuffer, 0,
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
        context()->Unmap(mTilingConstantBuffer, 0);

        ID3D11Buffer* cb[] =
        { mCameraConstantBuffer,mTilingConstantBuffer,mActiveListConstantBuffer };
        ID3D11ShaderResourceView* srv[] =
        { mViewSpacePos_Srv,mMaxRadius_Srv,mAliveIndex_Srv,
        mDepthTex_Srv,mCoarseTileIndex_Srv,mCoarseTileIndexCounter_Srv };
        ID3D11UnorderedAccessView* uav[] = { mTiledIndex_Uav };
        UINT initial[] = { (UINT)-1 };

        context()->CSSetShader(mTileCullingShader, nullptr, 0);
        context()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, initial);

        context()->Dispatch(
            g_ParticleSetUpPass->GetTilingConstantInfo().mNumTilesX,
            g_ParticleSetUpPass->GetTilingConstantInfo().mNumTilesY, 1);

        ZeroMemory(cb, sizeof(cb));
        ZeroMemory(srv, sizeof(srv));
        ZeroMemory(uav, sizeof(uav));
        context()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
    }

    {
        D3D11_MAPPED_SUBRESOURCE msr = {};
        DirectX::XMMATRIX mat = {};
        context()->Map(mCameraConstantBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        CAMERA_STATUS* camStatus = (CAMERA_STATUS*)msr.pData;
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvViewMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvView), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->InvProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mInvProj), mat);
        mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewProjMatrix);
        mat = DirectX::XMMatrixTranspose(mat);
        DirectX::XMStoreFloat4x4(&(camStatus->mViewProj), mat);
        camStatus->mEyePosition = mRSCameraInfo->EyePosition;
        context()->Unmap(mCameraConstantBuffer, 0);
        context()->Map(mTilingConstantBuffer, 0,
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
        context()->Unmap(mTilingConstantBuffer, 0);

        ID3D11Buffer* cb[] =
        { mCameraConstantBuffer,mTilingConstantBuffer };
        ID3D11ShaderResourceView* srv[] =
        { mPartA_Srv,mViewSpacePos_Srv,mDepthTex_Srv,mTiledIndex_Srv,
        mCoarseTileIndexCounter_Srv,mParticleTex_Srv };
        ID3D11UnorderedAccessView* uav[] = { mParticleRender_Uav };
        UINT initial[] = { (UINT)-1 };
        ID3D11SamplerState* sam[] = { mLinearClampSampler };

        context()->CSSetShader(mTileRenderShader, nullptr, 0);
        context()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, initial);
        context()->CSSetSamplers(0, ARRAYSIZE(sam), sam);

        context()->Dispatch(
            g_ParticleSetUpPass->GetTilingConstantInfo().mNumTilesX,
            g_ParticleSetUpPass->GetTilingConstantInfo().mNumTilesY, 1);

        ZeroMemory(cb, sizeof(cb));
        ZeroMemory(srv, sizeof(srv));
        ZeroMemory(uav, sizeof(uav));
        context()->CSSetConstantBuffers(0, ARRAYSIZE(cb), cb);
        context()->CSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(uav), uav, nullptr);
        context()->CSSetShader(nullptr, nullptr, 0);
    }

    {
        static auto rtv = getRSDX11RootInstance()->getDevices()->getHighDynamicRtv();
        static D3D11_VIEWPORT vp = {};
        vp.Width = 1280.f; vp.Height = 720.f; vp.MinDepth = 0.f;
        vp.MaxDepth = 1.f; vp.TopLeftX = 0.f; vp.TopLeftY = 0.f;
        context()->VSSetShader(mBlendVertexShader, nullptr, 0);
        context()->PSSetShader(mBlendPixelShader, nullptr, 0);
        context()->OMSetBlendState(mParticleBlendState, nullptr, 0xFFFFFFFF);
        context()->OMSetRenderTargets(1, &rtv, nullptr);
        context()->RSSetViewports(1, &vp);
        context()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
        context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        ID3D11ShaderResourceView* srv[] = { mParticleRender_Srv };
        context()->PSSetShaderResources(0, ARRAYSIZE(srv), srv);

        context()->Draw(3, 0);

        ZeroMemory(srv, sizeof(srv));
        context()->PSSetShaderResources(0, ARRAYSIZE(srv), srv);
        context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
        static ID3D11RenderTargetView* nullrtv = nullptr;
        context()->OMSetRenderTargets(1, &nullrtv, nullptr);
    }
}

bool RSPass_PriticleTileRender::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_coarse_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mCoarseCullingShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_cull_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mTileCullingShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_render_compute.hlsl",
        "Main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mTileRenderShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_blend_vertex.hlsl",
        "Main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mBlendVertexShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(L".\\Assets\\Shaders\\ptc_blend_pixel.hlsl",
        "Main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(), nullptr, &mBlendPixelShader);
    RS_RELEASE(shaderBlob);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleTileRender::CreateViews()
{
    HRESULT hr = S_OK;

    std::wstring path = L".\\Assets\\Textures\\particle_atlas.dds";
    hr = DirectX::CreateDDSTextureFromFile(device(), path.c_str(),
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

    hr = device()->CreateSamplerState(&sampDesc, &mLinearClampSampler);
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
    hr = device()->CreateBlendState(&bldDesc, &mParticleBlendState);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_PriticleTileRender::CheckResources()
{
    auto resManager = getRSDX11RootInstance()->getResourceManager();
    if (!resManager) { return false; }

    std::string name = PTC_CAMERA_CONSTANT_NAME;
    mCameraConstantBuffer = resManager->getResource(name)->
        Resource.Buffer;
    if (!mCameraConstantBuffer) { return false; }

    name = PTC_TILING_CONSTANT_NAME;
    mTilingConstantBuffer = resManager->getResource(name)->
        Resource.Buffer;
    if (!mTilingConstantBuffer) { return false; }

    name = PTC_ALIVE_LIST_CONSTANT_NAME;
    mActiveListConstantBuffer = resManager->getResource(name)->
        Resource.Buffer;
    if (!mActiveListConstantBuffer) { return false; }

    name = "mrt-depth";
    mDepthTex_Srv = resManager->getResource(name)->Srv;
    if (!mDepthTex_Srv) { return false; }

    name = PTC_VIEW_SPCACE_POS_NAME;
    mViewSpacePos_Srv = resManager->getResource(name)->Srv;
    if (!mViewSpacePos_Srv) { return false; }

    name = PTC_MAX_RADIUS_NAME;
    mMaxRadius_Srv = resManager->getResource(name)->Srv;
    if (!mMaxRadius_Srv) { return false; }

    name = PTC_ALIVE_INDEX_NAME;
    mAliveIndex_Srv = resManager->getResource(name)->Srv;
    mAliveIndex_Uav = resManager->getResource(name)->Uav;
    if (!mAliveIndex_Srv) { return false; }
    if (!mAliveIndex_Uav) { return false; }

    name = PTC_A_NAME;
    mPartA_Srv = resManager->getResource(name)->Srv;
    if (!mPartA_Srv) { return false; }

    name = PTC_COARSE_CULL_NAME;
    mCoarseTileIndex_Srv = resManager->getResource(name)->Srv;
    mCoarseTileIndex_Uav = resManager->getResource(name)->Uav;
    if (!mCoarseTileIndex_Srv) { return false; }
    if (!mCoarseTileIndex_Uav) { return false; }

    name = PTC_COARSE_CULL_COUNTER_NAME;
    mCoarseTileIndexCounter_Srv = resManager->getResource(name)->Srv;
    mCoarseTileIndexCounter_Uav = resManager->getResource(name)->Uav;
    if (!mCoarseTileIndexCounter_Srv) { return false; }
    if (!mCoarseTileIndexCounter_Uav) { return false; }

    name = PTC_TILED_INDEX_NAME;
    mTiledIndex_Srv = resManager->getResource(name)->Srv;
    mTiledIndex_Uav = resManager->getResource(name)->Uav;
    if (!mTiledIndex_Srv) { return false; }
    if (!mTiledIndex_Uav) { return false; }

    name = PTC_RENDER_BUFFER_NAME;
    mParticleRender_Srv = resManager->getResource(name)->Srv;
    mParticleRender_Uav = resManager->getResource(name)->Uav;
    if (!mParticleRender_Srv) { return false; }
    if (!mParticleRender_Uav) { return false; }

    return true;
}

RSPass_Sprite::RSPass_Sprite(std::string& _name,
    PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mPixelShader(nullptr),
    mDepthStencilState(nullptr),
    mBlendState(nullptr),
    mRenderTargetView(nullptr),
    mDrawCallType(DRAWCALL_TYPE::MAX),
    mDrawCallPipe(nullptr),
    mProjStructedBuffer(nullptr),
    mProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
    mLinearSampler(nullptr),
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
    if (HasBeenInited)
    {
        RS_ADDREF(mVertexShader);
        RS_ADDREF(mPixelShader);
        RS_ADDREF(mDepthStencilState);
        RS_ADDREF(mLinearSampler);
        RS_ADDREF(mProjStructedBufferSrv);
        RS_ADDREF(mProjStructedBuffer);
        RS_ADDREF(mInstanceStructedBufferSrv);
        RS_ADDREF(mInstanceStructedBuffer);
    }
}

RSPass_Sprite::~RSPass_Sprite()
{

}

RSPass_Sprite* RSPass_Sprite::clonePass()
{
    return new RSPass_Sprite(*this);
}

bool RSPass_Sprite::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateStates()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallType = DRAWCALL_TYPE::UI_SPRITE;
    mDrawCallPipe = g_Root->getDrawCallsPool()->
        getDrawCallsPipe(mDrawCallType);

    std::string name = "temp-ui-cam";
    mRSCameraInfo = g_Root->getCamerasContainer()->
        getRSCameraInfo(name);

    HasBeenInited = true;

    return true;
}

void RSPass_Sprite::releasePass()
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

void RSPass_Sprite::execuatePass()
{
    ID3D11RenderTargetView* rtvnull = nullptr;
    context()->OMSetRenderTargets(1,
        &mRenderTargetView, nullptr);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->OMSetDepthStencilState(mDepthStencilState, 0);
    static float factor[4] = { 0.f,0.f,0.f,0.f };
    context()->OMSetBlendState(mBlendState, factor, 0xFFFFFFFF);
    context()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);
    context()->PSSetSamplers(0, 1, &mLinearSampler);

    DirectX::XMMATRIX mat = {};
    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    context()->Map(mProjStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    OnlyProj* vp_data = (OnlyProj*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
    context()->Unmap(mProjStructedBuffer, 0);

    context()->VSSetShaderResources(
        0, 1, &mProjStructedBufferSrv);

    for (auto& call : mDrawCallPipe->Data)
    {
        auto vecPtr = call.InstanceData.DataArrayPtr;
        auto size = vecPtr->size();
        context()->Map(mInstanceStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            mat = DirectX::XMLoadFloat4x4(
                &(*vecPtr)[i].WorldMatrix);
            mat = DirectX::XMMatrixTranspose(mat);
            DirectX::XMStoreFloat4x4(&ins_data[i].WorldMatrix, mat);
            ins_data[i].MaterialData =
                (*vecPtr)[i].MaterialData;
            ins_data[i].CustomizedData1 =
                (*vecPtr)[i].CustomizedData1;
            ins_data[i].CustomizedData2 =
                (*vecPtr)[i].CustomizedData2;
        }
        context()->Unmap(mInstanceStructedBuffer, 0);

        context()->IASetInputLayout(
            call.MeshData.InputLayout);
        context()->IASetPrimitiveTopology(
            call.MeshData.TopologyType);
        context()->IASetVertexBuffers(
            0, 1, &call.MeshData.VertexBuffer,
            &stride, &offset);
        context()->IASetIndexBuffer(
            call.MeshData.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context()->VSSetShaderResources(
            1, 1, &mInstanceStructedBufferSrv);
        context()->PSSetShaderResources(
            0, 1, &(call.TextureData[0].Srv));

        context()->DrawIndexedInstanced(
            call.MeshData.IndexSize,
            (UINT)call.InstanceData.DataArrayPtr->size(), 0, 0, 0);
    }

    context()->OMSetRenderTargets(1, &rtvnull, nullptr);
    context()->OMSetDepthStencilState(nullptr, 0);
    context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

bool RSPass_Sprite::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\sprite_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\sprite_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
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
    hr = device()->CreateDepthStencilState(
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
    hr = device()->CreateBlendState(&bldDesc, &mBlendState);
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
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = sizeof(OnlyProj);
    bdc.StructureByteStride = sizeof(OnlyProj);
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mProjStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Sprite::CreateViews()
{
    mRenderTargetView = g_Root->getDevices()->getSwapChainRtv();

    D3D11_SHADER_RESOURCE_VIEW_DESC desSRV = {};
    HRESULT hr = S_OK;
    ZeroMemory(&desSRV, sizeof(desSRV));
    desSRV.Format = DXGI_FORMAT_UNKNOWN;
    desSRV.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desSRV.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
    hr = device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &desSRV, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    desSRV.Buffer.ElementWidth = 1;
    hr = device()->CreateShaderResourceView(
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

    hr = device()->CreateSamplerState(
        &sampDesc, &mLinearSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_SimpleLight::RSPass_SimpleLight(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mPixelShader(nullptr),
    mRenderTargetView(nullptr),
    mLinearWrapSampler(nullptr),
    mGeoBufferSrv(nullptr),
    mSsaoSrv(nullptr),
    mVertexBuffer(nullptr),
    mIndexBuffer(nullptr)
{

}

RSPass_SimpleLight::RSPass_SimpleLight(const RSPass_SimpleLight& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mRenderTargetView(_source.mRenderTargetView),
    mLinearWrapSampler(_source.mLinearWrapSampler),
    mGeoBufferSrv(_source.mGeoBufferSrv),
    mSsaoSrv(_source.mSsaoSrv),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer)
{

}

RSPass_SimpleLight::~RSPass_SimpleLight()
{

}

RSPass_SimpleLight* RSPass_SimpleLight::clonePass()
{
    return new RSPass_SimpleLight(*this);
}

bool RSPass_SimpleLight::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    HasBeenInited = true;

    return true;
}

void RSPass_SimpleLight::releasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mLinearWrapSampler);
    RS_RELEASE(mVertexBuffer);
    RS_RELEASE(mIndexBuffer);
}

void RSPass_SimpleLight::execuatePass()
{
    context()->OMSetRenderTargets(1, &mRenderTargetView, nullptr);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->ClearRenderTargetView(
        mRenderTargetView, DirectX::Colors::DarkGreen);
    context()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);

    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT offset = 0;

    static ID3D11ShaderResourceView* srvs[] =
    {
        mGeoBufferSrv, mSsaoSrv
    };
    context()->PSSetShaderResources(0, 2, srvs);

    static ID3D11SamplerState* samps[] =
    {
        mLinearWrapSampler,
    };
    context()->PSSetSamplers(0, 1, samps);

    context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context()->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    context()->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    context()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    ID3D11RenderTargetView* rtvnull = nullptr;
    context()->OMSetRenderTargets(1, &rtvnull, nullptr);
    static ID3D11ShaderResourceView* nullsrvs[] =
    {
        nullptr, nullptr
    };
    context()->PSSetShaderResources(0, 2, nullsrvs);
}

bool RSPass_SimpleLight::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\simplylit_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\simplylit_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
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

    vertex_type::TangentVertex v[4] = {};
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
    bufDesc.ByteWidth = sizeof(vertex_type::TangentVertex) * 4;
    bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = 0;
    bufDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = device()->CreateBuffer(&bufDesc, &vinitData, &mVertexBuffer);
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
    hr = device()->CreateBuffer(&bufDesc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_SimpleLight::CreateViews()
{
    mRenderTargetView = g_Root->getDevices()->getSwapChainRtv();

    std::string name = "mrt-geo-buffer";
    mGeoBufferSrv = g_Root->getResourceManager()->getResource(name)->Srv;
    name = "ssao-tex-compress-ssao";
    mSsaoSrv = g_Root->getResourceManager()->getResource(name)->Srv;

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
    hr = device()->CreateSamplerState(
        &sampDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_Billboard::RSPass_Billboard(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mGeometryShader(nullptr),
    mPixelShader(nullptr),
    mBlendState(nullptr),
    mRenderTargetView(nullptr),
    mDepthStencilView(nullptr),
    mLinearWrapSampler(nullptr),
    mViewProjStructedBuffer(nullptr),
    mViewProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
    mDrawCallType(DRAWCALL_TYPE::TRANSPARENCY),
    mDrawCallPipe(nullptr),
    mRSCameraInfo(nullptr),
    mRSCamera(nullptr)
{

}

RSPass_Billboard::RSPass_Billboard(const RSPass_Billboard& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mGeometryShader(_source.mGeometryShader),
    mPixelShader(_source.mPixelShader),
    mBlendState(_source.mBlendState),
    mRenderTargetView(_source.mRenderTargetView),
    mDepthStencilView(_source.mDepthStencilView),
    mLinearWrapSampler(_source.mLinearWrapSampler),
    mViewProjStructedBuffer(_source.mViewProjStructedBuffer),
    mViewProjStructedBufferSrv(_source.mViewProjStructedBufferSrv),
    mInstanceStructedBuffer(_source.mInstanceStructedBuffer),
    mInstanceStructedBufferSrv(_source.mInstanceStructedBufferSrv),
    mDrawCallType(_source.mDrawCallType),
    mDrawCallPipe(_source.mDrawCallPipe),
    mRSCameraInfo(_source.mRSCameraInfo),
    mRSCamera(_source.mRSCamera)
{
    if (HasBeenInited)
    {
        RS_ADDREF(mVertexShader);
        RS_ADDREF(mGeometryShader);
        RS_ADDREF(mPixelShader);
        RS_ADDREF(mLinearWrapSampler);
        RS_ADDREF(mBlendState);
        RS_ADDREF(mViewProjStructedBuffer);
        RS_ADDREF(mViewProjStructedBufferSrv);
        RS_ADDREF(mInstanceStructedBuffer);
        RS_ADDREF(mInstanceStructedBufferSrv);
    }
}

RSPass_Billboard::~RSPass_Billboard()
{

}

RSPass_Billboard* RSPass_Billboard::clonePass()
{
    return new RSPass_Billboard(*this);
}

bool RSPass_Billboard::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateStates()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallType = DRAWCALL_TYPE::TRANSPARENCY;
    mDrawCallPipe = g_Root->getDrawCallsPool()->getDrawCallsPipe(mDrawCallType);

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->getCamerasContainer()->getRSCameraInfo(name);
    mRSCamera = g_Root->getCamerasContainer()->getRSCamera(name);

    HasBeenInited = true;

    return true;
}

void RSPass_Billboard::releasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mGeometryShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mBlendState);
    RS_RELEASE(mLinearWrapSampler);
    RS_RELEASE(mViewProjStructedBuffer);
    RS_RELEASE(mViewProjStructedBufferSrv);
    RS_RELEASE(mInstanceStructedBuffer);
    RS_RELEASE(mInstanceStructedBufferSrv);
}

void RSPass_Billboard::execuatePass()
{
    context()->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
    context()->RSSetViewports(1, &g_ViewPort);
    static float factor[4] = { 0.f,0.f,0.f,0.f };
    context()->OMSetBlendState(mBlendState, factor, 0xFFFFFFFF);
    context()->VSSetShader(mVertexShader, nullptr, 0);
    context()->GSSetShader(mGeometryShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);
    context()->PSSetSamplers(0, 1, &mLinearWrapSampler);

    DirectX::XMMATRIX mat = {};
    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT offset = 0;

    D3D11_MAPPED_SUBRESOURCE msr = {};
    context()->Map(mViewProjStructedBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    ViewProjCamUpPos* vp_data = (ViewProjCamUpPos*)msr.pData;
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ViewMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mViewMat, mat);
    mat = DirectX::XMLoadFloat4x4(&mRSCameraInfo->ProjMatrix);
    mat = DirectX::XMMatrixTranspose(mat);
    DirectX::XMStoreFloat4x4(&vp_data[0].mProjMat, mat);
    vp_data->mCamUpVec = mRSCamera->getRSCameraUpVector();
    vp_data->mCamPos = mRSCamera->getRSCameraPosition();
    context()->Unmap(mViewProjStructedBuffer, 0);

    context()->GSSetShaderResources(0, 1, &mViewProjStructedBufferSrv);

    for (auto& call : mDrawCallPipe->Data)
    {
        auto vecPtr = call.InstanceData.DataArrayPtr;
        auto size = vecPtr->size();
        context()->Map(mInstanceStructedBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        RS_INSTANCE_DATA* ins_data = (RS_INSTANCE_DATA*)msr.pData;
        for (size_t i = 0; i < size; i++)
        {
            mat = DirectX::XMLoadFloat4x4(
                &(*vecPtr)[i].WorldMatrix);
            mat = DirectX::XMMatrixTranspose(mat);
            DirectX::XMStoreFloat4x4(&ins_data[i].WorldMatrix, mat);
            ins_data[i].MaterialData =
                (*vecPtr)[i].MaterialData;
            ins_data[i].CustomizedData1 =
                (*vecPtr)[i].CustomizedData1;
            ins_data[i].CustomizedData2 =
                (*vecPtr)[i].CustomizedData2;
        }
        context()->Unmap(mInstanceStructedBuffer, 0);

        context()->IASetInputLayout(call.MeshData.InputLayout);
        context()->IASetPrimitiveTopology(call.MeshData.TopologyType);
        context()->IASetVertexBuffers(0, 1, &call.MeshData.VertexBuffer,
            &stride, &offset);
        context()->IASetIndexBuffer(
            call.MeshData.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        context()->VSSetShaderResources(0, 1, &mInstanceStructedBufferSrv);
        context()->PSSetShaderResources(0, 1, &call.TextureData[0].Srv);

        context()->DrawIndexedInstanced(
            call.MeshData.IndexSize,
            (UINT)call.InstanceData.DataArrayPtr->size(), 0, 0, 0);
    }

    static ID3D11RenderTargetView* nullrtv[] = { nullptr };
    static ID3D11ShaderResourceView* nullsrv[] = { nullptr };
    context()->OMSetRenderTargets(1, nullrtv, nullptr);
    context()->OMSetBlendState(nullptr, factor, 0xFFFFFFFF);
    context()->GSSetShader(nullptr, nullptr, 0);
    context()->VSSetShaderResources(0, 1, nullsrv);
    context()->GSSetShaderResources(0, 1, nullsrv);
    context()->PSSetShaderResources(0, 1, nullsrv);
}

bool RSPass_Billboard::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\billboard_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\billboard_geometry.hlsl",
        "main", "gs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateGeometryShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mGeometryShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\billboard_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Billboard::CreateStates()
{
    HRESULT hr = S_OK;

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
    hr = device()->CreateBlendState(&bldDesc, &mBlendState);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Billboard::CreateBuffers()
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
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mInstanceStructedBuffer);
    if (FAILED(hr)) { return false; }

    bdc.ByteWidth = sizeof(ViewProjCamUpPos);
    bdc.StructureByteStride = sizeof(ViewProjCamUpPos);
    hr = device()->CreateBuffer(
        &bdc, nullptr, &mViewProjStructedBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Billboard::CreateViews()
{
    mRenderTargetView = g_Root->getDevices()->getHighDynamicRtv();

    std::string name = "";
    name = "mrt-depth";
    mDepthStencilView = g_Root->getResourceManager()->
        getResource(name)->Dsv;
    if (!mDepthStencilView) { return false; }

    HRESULT hr = S_OK;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
    hr = device()->CreateShaderResourceView(
        mInstanceStructedBuffer,
        &srvDesc, &mInstanceStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    srvDesc.Buffer.ElementWidth = 1;
    hr = device()->CreateShaderResourceView(
        mViewProjStructedBuffer,
        &srvDesc, &mViewProjStructedBufferSrv);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Billboard::CreateSamplers()
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
    hr = device()->CreateSamplerState(
        &sampDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_Tonemapping::RSPass_Tonemapping(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mAverLuminShader(nullptr),
    mDynamicExposureShader(nullptr),
    mToneMapShader(nullptr),
    mHdrUav(nullptr),
    mHdrSrv(nullptr),
    mAverageLuminBufferArray({ nullptr,nullptr }),
    mAverageLuminSrvArray({ nullptr,nullptr }),
    mAverageLuminUavArray({ nullptr,nullptr })
{

}

RSPass_Tonemapping::RSPass_Tonemapping(const RSPass_Tonemapping& _source) :
    RSPass_Base(_source),
    mAverLuminShader(_source.mAverLuminShader),
    mDynamicExposureShader(_source.mDynamicExposureShader),
    mToneMapShader(_source.mToneMapShader),
    mHdrUav(_source.mHdrUav),
    mHdrSrv(_source.mHdrSrv),
    mAverageLuminBufferArray(_source.mAverageLuminBufferArray),
    mAverageLuminSrvArray(_source.mAverageLuminSrvArray),
    mAverageLuminUavArray(_source.mAverageLuminUavArray)
{
    if (HasBeenInited)
    {
        RS_ADDREF(mDynamicExposureShader);
        RS_ADDREF(mAverLuminShader);
        RS_ADDREF(mToneMapShader);
        RS_ADDREF(mAverageLuminBufferArray[0]);
        RS_ADDREF(mAverageLuminBufferArray[1]);
        RS_ADDREF(mAverageLuminSrvArray[0]);
        RS_ADDREF(mAverageLuminSrvArray[1]);
        RS_ADDREF(mAverageLuminUavArray[0]);
        RS_ADDREF(mAverageLuminUavArray[1]);
    }
}

RSPass_Tonemapping::~RSPass_Tonemapping()
{

}

RSPass_Tonemapping* RSPass_Tonemapping::clonePass()
{
    return new RSPass_Tonemapping(*this);
}

bool RSPass_Tonemapping::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }

    HasBeenInited = true;

    return true;
}

void RSPass_Tonemapping::releasePass()
{
    RS_RELEASE(mDynamicExposureShader);
    RS_RELEASE(mAverLuminShader);
    RS_RELEASE(mToneMapShader);
    RS_RELEASE(mAverageLuminBufferArray[0]);
    RS_RELEASE(mAverageLuminBufferArray[1]);
    RS_RELEASE(mAverageLuminSrvArray[0]);
    RS_RELEASE(mAverageLuminSrvArray[1]);
    RS_RELEASE(mAverageLuminUavArray[0]);
    RS_RELEASE(mAverageLuminUavArray[1]);
}

void RSPass_Tonemapping::execuatePass()
{
    static ID3D11UnorderedAccessView* nullUav = nullptr;
    static ID3D11ShaderResourceView* nullSrv = nullptr;

    UINT width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
    UINT height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
    UINT dispatchX = rs_tool::align(width, 32) / 32;
    UINT dispatchY = rs_tool::align(height, 32) / 32;

    static int cindex = 0;
    int pindex = cindex;
    cindex = (cindex + 1) % 2;

    context()->CSSetShader(mAverLuminShader, nullptr, 0);
    context()->CSSetShaderResources(0, 1, &mHdrSrv);
    context()->CSSetUnorderedAccessViews(0, 1,
        &mAverageLuminUavArray[cindex], nullptr);

    context()->Dispatch(dispatchX, dispatchY, 1);

    context()->CSSetShaderResources(0, 1, &nullSrv);
    context()->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

    context()->CSSetShader(mDynamicExposureShader, nullptr, 0);
    context()->CSSetShaderResources(0, 1,
        &mAverageLuminSrvArray[pindex]);
    context()->CSSetUnorderedAccessViews(0, 1,
        &mAverageLuminUavArray[cindex], nullptr);

    context()->Dispatch(1, 1, 1);

    context()->CSSetShaderResources(0, 1, &nullSrv);
    context()->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);

    dispatchX = rs_tool::align(width, 256) / 256;
    context()->CSSetShader(mToneMapShader, nullptr, 0);
    context()->CSSetShaderResources(0, 1,
        &mAverageLuminSrvArray[cindex]);
    context()->CSSetUnorderedAccessViews(0, 1, &mHdrUav, nullptr);

    context()->Dispatch(dispatchX, height, 1);

    context()->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);
}

bool RSPass_Tonemapping::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\average_lumin_compute.hlsl",
        "main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mAverLuminShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    std::string transSpdStr = "(" +
        std::to_string(g_RenderEffectConfig.mExpoTransSpeed) + "f)";
    std::string expoMinStr = "(" +
        std::to_string(g_RenderEffectConfig.mExpoMin) + "f)";
    std::string expoMaxStr = "(" +
        std::to_string(g_RenderEffectConfig.mExpoMax) + "f)";
    std::string invFactorStr = "(" +
        std::to_string(g_RenderEffectConfig.mExpoInvFactor) + "f)";
    D3D_SHADER_MACRO expoMacro[] =
    {
        { "TRANS_SPEED", transSpdStr.c_str() },
        { "EXPO_MIN", expoMinStr.c_str() },
        { "EXPO_MAX", expoMaxStr.c_str() },
        { "INV_FACTOR", invFactorStr.c_str() },
        { nullptr, nullptr }
    };
    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\dynamic_exposure_compute.hlsl",
        "main", "cs_5_0", &shaderBlob, expoMacro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mDynamicExposureShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    std::string staticV = "(";
    D3D_SHADER_MACRO macro[2] = { { nullptr, nullptr }, { nullptr, nullptr } };
    if (!g_RenderEffectConfig.mDynamicExpoOff)
    {
        macro[0] = { "DYNAMIC_EXPOSURE", "1" };
    }
    else
    {
        staticV += std::to_string(g_RenderEffectConfig.mStaticExpo);
        staticV += "f)";
        macro[0] = { "STATIC_EXPOSURE", staticV.c_str() };
    }
    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\tonemap_compute.hlsl",
        "main", "cs_5_0", &shaderBlob, macro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mToneMapShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_Tonemapping::CreateViews()
{
    mHdrUav = g_Root->getDevices()->getHighDynamicUav();
    mHdrSrv = g_Root->getDevices()->getHighDynamicSrv();

    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bfrDesc = {};
    D3D11_SUBRESOURCE_DATA initData = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

    UINT width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
    UINT height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
    // TODO shader hasn't been supported for other sreen size
    UINT expoSize = rs_tool::align(width, 32) / 32 *
        rs_tool::align(height, 32) / 32;

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    ZeroMemory(&initData, sizeof(initData));
    bfrDesc.Usage = D3D11_USAGE_DEFAULT;
    bfrDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    bfrDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bfrDesc.ByteWidth = expoSize * (UINT)sizeof(float);
    bfrDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bfrDesc.StructureByteStride = sizeof(float);
    float* data = new float[expoSize];
    data[0] = g_RenderEffectConfig.mStaticExpo;
    initData.pSysMem = data;
    hr = device()->CreateBuffer(&bfrDesc, &initData,
        &mAverageLuminBufferArray[0]);
    if (FAILED(hr)) { delete[] data; return false; }
    hr = device()->CreateBuffer(&bfrDesc, &initData,
        &mAverageLuminBufferArray[1]);
    if (FAILED(hr)) { delete[] data; return false; }
    delete[] data;

    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementWidth = expoSize;
    hr = device()->CreateShaderResourceView(mAverageLuminBufferArray[0],
        &srvDesc, &mAverageLuminSrvArray[0]);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateShaderResourceView(mAverageLuminBufferArray[1],
        &srvDesc, &mAverageLuminSrvArray[1]);
    if (FAILED(hr)) { return false; }

    ZeroMemory(&uavDesc, sizeof(uavDesc));
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.NumElements = expoSize;
    hr = device()->CreateUnorderedAccessView(mAverageLuminBufferArray[0],
        &uavDesc, &mAverageLuminUavArray[0]);
    if (FAILED(hr)) { return false; }
    hr = device()->CreateUnorderedAccessView(mAverageLuminBufferArray[1],
        &uavDesc, &mAverageLuminUavArray[1]);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_BloomHdr::RSPass_BloomHdr(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mFilterPixelShader(nullptr),
    mKABlurHoriShader(nullptr),
    mKABlurVertShader(nullptr),
    mBlurHoriShader(nullptr),
    mBlurVertShader(nullptr),
    mUpSampleShader(nullptr),
    mBlendShader(nullptr),
    mLinearBorderSampler(nullptr),
    mBlurConstBuffer(nullptr),
    mIntensityConstBuffer(nullptr),
    mHdrSrv(nullptr),
    mHdrUav(nullptr),
    mNeedBloomTexture(nullptr),
    mNeedBloomSrv(nullptr),
    mNeedBloomUavArray({ nullptr }),
    mUpSampleTexture(nullptr),
    mUpSampleSrv(nullptr),
    mUpSampleUavArray({ nullptr })
{

}

RSPass_BloomHdr::RSPass_BloomHdr(const RSPass_BloomHdr& _source) :
    RSPass_Base(_source),
    mFilterPixelShader(_source.mFilterPixelShader),
    mKABlurHoriShader(_source.mKABlurHoriShader),
    mKABlurVertShader(_source.mKABlurVertShader),
    mBlurHoriShader(_source.mBlurHoriShader),
    mBlurVertShader(_source.mBlurVertShader),
    mUpSampleShader(_source.mUpSampleShader),
    mBlendShader(_source.mBlendShader),
    mLinearBorderSampler(_source.mLinearBorderSampler),
    mBlurConstBuffer(_source.mBlurConstBuffer),
    mIntensityConstBuffer(_source.mIntensityConstBuffer),
    mHdrSrv(_source.mHdrSrv),
    mHdrUav(_source.mHdrUav),
    mNeedBloomTexture(_source.mNeedBloomTexture),
    mNeedBloomSrv(_source.mNeedBloomSrv),
    mNeedBloomUavArray({ _source.mNeedBloomUavArray }),
    mUpSampleTexture(_source.mUpSampleTexture),
    mUpSampleSrv(_source.mUpSampleSrv),
    mUpSampleUavArray({ _source.mUpSampleUavArray })
{
    if (HasBeenInited)
    {
        RS_ADDREF(mUpSampleShader);
        RS_ADDREF(mBlurHoriShader);
        RS_ADDREF(mBlurVertShader);
        RS_ADDREF(mKABlurHoriShader);
        RS_ADDREF(mKABlurVertShader);
        RS_ADDREF(mFilterPixelShader);
        RS_ADDREF(mBlendShader);
        RS_ADDREF(mBlurConstBuffer);
        RS_ADDREF(mIntensityConstBuffer);
        RS_ADDREF(mLinearBorderSampler);
        RS_ADDREF(mNeedBloomTexture);
        RS_ADDREF(mNeedBloomSrv);
        for (auto uav : mNeedBloomUavArray) { RS_ADDREF(uav); }
        RS_ADDREF(mUpSampleTexture);
        RS_ADDREF(mUpSampleSrv);
        for (auto uav : mUpSampleUavArray) { RS_ADDREF(uav); }
    }
}

RSPass_BloomHdr::~RSPass_BloomHdr()
{

}

RSPass_BloomHdr* RSPass_BloomHdr::clonePass()
{
    return new RSPass_BloomHdr(*this);
}

bool RSPass_BloomHdr::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateSampler()) { return false; }

    HasBeenInited = true;

    return true;
}

void RSPass_BloomHdr::releasePass()
{
    RS_RELEASE(mFilterPixelShader);
    RS_RELEASE(mUpSampleShader);
    RS_RELEASE(mBlurHoriShader);
    RS_RELEASE(mBlurVertShader);
    RS_RELEASE(mKABlurHoriShader);
    RS_RELEASE(mKABlurVertShader);
    RS_RELEASE(mBlendShader);
    RS_RELEASE(mBlurConstBuffer);
    RS_RELEASE(mIntensityConstBuffer);
    RS_RELEASE(mLinearBorderSampler);
    RS_RELEASE(mNeedBloomTexture);
    RS_RELEASE(mNeedBloomSrv);
    for (auto uav : mNeedBloomUavArray) { RS_RELEASE(uav); }
    RS_RELEASE(mUpSampleTexture);
    RS_RELEASE(mUpSampleSrv);
    for (auto uav : mUpSampleUavArray) { RS_RELEASE(uav); }
}

void RSPass_BloomHdr::execuatePass()
{
    static ID3D11UnorderedAccessView* nullUav = nullptr;
    static ID3D11ShaderResourceView* nullSrv = nullptr;
    UINT width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
    UINT height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
    UINT dispatchX = rs_tool::align(width, 16) / 16;
    UINT dispatchY = rs_tool::align(height, 16) / 16;

    context()->CSSetShader(mFilterPixelShader, nullptr, 0);
    context()->CSSetShaderResources(0, 1, &mHdrSrv);
    context()->CSSetUnorderedAccessViews(0, 1,
        mNeedBloomUavArray.data(), nullptr);

    context()->Dispatch(dispatchX, dispatchY, 1);

    context()->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);
    context()->CSSetShaderResources(0, 1, &nullSrv);

    context()->GenerateMips(mNeedBloomSrv);

    D3D11_MAPPED_SUBRESOURCE msr = {};

    static const size_t DOWN_COUNT = g_RenderEffectConfig.mBloomDownSamplingCount;
    static const size_t UP_COUNT = DOWN_COUNT - 1;
    static const size_t BLUR_COUNT = g_RenderEffectConfig.mBloomBlurCount;

    {
        auto index = 1;
        UINT blurTexWidth = width / (1 << index);
        UINT blurTexHeight = height / (1 << index);
        context()->Map(mBlurConstBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        BLM_BLUR_INFO* info = (BLM_BLUR_INFO*)msr.pData;
        info->mTexWidth = blurTexWidth;
        info->mTexHeight = blurTexHeight;
        context()->Unmap(mBlurConstBuffer, 0);

        context()->CSSetUnorderedAccessViews(0, 1,
            &mNeedBloomUavArray[index], nullptr);
        context()->CSSetConstantBuffers(0, 1, &mBlurConstBuffer);

        for (size_t j = 0; j < BLUR_COUNT; j++)
        {
            context()->CSSetShader(mKABlurHoriShader,
                nullptr, 0);
            dispatchX = rs_tool::align(blurTexWidth, 256) / 256;
            dispatchY = blurTexHeight;
            context()->Dispatch(dispatchX, dispatchY, 1);

            context()->CSSetShader(mKABlurVertShader,
                nullptr, 0);
            dispatchX = blurTexWidth;
            dispatchY = rs_tool::align(blurTexHeight, 256) / 256;
            context()->Dispatch(dispatchX, dispatchY, 1);
        }
    }

    for (size_t i = 1; i < DOWN_COUNT; i++)
    {
        auto index = i + 1;
        UINT blurTexWidth = width / (1 << index);
        UINT blurTexHeight = height / (1 << index);
        context()->Map(mBlurConstBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        BLM_BLUR_INFO* info = (BLM_BLUR_INFO*)msr.pData;
        info->mTexWidth = blurTexWidth;
        info->mTexHeight = blurTexHeight;
        context()->Unmap(mBlurConstBuffer, 0);

        context()->CSSetUnorderedAccessViews(0, 1,
            &mNeedBloomUavArray[index], nullptr);
        context()->CSSetConstantBuffers(0, 1, &mBlurConstBuffer);

        for (size_t j = 0; j < BLUR_COUNT; j++)
        {
            context()->CSSetShader(mBlurHoriShader,
                nullptr, 0);
            dispatchX = rs_tool::align(blurTexWidth, 256) / 256;
            dispatchY = blurTexHeight;
            context()->Dispatch(dispatchX, dispatchY, 1);

            context()->CSSetShader(mBlurVertShader,
                nullptr, 0);
            dispatchX = blurTexWidth;
            dispatchY = rs_tool::align(blurTexHeight, 256) / 256;
            context()->Dispatch(dispatchX, dispatchY, 1);
        }
    }
    static ID3D11UnorderedAccessView* nulluav = nullptr;
    static ID3D11ShaderResourceView* nullsrv = nullptr;
    context()->CSSetUnorderedAccessViews(0, 1, &nulluav, nullptr);

    width /= 2;
    height /= 2;
    context()->CSSetShader(mUpSampleShader, nullptr, 0);
    context()->CSSetShaderResources(0, 1, &mNeedBloomSrv);
    context()->CSSetSamplers(0, 1, &mLinearBorderSampler);
    for (size_t i = 0; i < UP_COUNT; i++)
    {
        auto inv_i = UP_COUNT - 1 - i;
        UINT texWidth = width / (1 << inv_i);
        UINT texHeight = height / (1 << inv_i);

        context()->Map(mBlurConstBuffer, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &msr);
        BLM_BLUR_INFO* info = (BLM_BLUR_INFO*)msr.pData;
        info->mTexWidth = texWidth;
        info->mTexHeight = texHeight;
        info->mPads[0] = static_cast<UINT>(inv_i);
        context()->Unmap(mBlurConstBuffer, 0);

        context()->CSSetUnorderedAccessViews(0, 2,
            &mUpSampleUavArray[inv_i], nullptr);

        dispatchX = rs_tool::align(texWidth, 16) / 16;
        dispatchY = rs_tool::align(texHeight, 16) / 16;
        context()->Dispatch(dispatchX, dispatchY, 1);
    }
    context()->CSSetUnorderedAccessViews(0, 1, &nulluav, nullptr);
    context()->CSSetUnorderedAccessViews(1, 1, &nulluav, nullptr);


    context()->Map(mIntensityConstBuffer, 0,
        D3D11_MAP_WRITE_DISCARD, 0, &msr);
    BLM_INTENSITY_INFO* info = (BLM_INTENSITY_INFO*)msr.pData;
    info->mIntensityFactor = g_RenderEffectConfig.mBloomIntensityFactor;
    context()->Unmap(mIntensityConstBuffer, 0);
    context()->CSSetShader(mBlendShader, nullptr, 0);
    context()->CSSetConstantBuffers(0, 1, &mIntensityConstBuffer);
    context()->CSSetShaderResources(0, 1, &mUpSampleSrv);
    context()->CSSetUnorderedAccessViews(0, 1, &mHdrUav, nullptr);
    dispatchX = rs_tool::align(width * 2, 16) / 16;
    dispatchY = rs_tool::align(height * 2, 16) / 16;
    context()->Dispatch(dispatchX, dispatchY, 1);

    context()->CSSetShaderResources(0, 1, &nullsrv);
    context()->CSSetUnorderedAccessViews(0, 1, &nulluav, nullptr);
}

bool RSPass_BloomHdr::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    const std::string pickMinValue = std::to_string(
        g_RenderEffectConfig.mBloomMinValue);
    D3D_SHADER_MACRO pickMacro[] =
    {
        { "MIN_VALUE", pickMinValue.c_str() },
        { nullptr, nullptr }
    };
    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\bloom_pick_compute.hlsl",
        "main", "cs_5_0", &shaderBlob, pickMacro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mFilterPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    std::string kernelSize = "";
    std::string kernelHalf = "";
    std::string weightArray = "";
    {
        UINT kernel = g_RenderEffectConfig.mBloomBlurKernel;
        UINT half = kernel / 2;
        float sigma = g_RenderEffectConfig.mBloomBlurSigma;

        std::vector<float> weights = {};
        rs_tool::calcGaussWeight1D(kernel, sigma, weights);

        kernelSize = std::to_string(kernel);
        kernelHalf = std::to_string(half);
        weightArray = "static const float gBlurWeight[] = {";
        for (const auto& w : weights)
        {
            weightArray += std::to_string(w) + "f,";
        }
        weightArray += "}";
    }
    D3D_SHADER_MACRO blurMacro[] =
    {
        { "KERNEL_SIZE", kernelSize.c_str() },
        { "KERNEL_HALF", kernelHalf.c_str() },
        { "WEIGHT_ARRAY", weightArray.c_str() },
        { nullptr, nullptr }
    };
    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\gauss_blur_compute.hlsl",
        "HMain", "cs_5_0", &shaderBlob, blurMacro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mBlurHoriShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\gauss_blur_compute.hlsl",
        "VMain", "cs_5_0", &shaderBlob, blurMacro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mBlurVertShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\karis_average_compute.hlsl",
        "HMain", "cs_5_0", &shaderBlob, blurMacro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mKABlurHoriShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\karis_average_compute.hlsl",
        "VMain", "cs_5_0", &shaderBlob, blurMacro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mKABlurVertShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\bloom_upsample_compute.hlsl",
        "main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mUpSampleShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\bloom_blend_compute.hlsl",
        "main", "cs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mBlendShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_BloomHdr::CreateViews()
{
    mHdrUav = g_Root->getDevices()->getHighDynamicUav();
    mHdrSrv = g_Root->getDevices()->getHighDynamicSrv();

    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC texDesc = {};
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    ZeroMemory(&texDesc, sizeof(texDesc));
    ZeroMemory(&rtvDesc, sizeof(rtvDesc));
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    ZeroMemory(&uavDesc, sizeof(uavDesc));

    UINT downMips = g_RenderEffectConfig.mBloomDownSamplingCount + 1;
    UINT upMips = downMips - 2;

    texDesc.Width = g_Root->getDevices()->getCurrWndWidth();
    texDesc.Height = g_Root->getDevices()->getCurrWndHeight();
    texDesc.MipLevels = downMips;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET |
        D3D11_BIND_SHADER_RESOURCE |
        D3D11_BIND_UNORDERED_ACCESS;
    hr = device()->CreateTexture2D(&texDesc, nullptr, &mNeedBloomTexture);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = downMips;
    hr = device()->CreateShaderResourceView(mNeedBloomTexture,
        &srvDesc, &mNeedBloomSrv);
    if (FAILED(hr)) { return false; }

    for (size_t i = 0; i < downMips; i++)
    {
        uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = static_cast<UINT>(i);
        hr = device()->CreateUnorderedAccessView(mNeedBloomTexture,
            &uavDesc, &mNeedBloomUavArray[i]);
        if (FAILED(hr)) { return false; }
    }

    texDesc.Width /= 2;
    texDesc.Height /= 2;
    texDesc.MipLevels = upMips;
    texDesc.MiscFlags = 0;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE |
        D3D11_BIND_UNORDERED_ACCESS;
    srvDesc.Texture2D.MipLevels = upMips;
    hr = device()->CreateTexture2D(&texDesc, nullptr,
        &mUpSampleTexture);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateShaderResourceView(mUpSampleTexture,
        &srvDesc, &mUpSampleSrv);
    if (FAILED(hr)) { return false; }

    for (size_t i = 0; i < upMips; i++)
    {
        uavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = static_cast<UINT>(i);
        hr = device()->CreateUnorderedAccessView(mUpSampleTexture,
            &uavDesc, &mUpSampleUavArray[i]);
        if (FAILED(hr)) { return false; }
    }

    return true;
}

bool RSPass_BloomHdr::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bfrDesc = {};

    ZeroMemory(&bfrDesc, sizeof(bfrDesc));
    bfrDesc.Usage = D3D11_USAGE_DYNAMIC;
    bfrDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bfrDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bfrDesc.ByteWidth = sizeof(BLM_BLUR_INFO);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mBlurConstBuffer);
    if (FAILED(hr)) { return false; }

    bfrDesc.ByteWidth = sizeof(BLM_INTENSITY_INFO);
    hr = device()->CreateBuffer(&bfrDesc, nullptr, &mIntensityConstBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_BloomHdr::CreateSampler()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));

    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device()->CreateSamplerState(&sampDesc, &mLinearBorderSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_ToSwapChain::RSPass_ToSwapChain(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mVertexShader(nullptr),
    mPixelShader(nullptr),
    mVertexBuffer(nullptr),
    mIndexBuffer(nullptr),
    mSwapChainRtv(nullptr),
    mHdrSrv(nullptr),
    mLinearWrapSampler(nullptr)
{

}

RSPass_ToSwapChain::RSPass_ToSwapChain(const RSPass_ToSwapChain& _source) :
    RSPass_Base(_source),
    mVertexShader(_source.mVertexShader),
    mPixelShader(_source.mPixelShader),
    mVertexBuffer(_source.mVertexBuffer),
    mIndexBuffer(_source.mIndexBuffer),
    mSwapChainRtv(_source.mSwapChainRtv),
    mHdrSrv(_source.mHdrSrv),
    mLinearWrapSampler(_source.mLinearWrapSampler)
{
    if (HasBeenInited)
    {
        RS_ADDREF(mVertexBuffer);
        RS_ADDREF(mIndexBuffer);
        RS_ADDREF(mVertexShader);
        RS_ADDREF(mPixelShader);
        RS_ADDREF(mLinearWrapSampler);
    }
}

RSPass_ToSwapChain::~RSPass_ToSwapChain()
{

}

RSPass_ToSwapChain* RSPass_ToSwapChain::clonePass()
{
    return new RSPass_ToSwapChain(*this);
}

bool RSPass_ToSwapChain::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateBuffers()) { return false; }
    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    HasBeenInited = true;

    return true;
}

void RSPass_ToSwapChain::releasePass()
{
    RS_RELEASE(mVertexBuffer);
    RS_RELEASE(mIndexBuffer);
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mLinearWrapSampler);
}

void RSPass_ToSwapChain::execuatePass()
{
    context()->OMSetRenderTargets(1, &mSwapChainRtv, nullptr);
    context()->RSSetViewports(1, &g_ViewPort);
    context()->ClearRenderTargetView(
        mSwapChainRtv, DirectX::Colors::DarkGreen);
    context()->VSSetShader(mVertexShader, nullptr, 0);
    context()->PSSetShader(mPixelShader, nullptr, 0);

    UINT stride = sizeof(vertex_type::TangentVertex);
    UINT offset = 0;

    static ID3D11ShaderResourceView* srvs[] =
    {
        mHdrSrv
    };
    context()->PSSetShaderResources(0, 1, srvs);

    static ID3D11SamplerState* samps[] =
    {
        mLinearWrapSampler,
    };
    context()->PSSetSamplers(0, 1, samps);

    context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context()->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    context()->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    context()->DrawIndexedInstanced(6, 1, 0, 0, 0);

    ID3D11RenderTargetView* rtvnull = nullptr;
    context()->OMSetRenderTargets(1, &rtvnull, nullptr);
    static ID3D11ShaderResourceView* nullsrvs[] =
    {
        nullptr
    };
    context()->PSSetShaderResources(0, 1, nullsrvs);
}

bool RSPass_ToSwapChain::CreateBuffers()
{
    HRESULT hr = S_OK;
    D3D11_BUFFER_DESC bufDesc = {};

    vertex_type::TangentVertex v[4] = {};
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
    bufDesc.ByteWidth = sizeof(vertex_type::TangentVertex) * 4;
    bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufDesc.CPUAccessFlags = 0;
    bufDesc.MiscFlags = 0;
    bufDesc.StructureByteStride = 0;
    D3D11_SUBRESOURCE_DATA vinitData = {};
    ZeroMemory(&vinitData, sizeof(vinitData));
    vinitData.pSysMem = v;
    hr = device()->CreateBuffer(&bufDesc, &vinitData, &mVertexBuffer);
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
    hr = device()->CreateBuffer(&bufDesc, &iinitData, &mIndexBuffer);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_ToSwapChain::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\copy_texture_vertex.hlsl",
        "main", "vs_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateVertexShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mVertexShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\copy_texture_pixel.hlsl",
        "main", "ps_5_0", &shaderBlob);
    if (FAILED(hr)) { return false; }

    hr = device()->CreatePixelShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mPixelShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_ToSwapChain::CreateViews()
{
    mSwapChainRtv = g_Root->getDevices()->getSwapChainRtv();
    mHdrSrv = g_Root->getDevices()->getHighDynamicSrv();

    return true;
}

bool RSPass_ToSwapChain::CreateSamplers()
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
    hr = device()->CreateSamplerState(
        &sampDesc, &mLinearWrapSampler);
    if (FAILED(hr)) { return false; }

    return true;
}

RSPass_FXAA::RSPass_FXAA(
    std::string& _name, PASS_TYPE _type, RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mFXAAShader(nullptr),
    mHdrUav(nullptr),
    mCopyTex(nullptr),
    mCopySrv(nullptr),
    mLinearBorderSampler(nullptr)
{

}

RSPass_FXAA::RSPass_FXAA(const RSPass_FXAA& _source) :
    RSPass_Base(_source),
    mFXAAShader(_source.mFXAAShader),
    mHdrUav(_source.mHdrUav),
    mCopyTex(_source.mCopyTex),
    mCopySrv(_source.mCopySrv),
    mLinearBorderSampler(_source.mLinearBorderSampler)
{
    if (HasBeenInited)
    {
        RS_ADDREF(mFXAAShader);
        RS_ADDREF(mLinearBorderSampler);
        RS_ADDREF(mCopyTex);
        RS_ADDREF(mCopySrv);
    }
}

RSPass_FXAA::~RSPass_FXAA()
{

}

RSPass_FXAA* RSPass_FXAA::clonePass()
{
    return new RSPass_FXAA(*this);
}

bool RSPass_FXAA::initPass()
{
    if (HasBeenInited) { return true; }

    if (!CreateShaders()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    HasBeenInited = true;

    return true;
}

void RSPass_FXAA::releasePass()
{
    RS_RELEASE(mFXAAShader);
    RS_RELEASE(mLinearBorderSampler);
    RS_RELEASE(mCopyTex);
    RS_RELEASE(mCopySrv);
}

void RSPass_FXAA::execuatePass()
{
    UINT dispatchX = g_Root->getDevices()->getCurrWndWidth();
    UINT dispatchY = g_Root->getDevices()->getCurrWndHeight();
    dispatchX = rs_tool::align(dispatchX, 16) / 16;
    dispatchY = rs_tool::align(dispatchY, 16) / 16;

    g_Root->getDevices()->copyHighDynamicTexture(context(), mCopyTex);

    context()->CSSetShader(mFXAAShader, nullptr, 0);
    context()->CSSetSamplers(0, 1, &mLinearBorderSampler);
    context()->CSSetShaderResources(0, 1, &mCopySrv);
    context()->CSSetUnorderedAccessViews(0, 1, &mHdrUav, nullptr);

    context()->Dispatch(dispatchX, dispatchY, 1);

    static ID3D11ShaderResourceView* nullSrv = nullptr;
    static ID3D11UnorderedAccessView* nullUav = nullptr;
    context()->CSSetShaderResources(0, 1, &nullSrv);
    context()->CSSetUnorderedAccessViews(0, 1, &nullUav, nullptr);
}

bool RSPass_FXAA::CreateShaders()
{
    ID3DBlob* shaderBlob = nullptr;
    HRESULT hr = S_OK;

    std::string threshouldStr = "(";
    std::string minThreshouldStr = "(";
    std::string searchStepStr = "(";
    std::string borderGuessStr = "(";
    threshouldStr += std::to_string(g_RenderEffectConfig.mFXAAThreshould) + "f)";
    minThreshouldStr += std::to_string(g_RenderEffectConfig.mFXAAMinThreshould) + "f)";
    searchStepStr += std::to_string(g_RenderEffectConfig.mFXAASearchStep) + ")";
    borderGuessStr += std::to_string(g_RenderEffectConfig.mFXAAGuess) + ")";
    D3D_SHADER_MACRO macro[] =
    {
        { "EDGE_THRESHOLD", threshouldStr.c_str() },
        { "MIN_EDGE_THRESHOLD", minThreshouldStr.c_str() },
        { "EDGE_SEARCH_STEP", searchStepStr.c_str() },
        { "EDGE_GUESS", borderGuessStr.c_str() },
        { nullptr, nullptr }
    };
    hr = rs_tool::compileShaderFromFile(
        L".\\Assets\\Shaders\\fxaa_compute.hlsl",
        "main", "cs_5_0", &shaderBlob, macro);
    if (FAILED(hr)) { return false; }

    hr = device()->CreateComputeShader(
        shaderBlob->GetBufferPointer(),
        shaderBlob->GetBufferSize(),
        nullptr, &mFXAAShader);
    shaderBlob->Release();
    shaderBlob = nullptr;
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_FXAA::CreateViews()
{
    mHdrUav = g_Root->getDevices()->getHighDynamicUav();

    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC texDesc = {};
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    ZeroMemory(&texDesc, sizeof(texDesc));
    ZeroMemory(&srvDesc, sizeof(srvDesc));

    texDesc.Width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
    texDesc.Height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    hr = device()->CreateTexture2D(&texDesc, nullptr, &mCopyTex);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    hr = device()->CreateShaderResourceView(mCopyTex, &srvDesc, &mCopySrv);
    if (FAILED(hr)) { return false; }

    return true;
}

bool RSPass_FXAA::CreateSamplers()
{
    HRESULT hr = S_OK;
    D3D11_SAMPLER_DESC sampDesc = {};
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = device()->CreateSamplerState(&sampDesc, &mLinearBorderSampler);
    if (FAILED(hr)) { return false; }

    return true;
}
