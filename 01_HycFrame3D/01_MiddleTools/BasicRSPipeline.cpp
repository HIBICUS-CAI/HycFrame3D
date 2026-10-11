#include "BasicRSPipeline.h"

#include <FormatUtility.h>
#include <RSCamera.h>
#include <RSCamerasContainer.h>
#include <RSCommandContext.h>
#include <RSDevices.h>
#include <RSDrawCallsPool.h>
#include <RSLight.h>
#include <RSLightsContainer.h>
#include <RSMeshHelper.h>
#include <RSParticleEmitter.h>
#include <RSParticlesContainer.h>
#include <RSPipeline.h>
#include <RSPipelinesManager.h>
#include <RSResourceManager.h>
#include <RSRoot_DX11.h>
#include <RSShaderCompile.h>
#include <RSStaticResources.h>
#include <RSTextureLoader.h>
#include <RSTopic.h>
#include <RSUtilityFunctions.h>
#include <RSVkObjects.h>
#include <TextUtility.h>
#include <WM_Interface.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iterator>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>
#include <string>
#include <vector>

#define RS_RELEASE(PTR)                                                        \
  {                                                                            \
    if (PTR) {                                                                 \
      (PTR)->Release();                                                        \
      (PTR) = nullptr;                                                         \
    }                                                                          \
  }
#define RS_ADDREF(PTR)                                                         \
  {                                                                            \
    if (PTR) {                                                                 \
      PTR->AddRef();                                                           \
    }                                                                          \
  }

static RSRoot_DX11 *G_RSRoot = nullptr;
static RSPass_PriticleSetUp *G_ParticleSetUpPass = nullptr;
static RSPipeline *G_BasicPipeline = nullptr;
static RSPipeline *G_SimplePipeline = nullptr;
static VkViewport G_ViewPort = {};

static float G_DeltaTimeInSecond = 0.f;

static RSShaderResourceView *G_IblBrdfSrv = nullptr;
static RSShaderResourceView *G_EnviMapSrv = nullptr;
static RSShaderResourceView *G_DiffMapSrv = nullptr;
static RSShaderResourceView *G_SpecMapSrv = nullptr;

enum class SAMPLER_LEVEL {
  POINT = 0,
  BILINEAR = 1,
  ANISO_8X = 2,
  ANISO_16X = 3
};

struct RENDER_EFFECT_CONFIG {
  bool SimplyLitOn = true;
  std::string LightModel = "brdf_disney";

  float SsaoRadius = 0.5f;
  float SsaoStart = 0.2f;
  float SsaoEnd = 1.f;
  float SsaoEpsilon = 0.05f;
  UINT SsaoBlurCount = 4;

  SAMPLER_LEVEL SamplerLevel = SAMPLER_LEVEL::ANISO_16X;

  bool ParticleOff = false;

  bool BloomOff = false;
  float BloomMinValue = 2.f;
  UINT BloomDownSamplingCount = 7;
  UINT BloomBlurCount = 2;
  UINT BloomBlurKernel = 5;
  float BloomBlurSigma = 1.f;
  float BloomIntensityFactor = 1.f;
  float BloomLightPixelFactor = 0.02f;

  bool DynamicExpoOff = false;
  float StaticExpo = 0.2f;
  float ExpoTransSpeed = 0.05f;
  float ExpoMin = 0.01f;
  float ExpoMax = 10.f;
  float ExpoInvFactor = 25.f;

  bool FXAAOff = false;
  float FXAAThreshould = 0.125f;
  float FXAAMinThreshould = 0.0625f;
  UINT FXAASearchStep = 10;
  UINT FXAAGuess = 8;
};

static RENDER_EFFECT_CONFIG G_RenderEffectConfig = {};

namespace {

constexpr VkFormat DEPTH_FORMAT = VK_FORMAT_D32_SFLOAT;

RSShader *compileShader(const std::string &FileName,
                        const std::string &EntryPoint,
                        const std::string &ShaderModel,
                        const rs_tool::SHADER_MACRO *Macros = nullptr) {
  std::vector<uint32_t> Spirv = {};
  if (!rs_tool::compileShaderFromFile(FileName, EntryPoint, ShaderModel, Spirv,
                                      Macros)) {
    return nullptr;
  }
  return G_RSRoot->getDevices()->createShader(std::move(Spirv));
}

// a compute shader is held as its pipeline state, the shader module isn't
// needed after the pipeline gets created
RSPipelineState *
compileComputePipeline(const std::string &FileName,
                       const std::string &EntryPoint,
                       const rs_tool::SHADER_MACRO *Macros = nullptr) {
  RSShader *Shader = compileShader(FileName, EntryPoint, "cs_5_0", Macros);
  if (!Shader) {
    return nullptr;
  }
  RSPipelineState *Pipeline =
      G_RSRoot->getDevices()->createComputePipeline(Shader);
  Shader->Release();
  return Pipeline;
}

VkSamplerCreateInfo makeSamplerInfo(VkFilter Filter,
                                    VkSamplerMipmapMode MipMode,
                                    VkSamplerAddressMode Address) {
  VkSamplerCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  Info.magFilter = Filter;
  Info.minFilter = Filter;
  Info.mipmapMode = MipMode;
  Info.addressModeU = Address;
  Info.addressModeV = Address;
  Info.addressModeW = Address;
  Info.compareOp = VK_COMPARE_OP_NEVER;
  Info.minLod = 0.f;
  Info.maxLod = VK_LOD_CLAMP_NONE;
  Info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
  return Info;
}

VkSamplerCreateInfo makePointSamplerInfo(VkSamplerAddressMode Address) {
  return makeSamplerInfo(VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST,
                         Address);
}

VkSamplerCreateInfo makeLinearSamplerInfo(VkSamplerAddressMode Address) {
  return makeSamplerInfo(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR,
                         Address);
}

VkSamplerCreateInfo makeConfigSamplerInfo(VkSamplerAddressMode Address) {
  VkSamplerCreateInfo Info = makeLinearSamplerInfo(Address);
  switch (G_RenderEffectConfig.SamplerLevel) {
  case SAMPLER_LEVEL::POINT:
    Info = makePointSamplerInfo(Address);
    break;
  case SAMPLER_LEVEL::BILINEAR:
    break;
  case SAMPLER_LEVEL::ANISO_8X:
    Info.anisotropyEnable = VK_TRUE;
    Info.maxAnisotropy = 8.f;
    break;
  case SAMPLER_LEVEL::ANISO_16X:
  default:
    Info.anisotropyEnable = VK_TRUE;
    Info.maxAnisotropy = 16.f;
    break;
  }
  return Info;
}

// linear filtering of depth formats is optional in vulkan
VkSamplerCreateInfo fitDepthSamplerInfo(VkSamplerCreateInfo Info) {
  VkFormatProperties Props = {};
  vkGetPhysicalDeviceFormatProperties(
      G_RSRoot->getDevices()->getPhysicalDevice(), DEPTH_FORMAT, &Props);
  if (!(Props.optimalTilingFeatures &
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
    Info.magFilter = VK_FILTER_NEAREST;
    Info.minFilter = VK_FILTER_NEAREST;
    Info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
  }
  return Info;
}

RSTexture *createTexture2D(VkFormat Format,
                           UINT Width,
                           UINT Height,
                           VkImageUsageFlags Usage,
                           UINT MipLevels = 1) {
  RS_TEXTURE_DESC Desc = {};
  Desc.Type = VK_IMAGE_TYPE_2D;
  Desc.Format = Format;
  Desc.Width = Width;
  Desc.Height = Height;
  Desc.MipLevels = MipLevels;
  Desc.Usage = Usage;
  return G_RSRoot->getDevices()->createTexture(Desc);
}

RSBuffer *createDynamicStructuredBuffer(UINT ElementSize, UINT ElementCount) {
  RS_BUFFER_DESC Desc = {};
  Desc.Size = static_cast<VkDeviceSize>(ElementSize) * ElementCount;
  Desc.Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  Desc.Memory = RS_MEMORY_USAGE::DYNAMIC;
  Desc.StructureStride = ElementSize;
  return G_RSRoot->getDevices()->createBuffer(Desc);
}

RSShaderResourceView *createStructuredSrv(RSBuffer *Buffer,
                                          UINT ElementSize,
                                          UINT ElementCount) {
  RS_BUFFER_VIEW_DESC Desc = {};
  Desc.ElementSize = ElementSize;
  Desc.NumElements = ElementCount;
  return G_RSRoot->getDevices()->createShaderResourceView(Buffer, Desc);
}

RSBuffer *createImmutableBuffer(VkBufferUsageFlags Usage,
                                const void *Data,
                                VkDeviceSize Size) {
  RS_BUFFER_DESC Desc = {};
  Desc.Size = Size;
  Desc.Usage = Usage;
  Desc.Memory = RS_MEMORY_USAGE::GPU_ONLY;
  return G_RSRoot->getDevices()->createBuffer(Desc, Data);
}

// two triangles covering the screen, the normal x and tangent x store the
// vertex index for shaders using vertex input
bool createScreenQuad(RSBuffer **OutVertexBuffer, RSBuffer **OutIndexBuffer) {
  vertex_type::TangentVertex V[4] = {};
  V[0].Position = dx::XMFLOAT3(-1.0f, -1.0f, 0.0f);
  V[1].Position = dx::XMFLOAT3(-1.0f, +1.0f, 0.0f);
  V[2].Position = dx::XMFLOAT3(+1.0f, +1.0f, 0.0f);
  V[3].Position = dx::XMFLOAT3(+1.0f, -1.0f, 0.0f);
  V[0].Normal = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);
  V[1].Normal = dx::XMFLOAT3(1.0f, 0.0f, 0.0f);
  V[2].Normal = dx::XMFLOAT3(2.0f, 0.0f, 0.0f);
  V[3].Normal = dx::XMFLOAT3(3.0f, 0.0f, 0.0f);
  V[0].Tangent = dx::XMFLOAT3(0.0f, 0.0f, 0.0f);
  V[1].Tangent = dx::XMFLOAT3(1.0f, 0.0f, 0.0f);
  V[2].Tangent = dx::XMFLOAT3(2.0f, 0.0f, 0.0f);
  V[3].Tangent = dx::XMFLOAT3(3.0f, 0.0f, 0.0f);
  V[0].TexCoord = dx::XMFLOAT2(0.0f, 1.0f);
  V[1].TexCoord = dx::XMFLOAT2(0.0f, 0.0f);
  V[2].TexCoord = dx::XMFLOAT2(1.0f, 0.0f);
  V[3].TexCoord = dx::XMFLOAT2(1.0f, 1.0f);
  *OutVertexBuffer =
      createImmutableBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, V, sizeof(V));
  FAIL_NULL_RETURN(*OutVertexBuffer);

  UINT Indices[6] = {0, 1, 2, 0, 2, 3};
  *OutIndexBuffer = createImmutableBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                          Indices, sizeof(Indices));
  FAIL_NULL_RETURN(*OutIndexBuffer);

  return true;
}

} // namespace

void setPipelineDeltaTime(float DeltaMilliSecond) {
  G_DeltaTimeInSecond = DeltaMilliSecond / 1000.f;
}

void setPipelineIBLTextures(RSShaderResourceView *EnvSrv,
                            RSShaderResourceView *DiffSrv,
                            RSShaderResourceView *SpecSrv) {
  G_EnviMapSrv = EnvSrv;
  G_DiffMapSrv = DiffSrv;
  G_SpecMapSrv = SpecSrv;
}

bool createBasicPipeline() {
  {
    using namespace hyc;
    using namespace hyc::text;
    TomlNode Config = {};
    TomlNode Node = {};
    std::string ErrorMess = "";
    if (!loadTomlAndParse(Config,
                          "./Assets/Configs/render-effect-config.toml",
                          ErrorMess)) {
      return false;
    }

    if (!getTomlNode(Config, "pipeline", Node)) {
      return false;
    }
    G_RenderEffectConfig.SimplyLitOn = getAs<bool>(Node["simply-lit"]);
    G_RenderEffectConfig.LightModel = getAs<std::string>(Node["light-model"]);
    if (G_RenderEffectConfig.LightModel != "brdf_disney" &&
        G_RenderEffectConfig.LightModel != "blinn_phong") {
      G_RenderEffectConfig.LightModel = "brdf_disney";
    }

    if (!getTomlNode(Config, "ssao", Node)) {
      return false;
    }
    G_RenderEffectConfig.SsaoRadius = getAs<float>(Node["radius"]);
    G_RenderEffectConfig.SsaoStart = getAs<float>(Node["range-start"]);
    G_RenderEffectConfig.SsaoEnd = getAs<float>(Node["range-end"]);
    G_RenderEffectConfig.SsaoEpsilon = getAs<float>(Node["epsilon"]);
    G_RenderEffectConfig.SsaoBlurCount = getAs<uint>(Node["blur-count"]);

    if (!getTomlNode(Config, "sampler", Node)) {
      return false;
    }
    G_RenderEffectConfig.SamplerLevel =
        static_cast<SAMPLER_LEVEL>(getAs<uint>(Node["filter-level"]));
    if (static_cast<UINT>(G_RenderEffectConfig.SamplerLevel) < 0 ||
        static_cast<UINT>(G_RenderEffectConfig.SamplerLevel) > 3) {
      return false;
    }

    if (!getTomlNode(Config, "particle", Node)) {
      return false;
    }
    G_RenderEffectConfig.ParticleOff = getAs<bool>(Node["disable-particle"]);

    if (!getTomlNode(Config, "bloom", Node)) {
      return false;
    }
    G_RenderEffectConfig.BloomOff = getAs<bool>(Node["disable-bloom"]);
    if (G_RenderEffectConfig.LightModel == "blinn_phong") {
      G_RenderEffectConfig.BloomOff = true;
    }
    G_RenderEffectConfig.BloomMinValue = getAs<float>(Node["min-luminous"]);
    G_RenderEffectConfig.BloomDownSamplingCount =
        getAs<uint>(Node["downsampling-count"]);
    G_RenderEffectConfig.BloomBlurCount =
        getAs<uint>(Node["downsampling-blur-count"]);
    G_RenderEffectConfig.BloomBlurKernel =
        getAs<uint>(Node["gauss-kernel-size"]);
    G_RenderEffectConfig.BloomBlurSigma = getAs<float>(Node["gauss-sigma"]);
    G_RenderEffectConfig.BloomIntensityFactor =
        getAs<float>(Node["intensity-factor"]);
    G_RenderEffectConfig.BloomLightPixelFactor =
        getAs<float>(Node["light-source-factor"]);

    if (!getTomlNode(Config, "exposure", Node)) {
      return false;
    }
    G_RenderEffectConfig.DynamicExpoOff = getAs<bool>(Node["disable-dynamic"]);
    G_RenderEffectConfig.StaticExpo = getAs<float>(Node["static-exposure"]);
    G_RenderEffectConfig.ExpoTransSpeed = getAs<float>(Node["trans-speed"]);
    G_RenderEffectConfig.ExpoMin = getAs<float>(Node["min-value"]);
    G_RenderEffectConfig.ExpoMax = getAs<float>(Node["max-value"]);
    G_RenderEffectConfig.ExpoInvFactor = getAs<float>(Node["inverse-factor"]);

    if (!getTomlNode(Config, "fxaa", Node)) {
      return false;
    }
    G_RenderEffectConfig.FXAAOff = getAs<bool>(Node["disable-fxaa"]);
    G_RenderEffectConfig.FXAAThreshould = getAs<float>(Node["threshold"]);
    G_RenderEffectConfig.FXAAMinThreshould =
        getAs<float>(Node["min-threshold"]);
    G_RenderEffectConfig.FXAASearchStep = getAs<uint>(Node["search-step"]);
    G_RenderEffectConfig.FXAAGuess = getAs<uint>(Node["edge-guess"]);
    if (!G_RenderEffectConfig.FXAASearchStep) {
      G_RenderEffectConfig.FXAAOff = true;
    }
  }

  G_RSRoot = getRSDX11RootInstance();
  std::string Name = "";

  Name = "mrt-pass";
  RSPass_MRT *MrtPass = new RSPass_MRT(Name, PASS_TYPE::RENDER, G_RSRoot);
  MrtPass->setExecuateOrder(1);

  Name = "mrt-topic";
  RSTopic *MrtTopic = new RSTopic(Name);
  MrtTopic->startAssembly();
  MrtTopic->insertPass(MrtPass);
  MrtTopic->setExecuateOrder(1);
  MrtTopic->finishAssembly();

  Name = "basic-ssao";
  RSPass_Ssao *SsaoPass = new RSPass_Ssao(Name, PASS_TYPE::RENDER, G_RSRoot);
  SsaoPass->setExecuateOrder(2);

  Name = "kbblur-ssao";
  RSPass_KBBlur *KBBlurPass =
      new RSPass_KBBlur(Name, PASS_TYPE::COMPUTE, G_RSRoot);
  KBBlurPass->setExecuateOrder(3);

  Name = "ssao-topic";
  RSTopic *SsaoTopic = new RSTopic(Name);
  SsaoTopic->startAssembly();
  SsaoTopic->insertPass(SsaoPass);
  SsaoTopic->insertPass(KBBlurPass);
  SsaoTopic->setExecuateOrder(2);
  SsaoTopic->finishAssembly();

  Name = "basic-shadowmap";
  RSPass_Shadow *ShadowPass =
      new RSPass_Shadow(Name, PASS_TYPE::RENDER, G_RSRoot);
  ShadowPass->setExecuateOrder(1);

  Name = "shadowmap-topic";
  RSTopic *ShadowTopic = new RSTopic(Name);
  ShadowTopic->startAssembly();
  ShadowTopic->insertPass(ShadowPass);
  ShadowTopic->setExecuateOrder(3);
  ShadowTopic->finishAssembly();

  Name = "defered-light";
  RSPass_Defered *DeferedPass =
      new RSPass_Defered(Name, PASS_TYPE::RENDER, G_RSRoot);
  DeferedPass->setExecuateOrder(1);

  Name = "defered-light-topic";
  RSTopic *DeferedTopic = new RSTopic(Name);
  DeferedTopic->startAssembly();
  DeferedTopic->insertPass(DeferedPass);
  DeferedTopic->setExecuateOrder(4);
  DeferedTopic->finishAssembly();

  Name = "sky-skysphere";
  RSPass_SkyShpere *SkySpherePass =
      new RSPass_SkyShpere(Name, PASS_TYPE::RENDER, G_RSRoot);
  SkySpherePass->setExecuateOrder(1);

  Name = "skysphere-topic";
  RSTopic *SkyTopic = new RSTopic(Name);
  SkyTopic->startAssembly();
  SkyTopic->insertPass(SkySpherePass);
  SkyTopic->setExecuateOrder(5);
  SkyTopic->finishAssembly();

  Name = "billboard-pass";
  RSPass_Billboard *BillboardPass =
      new RSPass_Billboard(Name, PASS_TYPE::RENDER, G_RSRoot);
  BillboardPass->setExecuateOrder(1);

  Name = "billboard-topic";
  RSTopic *BillboardTopic = new RSTopic(Name);
  BillboardTopic->startAssembly();
  BillboardTopic->insertPass(BillboardPass);
  BillboardTopic->setExecuateOrder(6);
  BillboardTopic->finishAssembly();

  Name = "bloomdraw-pass";
  RSPass_Bloom *BloomDrawPass =
      new RSPass_Bloom(Name, PASS_TYPE::RENDER, G_RSRoot);
  BloomDrawPass->setExecuateOrder(1);

  Name = "bloom-topic";
  RSTopic *BloomTopic = new RSTopic(Name);
  BloomTopic->startAssembly();
  BloomTopic->insertPass(BloomDrawPass);
  BloomTopic->setExecuateOrder(7);
  BloomTopic->finishAssembly();

  RSTopic *ParticleTopic = nullptr;
  if (!G_RenderEffectConfig.ParticleOff) {
    Name = "particle-setup-pass";
    RSPass_PriticleSetUp *PtcSetupPass =
        new RSPass_PriticleSetUp(Name, PASS_TYPE::COMPUTE, G_RSRoot);
    PtcSetupPass->setExecuateOrder(1);

    Name = "particle-emit-simulate-pass";
    RSPass_PriticleEmitSimulate *PtcEmitSimulatePass =
        new RSPass_PriticleEmitSimulate(Name, PASS_TYPE::COMPUTE, G_RSRoot);
    PtcEmitSimulatePass->setExecuateOrder(2);

    Name = "particle-tile-render-pass";
    RSPass_PriticleTileRender *PtcTilePass =
        new RSPass_PriticleTileRender(Name, PASS_TYPE::COMPUTE, G_RSRoot);
    PtcTilePass->setExecuateOrder(3);

    Name = "paricle-topic";
    ParticleTopic = new RSTopic(Name);
    ParticleTopic->startAssembly();
    ParticleTopic->insertPass(PtcSetupPass);
    ParticleTopic->insertPass(PtcEmitSimulatePass);
    ParticleTopic->insertPass(PtcTilePass);
    ParticleTopic->setExecuateOrder(8);
    ParticleTopic->finishAssembly();
  }

  Name = "sprite-ui";
  RSPass_Sprite *SpritePass =
      new RSPass_Sprite(Name, PASS_TYPE::RENDER, G_RSRoot);
  SpritePass->setExecuateOrder(1);

  Name = "sprite-topic";
  RSTopic *SpriteTopic = new RSTopic(Name);
  SpriteTopic->startAssembly();
  SpriteTopic->insertPass(SpritePass);
  SpriteTopic->setExecuateOrder(10);
  SpriteTopic->finishAssembly();

  Name = "tonemapping-pass";
  RSPass_Tonemapping *TonemapPass =
      new RSPass_Tonemapping(Name, PASS_TYPE::COMPUTE, G_RSRoot);
  TonemapPass->setExecuateOrder(2);

  RSPass_BloomHdr *HdrBloomPass = nullptr;
  if (!G_RenderEffectConfig.BloomOff) {
    Name = "bloom-hdr-pass";
    HdrBloomPass = new RSPass_BloomHdr(Name, PASS_TYPE::COMPUTE, G_RSRoot);
    HdrBloomPass->setExecuateOrder(1);
  }

  Name = "fxaa-pass";
  RSPass_FXAA *FxaaPass = new RSPass_FXAA(Name, PASS_TYPE::COMPUTE, G_RSRoot);
  FxaaPass->setExecuateOrder(3);

  Name = "to-swapchain-pass";
  RSPass_ToSwapChain *ToSwapPass =
      new RSPass_ToSwapChain(Name, PASS_TYPE::RENDER, G_RSRoot);
  ToSwapPass->setExecuateOrder(4);

  Name = "post-processing-topic";
  RSTopic *PostProcsssingTopic = new RSTopic(Name);
  PostProcsssingTopic->startAssembly();
  PostProcsssingTopic->insertPass(TonemapPass);
  if (!G_RenderEffectConfig.FXAAOff) {
    PostProcsssingTopic->insertPass(FxaaPass);
  } else {
    FxaaPass->releasePass();
    delete FxaaPass;
  }
  PostProcsssingTopic->insertPass(ToSwapPass);
  if (!G_RenderEffectConfig.BloomOff) {
    PostProcsssingTopic->insertPass(HdrBloomPass);
  }
  PostProcsssingTopic->setExecuateOrder(9);
  PostProcsssingTopic->finishAssembly();

  Name = "light-pipeline";
  G_BasicPipeline = new RSPipeline(Name);
  G_BasicPipeline->startAssembly();
  G_BasicPipeline->insertTopic(MrtTopic);
  G_BasicPipeline->insertTopic(SsaoTopic);
  G_BasicPipeline->insertTopic(ShadowTopic);
  G_BasicPipeline->insertTopic(DeferedTopic);
  G_BasicPipeline->insertTopic(SkyTopic);
  G_BasicPipeline->insertTopic(BloomTopic);
  G_BasicPipeline->insertTopic(BillboardTopic);
  if (!G_RenderEffectConfig.ParticleOff) {
    G_BasicPipeline->insertTopic(ParticleTopic);
  }
  G_BasicPipeline->insertTopic(PostProcsssingTopic);
  G_BasicPipeline->insertTopic(SpriteTopic);
  G_BasicPipeline->finishAssembly();

  if (!G_BasicPipeline->initAllTopics(G_RSRoot->getDevices())) {
    return false;
  }

  Name = G_BasicPipeline->getPipelineName();
  G_RSRoot->getPipelinesManager()->addPipeline(Name, G_BasicPipeline);
  G_RSRoot->getPipelinesManager()->setPipeline(Name);
  G_RSRoot->getPipelinesManager()->changeToNextPipeline();

  Name = "simp-mrt-pass";
  RSPass_MRT *SimpMrtPass = MrtPass->clonePass();
  SimpMrtPass->setExecuateOrder(1);

  Name = "simp-mrt-topic";
  RSTopic *SimpMrtTopic = new RSTopic(Name);
  SimpMrtTopic->startAssembly();
  SimpMrtTopic->insertPass(SimpMrtPass);
  SimpMrtTopic->setExecuateOrder(1);
  SimpMrtTopic->finishAssembly();

  Name = "simp-basic-ssao";
  RSPass_Ssao *SimpSsaoPass = SsaoPass->clonePass();
  SimpSsaoPass->setExecuateOrder(1);

  Name = "simp-ssao-topic";
  RSTopic *SimpSsaoTopic = new RSTopic(Name);
  SimpSsaoTopic->startAssembly();
  SimpSsaoTopic->insertPass(SimpSsaoPass);
  SimpSsaoTopic->setExecuateOrder(2);
  SimpSsaoTopic->finishAssembly();

  Name = "simp-lit-shadowmap";
  RSPass_SimpleLight *SimpLitPass =
      new RSPass_SimpleLight(Name, PASS_TYPE::RENDER, G_RSRoot);
  SimpLitPass->setExecuateOrder(1);

  Name = "simp-lit-topic";
  RSTopic *SimpLitTopic = new RSTopic(Name);
  SimpLitTopic->startAssembly();
  SimpLitTopic->insertPass(SimpLitPass);
  SimpLitTopic->setExecuateOrder(3);
  SimpLitTopic->finishAssembly();

  Name = "simp-billboard";
  RSPass_Billboard *SimpBillPass = BillboardPass->clonePass();
  SimpBillPass->setExecuateOrder(1);

  Name = "simp-billboard-topic";
  RSTopic *SimpBillTopic = new RSTopic(Name);
  SimpBillTopic->startAssembly();
  SimpBillTopic->insertPass(SimpBillPass);
  SimpBillTopic->setExecuateOrder(4);
  SimpBillTopic->finishAssembly();

  Name = "simp-sprite-ui";
  RSPass_Sprite *SimpSpritePass = SpritePass->clonePass();
  SimpSpritePass->setExecuateOrder(1);

  Name = "simp-sprite-topic";
  RSTopic *SimpSpriteTopic = new RSTopic(Name);
  SimpSpriteTopic->startAssembly();
  SimpSpriteTopic->insertPass(SimpSpritePass);
  SimpSpriteTopic->setExecuateOrder(5);
  SimpSpriteTopic->finishAssembly();

  Name = "simple-pipeline";
  G_SimplePipeline = new RSPipeline(Name);
  G_SimplePipeline->startAssembly();
  G_SimplePipeline->insertTopic(SimpMrtTopic);
  G_SimplePipeline->insertTopic(SimpSsaoTopic);
  G_SimplePipeline->insertTopic(SimpLitTopic);
  G_SimplePipeline->insertTopic(SimpBillTopic);
  G_SimplePipeline->insertTopic(SimpSpriteTopic);
  G_SimplePipeline->finishAssembly();

  if (!G_SimplePipeline->initAllTopics(G_RSRoot->getDevices())) {
    return false;
  }

  Name = G_SimplePipeline->getPipelineName();
  G_RSRoot->getPipelinesManager()->addPipeline(Name, G_SimplePipeline);
  if (G_RenderEffectConfig.SimplyLitOn) {
    G_RSRoot->getPipelinesManager()->setPipeline(Name);
    G_RSRoot->getPipelinesManager()->changeToNextPipeline();
  }

  G_ViewPort.width =
      static_cast<float>(G_RSRoot->getDevices()->getCurrWndWidth());
  G_ViewPort.height =
      static_cast<float>(G_RSRoot->getDevices()->getCurrWndHeight());
  G_ViewPort.minDepth = 0.f;
  G_ViewPort.maxDepth = 1.f;
  G_ViewPort.x = 0.f;
  G_ViewPort.y = 0.f;

  return true;
}

RSPass_MRT::RSPass_MRT(std::string &Name, PASS_TYPE Type, RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      AniVertexShader(nullptr), PixelShader(nullptr), NDPixelShader(nullptr),
      StaticPipeline(nullptr), AniPipeline(nullptr),
      DrawCallType(DRAWCALL_TYPE::OPACITY), DrawCallPipe(nullptr),
      ViewProjStructedBuffer(nullptr), ViewProjStructedBufferSrv(nullptr),
      InstanceStructedBuffer(nullptr), InstanceStructedBufferSrv(nullptr),
      BonesStructedBuffer(nullptr), BonesStructedBufferSrv(nullptr),
      LinearSampler(nullptr), GeoBufferRtv(nullptr), AnisotropicRtv(nullptr),
      DepthDsv(nullptr), RSCameraInfo(nullptr) {}

RSPass_MRT::RSPass_MRT(const RSPass_MRT &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      AniVertexShader(Source.AniVertexShader), PixelShader(Source.PixelShader),
      NDPixelShader(Source.NDPixelShader),
      StaticPipeline(Source.StaticPipeline), AniPipeline(Source.AniPipeline),
      DrawCallType(Source.DrawCallType), DrawCallPipe(Source.DrawCallPipe),
      ViewProjStructedBuffer(Source.ViewProjStructedBuffer),
      ViewProjStructedBufferSrv(Source.ViewProjStructedBufferSrv),
      InstanceStructedBuffer(Source.InstanceStructedBuffer),
      InstanceStructedBufferSrv(Source.InstanceStructedBufferSrv),
      BonesStructedBuffer(Source.BonesStructedBuffer),
      BonesStructedBufferSrv(Source.BonesStructedBufferSrv),
      LinearSampler(Source.LinearSampler), GeoBufferRtv(Source.GeoBufferRtv),
      AnisotropicRtv(Source.AnisotropicRtv), DepthDsv(Source.DepthDsv),
      RSCameraInfo(Source.RSCameraInfo) {
  if (HasBeenInited) {
    RS_ADDREF(VertexShader);
    RS_ADDREF(AniVertexShader);
    RS_ADDREF(PixelShader);
    RS_ADDREF(NDPixelShader);
    RS_ADDREF(StaticPipeline);
    RS_ADDREF(AniPipeline);
    RS_ADDREF(ViewProjStructedBuffer);
    RS_ADDREF(ViewProjStructedBufferSrv);
    RS_ADDREF(InstanceStructedBuffer);
    RS_ADDREF(InstanceStructedBufferSrv);
    RS_ADDREF(BonesStructedBuffer);
    RS_ADDREF(BonesStructedBufferSrv);
    RS_ADDREF(LinearSampler);
  }
}

RSPass_MRT::~RSPass_MRT() {}

RSPass_MRT *RSPass_MRT::clonePass() { return new RSPass_MRT(*this); }

bool RSPass_MRT::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }
  if (!createPipelines()) {
    return false;
  }

  DrawCallType = DRAWCALL_TYPE::OPACITY;
  DrawCallPipe = G_RSRoot->getDrawCallsPool()->getDrawCallsPipe(DrawCallType);

  RSCameraInfo = G_RSRoot->getCamerasContainer()->getRSCameraInfo("temp-cam");

  HasBeenInited = true;

  return true;
}

void RSPass_MRT::releasePass() {
  RS_RELEASE(VertexShader);
  RS_RELEASE(AniVertexShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(NDPixelShader);
  RS_RELEASE(StaticPipeline);
  RS_RELEASE(AniPipeline);
  RS_RELEASE(ViewProjStructedBuffer);
  RS_RELEASE(ViewProjStructedBufferSrv);
  RS_RELEASE(InstanceStructedBuffer);
  RS_RELEASE(InstanceStructedBufferSrv);
  RS_RELEASE(BonesStructedBuffer);
  RS_RELEASE(BonesStructedBufferSrv);
  RS_RELEASE(LinearSampler);

  G_RSRoot->getResourceManager()->deleteResource("mrt-depth");
  G_RSRoot->getResourceManager()->deleteResource("mrt-geo-buffer");
  G_RSRoot->getResourceManager()->deleteResource("mrt-anisotropic");
}

void RSPass_MRT::execuatePass() {
  RSRenderTargetView *MRtv[] = {GeoBufferRtv, AnisotropicRtv};
  context()->setRenderTargets(2, MRtv, DepthDsv);
  context()->setViewport(G_ViewPort);
  context()->clearRenderTargetView(GeoBufferRtv, dx::Colors::Transparent);
  context()->clearRenderTargetView(AnisotropicRtv, dx::Colors::Transparent);
  context()->clearDepthStencilView(DepthDsv, 1.f);

  context()->setSamplers(RS_SHADER_STAGE::PIXEL, 0, 1, &LinearSampler);

  dx::XMMATRIX Mat = {};

  ViewProj *VpData = static_cast<ViewProj *>(ViewProjStructedBuffer->map());
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ViewMat, Mat);
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ProjMat, Mat);
  ViewProjStructedBuffer->unmap();

  context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 0, 1,
                                &ViewProjStructedBufferSrv);

  static std::string A_NAME = "AnimationVertex";
  static const auto ANIMAT_LAYOUT =
      getRSDX11RootInstance()->getStaticResources()->getStaticInputLayout(
          A_NAME);

  for (auto &Draw : DrawCallPipe->Data) {
    if (Draw.MeshData.InputLayout == ANIMAT_LAYOUT) {
      context()->setPipelineState(AniPipeline);
      context()->setVertexBuffer(Draw.MeshData.VertexBuffer);

      dx::XMFLOAT4X4 *BData =
          static_cast<dx::XMFLOAT4X4 *>(BonesStructedBuffer->map());
      void *RawBoneData = Draw.InstanceData.BonesArrayPtr;
      std::vector<std::vector<RS_SUBMESH_BONE_DATA>> *BonesPtr = nullptr;
      BonesPtr = static_cast<decltype(BonesPtr)>(RawBoneData);
      // TEMP-----------------------
      auto BoneInsSize = BonesPtr->size();
      // TEMP-----------------------
      for (size_t I = 0; I < BoneInsSize; I++) {
        for (size_t J = 0; J < MAX_STRUCTURED_BUFFER_SIZE; J++) {
          if (J < (*BonesPtr)[I].size()) {
            dx::XMMATRIX Trans =
                dx::XMLoadFloat4x4(&((*BonesPtr)[I][J].BoneTransform));
            Trans = dx::XMMatrixTranspose(Trans);
            dx::XMStoreFloat4x4(BData + I * MAX_STRUCTURED_BUFFER_SIZE + J,
                                Trans);
          } else {
            dx::XMStoreFloat4x4(BData + I * MAX_STRUCTURED_BUFFER_SIZE + J,
                                dx::XMMatrixIdentity());
          }
        }
      }
      BonesStructedBuffer->unmap(BoneInsSize * MAX_STRUCTURED_BUFFER_SIZE *
                                 sizeof(dx::XMFLOAT4X4));

      context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 2, 1,
                                    &BonesStructedBufferSrv);
    } else {
      context()->setPipelineState(StaticPipeline);
      context()->setVertexBuffer(Draw.MeshData.VertexBuffer);
    }

    auto InsVecPtr = Draw.InstanceData.DataArrayPtr;
    auto Size = InsVecPtr->size();
    RS_INSTANCE_DATA *InsDataPtr =
        static_cast<RS_INSTANCE_DATA *>(InstanceStructedBuffer->map());
    for (size_t I = 0; I < Size; I++) {
      Mat = dx::XMLoadFloat4x4(&(*InsVecPtr)[I].WorldMatrix);
      Mat = dx::XMMatrixTranspose(Mat);
      dx::XMStoreFloat4x4(&InsDataPtr[I].WorldMatrix, Mat);
      InsDataPtr[I].MaterialData = (*InsVecPtr)[I].MaterialData;
      InsDataPtr[I].CustomizedData1 = (*InsVecPtr)[I].CustomizedData1;
      InsDataPtr[I].CustomizedData2 = (*InsVecPtr)[I].CustomizedData2;
    }
    InstanceStructedBuffer->unmap(Size * sizeof(RS_INSTANCE_DATA));

    context()->setPrimitiveTopology(Draw.MeshData.TopologyType);
    context()->setIndexBuffer(Draw.MeshData.IndexBuffer);
    context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 1, 1,
                                  &InstanceStructedBufferSrv);
    RSShaderResourceView *MatSrv =
        G_RSRoot->getStaticResources()->getMaterialSrv();
    context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 0, 1, &MatSrv);
    for (UINT I = 0, E = static_cast<UINT>(MESH_TEXTURE_TYPE::SIZE); I < E;
         I++) {
      if (Draw.TextureData[I].EnabledFlag) {
        context()->setShaderResources(RS_SHADER_STAGE::PIXEL, I + 1, 1,
                                      &(Draw.TextureData[I].Srv));
      }
    }

    context()->drawIndexedInstanced(
        Draw.MeshData.IndexSize,
        static_cast<UINT>(Draw.InstanceData.DataArrayPtr->size()), 0, 0, 0);

    if (Draw.MeshData.InputLayout == ANIMAT_LAYOUT) {
      RSShaderResourceView *NullSrv = nullptr;
      context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 2, 1, &NullSrv);
    }
  }

  context()->setRenderTargets(0, nullptr, nullptr);
}

bool RSPass_MRT::createShaders() {
  VertexShader =
      compileShader("./Assets/Shaders/mrt_vertex.hlsl", "main", "vs_5_0");
  FAIL_NULL_RETURN(VertexShader);

  rs_tool::SHADER_MACRO Macro[] = {{"ANIMATION_VERTEX", "1"},
                                   {nullptr, nullptr}};
  AniVertexShader = compileShader("./Assets/Shaders/mrt_vertex.hlsl", "main",
                                  "vs_5_0", Macro);
  FAIL_NULL_RETURN(AniVertexShader);

  PixelShader =
      compileShader("./Assets/Shaders/mrt_pixel.hlsl", "main", "ps_5_0");
  FAIL_NULL_RETURN(PixelShader);

  NDPixelShader =
      compileShader("./Assets/Shaders/mrt_nd_pixel.hlsl", "main", "ps_5_0");
  FAIL_NULL_RETURN(NDPixelShader);

  return true;
}

bool RSPass_MRT::createBuffers() {
  InstanceStructedBuffer = createDynamicStructuredBuffer(
      sizeof(RS_INSTANCE_DATA), MAX_INSTANCE_SIZE);
  FAIL_NULL_RETURN(InstanceStructedBuffer);

  BonesStructedBuffer = createDynamicStructuredBuffer(
      sizeof(dx::XMFLOAT4X4),
      MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE);
  FAIL_NULL_RETURN(BonesStructedBuffer);

  ViewProjStructedBuffer = createDynamicStructuredBuffer(sizeof(ViewProj), 1);
  FAIL_NULL_RETURN(ViewProjStructedBuffer);

  return true;
}

bool RSPass_MRT::createViews() {
  InstanceStructedBufferSrv = createStructuredSrv(
      InstanceStructedBuffer, sizeof(RS_INSTANCE_DATA), MAX_INSTANCE_SIZE);
  FAIL_NULL_RETURN(InstanceStructedBufferSrv);

  BonesStructedBufferSrv = createStructuredSrv(
      BonesStructedBuffer, sizeof(dx::XMFLOAT4X4),
      MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE);
  FAIL_NULL_RETURN(BonesStructedBufferSrv);

  ViewProjStructedBufferSrv =
      createStructuredSrv(ViewProjStructedBuffer, sizeof(ViewProj), 1);
  FAIL_NULL_RETURN(ViewProjStructedBufferSrv);

  auto Devices = G_RSRoot->getDevices();
  UINT Width = Devices->getCurrWndWidth();
  UINT Height = Devices->getCurrWndHeight();
  RS_RESOURCE_INFO RDI = {};

  // d24s8 isn't required by vulkan, the stencil isn't used anyway
  RSTexture *Texture = createTexture2D(
      DEPTH_FORMAT, Width, Height,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
  FAIL_NULL_RETURN(Texture);
  DepthDsv = Devices->createDepthStencilView(Texture);
  FAIL_NULL_RETURN(DepthDsv);
  RSShaderResourceView *Srv = Devices->createShaderResourceView(Texture);
  FAIL_NULL_RETURN(Srv);

  RDI = {};
  RDI.Type = RS_RESOURCE_TYPE::TEXTURE2D;
  RDI.Resource.Texture2D = Texture;
  RDI.Dsv = DepthDsv;
  RDI.Srv = Srv;
  G_RSRoot->getResourceManager()->addResource("mrt-depth", RDI);

  Texture = createTexture2D(
      VK_FORMAT_R32G32B32A32_UINT, Width, Height,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
  FAIL_NULL_RETURN(Texture);
  GeoBufferRtv = Devices->createRenderTargetView(Texture);
  FAIL_NULL_RETURN(GeoBufferRtv);
  Srv = Devices->createShaderResourceView(Texture);
  FAIL_NULL_RETURN(Srv);

  RDI = {};
  RDI.Type = RS_RESOURCE_TYPE::TEXTURE2D;
  RDI.Resource.Texture2D = Texture;
  RDI.Rtv = GeoBufferRtv;
  RDI.Srv = Srv;
  G_RSRoot->getResourceManager()->addResource("mrt-geo-buffer", RDI);

  Texture = createTexture2D(
      VK_FORMAT_R16G16B16A16_SFLOAT, Width, Height,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
  FAIL_NULL_RETURN(Texture);
  AnisotropicRtv = Devices->createRenderTargetView(Texture);
  FAIL_NULL_RETURN(AnisotropicRtv);
  Srv = Devices->createShaderResourceView(Texture);
  FAIL_NULL_RETURN(Srv);

  RDI = {};
  RDI.Type = RS_RESOURCE_TYPE::TEXTURE2D;
  RDI.Resource.Texture2D = Texture;
  RDI.Rtv = AnisotropicRtv;
  RDI.Srv = Srv;
  G_RSRoot->getResourceManager()->addResource("mrt-anisotropic", RDI);

  return true;
}

bool RSPass_MRT::createSamplers() {
  LinearSampler = G_RSRoot->getDevices()->createSampler(
      makeConfigSamplerInfo(VK_SAMPLER_ADDRESS_MODE_REPEAT));
  FAIL_NULL_RETURN(LinearSampler);

  return true;
}

bool RSPass_MRT::createPipelines() {
  auto Static = G_RSRoot->getStaticResources();

  RS_GRAPHICS_PIPELINE_DESC Desc = {};
  Desc.VertexShader = VertexShader;
  Desc.PixelShader = PixelShader;
  Desc.InputLayout = Static->getStaticInputLayout("TangentVertex");
  Desc.ColorTargetCount = 2;
  Desc.ColorFormats[0] = VK_FORMAT_R32G32B32A32_UINT;
  Desc.ColorFormats[1] = VK_FORMAT_R16G16B16A16_SFLOAT;
  Desc.DepthFormat = DEPTH_FORMAT;
  StaticPipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(StaticPipeline);

  Desc.VertexShader = AniVertexShader;
  Desc.InputLayout = Static->getStaticInputLayout("AnimationVertex");
  AniPipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(AniPipeline);

  return true;
}

RSPass_Ssao::RSPass_Ssao(std::string &Name, PASS_TYPE Type, RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      PixelShader(nullptr), CompressVertexShader(nullptr),
      CompressPixelShader(nullptr), SsaoPipeline(nullptr),
      CompressPipeline(nullptr), RenderTargetView(nullptr),
      NotCompressSrv(nullptr), CompressRtv(nullptr), SamplePointClamp(nullptr),
      SampleLinearClamp(nullptr), SampleDepthMap(nullptr),
      SampleLinearWrap(nullptr), SsaoInfoStructedBuffer(nullptr),
      SsaoInfoStructedBufferSrv(nullptr), GeoBufferSrv(nullptr),
      DepthMapSrv(nullptr), RandomMapSrv(nullptr), OffsetVec({{}}),
      VertexBuffer(nullptr), IndexBuffer(nullptr), RSCameraInfo(nullptr) {}

RSPass_Ssao::RSPass_Ssao(const RSPass_Ssao &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      PixelShader(Source.PixelShader),
      CompressVertexShader(Source.CompressVertexShader),
      CompressPixelShader(Source.CompressPixelShader),
      SsaoPipeline(Source.SsaoPipeline),
      CompressPipeline(Source.CompressPipeline),
      RenderTargetView(Source.RenderTargetView),
      NotCompressSrv(Source.NotCompressSrv), CompressRtv(Source.CompressRtv),
      SamplePointClamp(Source.SamplePointClamp),
      SampleLinearClamp(Source.SampleLinearClamp),
      SampleDepthMap(Source.SampleDepthMap),
      SampleLinearWrap(Source.SampleLinearWrap),
      SsaoInfoStructedBuffer(Source.SsaoInfoStructedBuffer),
      SsaoInfoStructedBufferSrv(Source.SsaoInfoStructedBufferSrv),
      GeoBufferSrv(Source.GeoBufferSrv), DepthMapSrv(Source.DepthMapSrv),
      RandomMapSrv(Source.RandomMapSrv), OffsetVec(Source.OffsetVec),
      VertexBuffer(Source.VertexBuffer), IndexBuffer(Source.IndexBuffer),
      RSCameraInfo(Source.RSCameraInfo) {
  if (HasBeenInited) {
    RS_ADDREF(VertexShader);
    RS_ADDREF(PixelShader);
    RS_ADDREF(CompressVertexShader);
    RS_ADDREF(CompressPixelShader);
    RS_ADDREF(SsaoPipeline);
    RS_ADDREF(CompressPipeline);
    RS_ADDREF(SamplePointClamp);
    RS_ADDREF(SampleLinearClamp);
    RS_ADDREF(SampleDepthMap);
    RS_ADDREF(SampleLinearWrap);
    RS_ADDREF(SsaoInfoStructedBuffer);
    RS_ADDREF(SsaoInfoStructedBufferSrv);
    RS_ADDREF(VertexBuffer);
    RS_ADDREF(IndexBuffer);
  }
}

RSPass_Ssao::~RSPass_Ssao() {}

RSPass_Ssao *RSPass_Ssao::clonePass() { return new RSPass_Ssao(*this); }

bool RSPass_Ssao::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createTextures()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }
  if (!createPipelines()) {
    return false;
  }

  dx::XMFLOAT4 Vector[14] = {};
  Vector[0] = dx::XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
  Vector[1] = dx::XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);
  Vector[2] = dx::XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
  Vector[3] = dx::XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);
  Vector[4] = dx::XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
  Vector[5] = dx::XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);
  Vector[6] = dx::XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
  Vector[7] = dx::XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);
  Vector[8] = dx::XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
  Vector[9] = dx::XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);
  Vector[10] = dx::XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
  Vector[11] = dx::XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);
  Vector[12] = dx::XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
  Vector[13] = dx::XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);
  const int Basic = 25;
  const int Range = 75;
  for (int I = 0; I < 14; I++) {
    std::srand(static_cast<UINT>(std::time(nullptr)) +
               static_cast<UINT>(std::rand()));
    float S = static_cast<float>(std::rand() % Range + Basic) / 100.f;
    dx::XMVECTOR V = dx::XMVector4Normalize(dx::XMLoadFloat4(&Vector[I]));
    dx::XMStoreFloat4(&Vector[I], V);
    Vector[I].x *= S;
    Vector[I].y *= S;
    Vector[I].z *= S;
    OffsetVec[I] = Vector[I];
  }

  RSCameraInfo = G_RSRoot->getCamerasContainer()->getRSCameraInfo("temp-cam");

  HasBeenInited = true;

  return true;
}

void RSPass_Ssao::releasePass() {
  RS_RELEASE(VertexShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(CompressVertexShader);
  RS_RELEASE(CompressPixelShader);
  RS_RELEASE(SsaoPipeline);
  RS_RELEASE(CompressPipeline);
  RS_RELEASE(SamplePointClamp);
  RS_RELEASE(SampleLinearClamp);
  RS_RELEASE(SampleDepthMap);
  RS_RELEASE(SampleLinearWrap);
  RS_RELEASE(SsaoInfoStructedBuffer);
  RS_RELEASE(SsaoInfoStructedBufferSrv);
  RS_RELEASE(VertexBuffer);
  RS_RELEASE(IndexBuffer);

  G_RSRoot->getResourceManager()->deleteResource("random-tex-ssao");
  G_RSRoot->getResourceManager()->deleteResource("ssao-tex-ssao");
  G_RSRoot->getResourceManager()->deleteResource("ssao-tex-compress-ssao");
}

void RSPass_Ssao::execuatePass() {
  RSShaderResourceView *NullSrv = nullptr;
  context()->setRenderTargets(1, &RenderTargetView, nullptr);
  context()->setViewport(G_ViewPort);
  context()->setPipelineState(SsaoPipeline);

  dx::XMMATRIX Mat = {};

  SsaoInfo *SsaoData = static_cast<SsaoInfo *>(SsaoInfoStructedBuffer->map());

  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&SsaoData[0].Proj, Mat);

  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&SsaoData[0].View, Mat);

  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvProjMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&SsaoData[0].InvProj, Mat);

  static dx::XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
  Mat =
      dx::XMMatrixTranspose(dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix) * T);
  dx::XMStoreFloat4x4(&SsaoData[0].TexProj, Mat);

  for (UINT I = 0; I < 14; I++) {
    SsaoData[0].OffsetVec[I] = OffsetVec[I];
  }

  SsaoData[0].OcclusionRadius = G_RenderEffectConfig.SsaoRadius;
  SsaoData[0].OcclusionFadeStart = G_RenderEffectConfig.SsaoStart;
  SsaoData[0].OcclusionFadeEnd = G_RenderEffectConfig.SsaoEnd;
  SsaoData[0].SurfaceEpsilon = G_RenderEffectConfig.SsaoEpsilon;
  SsaoInfoStructedBuffer->unmap();

  context()->setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  context()->setVertexBuffer(VertexBuffer);
  context()->setIndexBuffer(IndexBuffer);
  context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 0, 1,
                                &SsaoInfoStructedBufferSrv);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 0, 1,
                                &SsaoInfoStructedBufferSrv);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 1, 1, &GeoBufferSrv);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 2, 1, &DepthMapSrv);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 3, 1, &RandomMapSrv);

  context()->setSamplers(RS_SHADER_STAGE::PIXEL, 0, 1, &SamplePointClamp);
  context()->setSamplers(RS_SHADER_STAGE::PIXEL, 1, 1, &SampleLinearClamp);
  context()->setSamplers(RS_SHADER_STAGE::PIXEL, 2, 1, &SampleDepthMap);
  context()->setSamplers(RS_SHADER_STAGE::PIXEL, 3, 1, &SampleLinearWrap);

  context()->drawIndexedInstanced(6, 1, 0, 0, 0);

  context()->setRenderTargets(1, &CompressRtv, nullptr);
  context()->setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  context()->setVertexBuffer(VertexBuffer);
  context()->setIndexBuffer(IndexBuffer);
  // the full size viewport is clipped to the half size target like d3d11,
  // the pixel shader doubles texcoords for it
  context()->setViewport(G_ViewPort);
  context()->setPipelineState(CompressPipeline);
  context()->setSamplers(RS_SHADER_STAGE::PIXEL, 0, 1, &SampleLinearWrap);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 0, 1, &NotCompressSrv);

  context()->drawIndexedInstanced(6, 1, 0, 0, 0);

  context()->setRenderTargets(0, nullptr, nullptr);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 1, 1, &NullSrv);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 2, 1, &NullSrv);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 3, 1, &NullSrv);
}

bool RSPass_Ssao::createShaders() {
  VertexShader =
      compileShader("./Assets/Shaders/ssao_vertex.hlsl", "main", "vs_5_0");
  FAIL_NULL_RETURN(VertexShader);

  PixelShader =
      compileShader("./Assets/Shaders/ssao_pixel.hlsl", "main", "ps_5_0");
  FAIL_NULL_RETURN(PixelShader);

  CompressVertexShader =
      compileShader("./Assets/Shaders/compress_vertex.hlsl", "main", "vs_5_0");
  FAIL_NULL_RETURN(CompressVertexShader);

  CompressPixelShader =
      compileShader("./Assets/Shaders/compress_pixel.hlsl", "main", "ps_5_0");
  FAIL_NULL_RETURN(CompressPixelShader);

  return true;
}

bool RSPass_Ssao::createBuffers() {
  SsaoInfoStructedBuffer = createDynamicStructuredBuffer(sizeof(SsaoInfo), 1);
  FAIL_NULL_RETURN(SsaoInfoStructedBuffer);

  return createScreenQuad(&VertexBuffer, &IndexBuffer);
}

bool RSPass_Ssao::createTextures() {
  auto Devices = G_RSRoot->getDevices();
  RS_RESOURCE_INFO Dti = {};

  std::vector<dx::PackedVector::XMCOLOR> Random(256 * 256);
  int Basic = 1;
  int Range = 100;
  dx::XMFLOAT3 V = {0.f, 0.f, 0.f};
  std::srand(static_cast<UINT>(std::time(nullptr)) +
             static_cast<UINT>(std::rand()));
  for (int I = 0; I < 256; I++) {
    for (int J = 0; J < 256; J++) {
      V.x = static_cast<float>(std::rand() % Range + Basic) / 100.f;
      V.y = static_cast<float>(std::rand() % Range + Basic) / 100.f;
      V.z = static_cast<float>(std::rand() % Range + Basic) / 100.f;
      Random[I * 256 + J] = dx::PackedVector::XMCOLOR(V.x, V.y, V.z, 0.f);
    }
  }

  RS_SUBRESOURCE_DATA InitData = {};
  InitData.Data = Random.data();
  InitData.RowPitch = 256 * sizeof(dx::PackedVector::XMCOLOR);
  InitData.SlicePitch = InitData.RowPitch * 256;

  RS_TEXTURE_DESC TexDesc = {};
  TexDesc.Format = VK_FORMAT_R8G8B8A8_UNORM;
  TexDesc.Width = 256;
  TexDesc.Height = 256;
  TexDesc.Usage = VK_IMAGE_USAGE_SAMPLED_BIT;
  RSTexture *Texture = Devices->createTexture(TexDesc, &InitData);
  FAIL_NULL_RETURN(Texture);
  RSShaderResourceView *Srv = Devices->createShaderResourceView(Texture);
  FAIL_NULL_RETURN(Srv);

  Dti = {};
  Dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
  Dti.Resource.Texture2D = Texture;
  Dti.Srv = Srv;
  G_RSRoot->getResourceManager()->addResource("random-tex-ssao", Dti);

  UINT Width = Devices->getCurrWndWidth();
  UINT Height = Devices->getCurrWndHeight();
  Texture = createTexture2D(
      VK_FORMAT_R8G8B8A8_UNORM, Width, Height,
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
  FAIL_NULL_RETURN(Texture);
  RSRenderTargetView *Rtv = Devices->createRenderTargetView(Texture);
  FAIL_NULL_RETURN(Rtv);
  Srv = Devices->createShaderResourceView(Texture);
  FAIL_NULL_RETURN(Srv);

  Dti = {};
  Dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
  Dti.Resource.Texture2D = Texture;
  Dti.Rtv = Rtv;
  Dti.Srv = Srv;
  G_RSRoot->getResourceManager()->addResource("ssao-tex-ssao", Dti);

  Texture = createTexture2D(VK_FORMAT_R8G8B8A8_UNORM, Width / 2, Height / 2,
                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                VK_IMAGE_USAGE_SAMPLED_BIT |
                                VK_IMAGE_USAGE_STORAGE_BIT);
  FAIL_NULL_RETURN(Texture);
  Rtv = Devices->createRenderTargetView(Texture);
  FAIL_NULL_RETURN(Rtv);
  Srv = Devices->createShaderResourceView(Texture);
  FAIL_NULL_RETURN(Srv);
  RSUnorderedAccessView *Uav = Devices->createUnorderedAccessView(Texture);
  FAIL_NULL_RETURN(Uav);

  Dti = {};
  Dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
  Dti.Resource.Texture2D = Texture;
  Dti.Rtv = Rtv;
  Dti.Srv = Srv;
  Dti.Uav = Uav;
  G_RSRoot->getResourceManager()->addResource("ssao-tex-compress-ssao", Dti);

  return true;
}

bool RSPass_Ssao::createViews() {
  RandomMapSrv =
      G_RSRoot->getResourceManager()->getResource("random-tex-ssao")->Srv;
  GeoBufferSrv =
      G_RSRoot->getResourceManager()->getResource("mrt-geo-buffer")->Srv;
  DepthMapSrv = G_RSRoot->getResourceManager()->getResource("mrt-depth")->Srv;
  RenderTargetView =
      G_RSRoot->getResourceManager()->getResource("ssao-tex-ssao")->Rtv;
  NotCompressSrv =
      G_RSRoot->getResourceManager()->getResource("ssao-tex-ssao")->Srv;
  CompressRtv = G_RSRoot->getResourceManager()
                    ->getResource("ssao-tex-compress-ssao")
                    ->Rtv;

  SsaoInfoStructedBufferSrv =
      createStructuredSrv(SsaoInfoStructedBuffer, sizeof(SsaoInfo), 1);
  FAIL_NULL_RETURN(SsaoInfoStructedBufferSrv);

  return true;
}

bool RSPass_Ssao::createSamplers() {
  auto Devices = G_RSRoot->getDevices();

  SamplePointClamp = Devices->createSampler(
      makePointSamplerInfo(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));
  FAIL_NULL_RETURN(SamplePointClamp);

  SampleLinearClamp = Devices->createSampler(
      makeLinearSamplerInfo(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));
  FAIL_NULL_RETURN(SampleLinearClamp);

  // the comparison func was ignored by d3d11 with a non comparison filter
  SampleDepthMap = Devices->createSampler(fitDepthSamplerInfo(
      makeLinearSamplerInfo(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)));
  FAIL_NULL_RETURN(SampleDepthMap);

  SampleLinearWrap = Devices->createSampler(
      makeLinearSamplerInfo(VK_SAMPLER_ADDRESS_MODE_REPEAT));
  FAIL_NULL_RETURN(SampleLinearWrap);

  return true;
}

bool RSPass_Ssao::createPipelines() {
  RS_GRAPHICS_PIPELINE_DESC Desc = {};
  Desc.VertexShader = VertexShader;
  Desc.PixelShader = PixelShader;
  Desc.ColorTargetCount = 1;
  Desc.ColorFormats[0] = VK_FORMAT_R8G8B8A8_UNORM;
  SsaoPipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(SsaoPipeline);

  Desc.VertexShader = CompressVertexShader;
  Desc.PixelShader = CompressPixelShader;
  CompressPipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(CompressPipeline);

  return true;
}

RSPass_KBBlur::RSPass_KBBlur(std::string &Name,
                             PASS_TYPE Type,
                             RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), HoriBlurShader(nullptr),
      VertBlurShader(nullptr), SsaoTexUav(nullptr), GeoBufferSrv(nullptr),
      DepthMapSrv(nullptr) {}

RSPass_KBBlur::RSPass_KBBlur(const RSPass_KBBlur &Source)
    : RSPass_Base(Source), HoriBlurShader(Source.HoriBlurShader),
      VertBlurShader(Source.VertBlurShader), SsaoTexUav(Source.SsaoTexUav),
      GeoBufferSrv(Source.GeoBufferSrv), DepthMapSrv(Source.DepthMapSrv) {
  if (HasBeenInited) {
    RS_ADDREF(HoriBlurShader);
    RS_ADDREF(VertBlurShader);
  }
}

RSPass_KBBlur::~RSPass_KBBlur() {}

RSPass_KBBlur *RSPass_KBBlur::clonePass() { return new RSPass_KBBlur(*this); }

bool RSPass_KBBlur::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }

  HasBeenInited = true;

  return true;
}

void RSPass_KBBlur::releasePass() {
  RS_RELEASE(HoriBlurShader);
  RS_RELEASE(VertBlurShader);
}

void RSPass_KBBlur::execuatePass() {
  RSShaderResourceView *Srv[] = {GeoBufferSrv, DepthMapSrv};
  static RSUnorderedAccessView *NullUav = nullptr;
  static RSShaderResourceView *NullSrv[] = {nullptr, nullptr};

  static const UINT LoopCount = G_RenderEffectConfig.SsaoBlurCount;
  static UINT Width =
      getRSDX11RootInstance()->getDevices()->getCurrWndWidth() / 2;
  static UINT Height =
      getRSDX11RootInstance()->getDevices()->getCurrWndHeight() / 2;
  UINT DispatchVert = rs_tool::align(Width, 256) / 256;
  UINT DispatchHori = rs_tool::align(Height, 256) / 256;

  for (UINT I = 0; I < LoopCount; I++) {
    context()->setPipelineState(HoriBlurShader);
    context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0, 1,
                                       &SsaoTexUav);
    context()->setShaderResources(RS_SHADER_STAGE::COMPUTE, 0, 2, Srv);
    context()->dispatch(DispatchVert, Height, 1);
    context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0, 1,
                                       &NullUav);
    context()->setShaderResources(RS_SHADER_STAGE::COMPUTE, 0, 2, NullSrv);

    context()->setPipelineState(VertBlurShader);
    context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0, 1,
                                       &SsaoTexUav);
    context()->setShaderResources(RS_SHADER_STAGE::COMPUTE, 0, 2, Srv);
    context()->dispatch(Width, DispatchHori, 1);
    context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0, 1,
                                       &NullUav);
    context()->setShaderResources(RS_SHADER_STAGE::COMPUTE, 0, 2, NullSrv);
  }
}

bool RSPass_KBBlur::createShaders() {
  HoriBlurShader =
      compileComputePipeline("./Assets/Shaders/ssao_compute.hlsl", "HMain");
  FAIL_NULL_RETURN(HoriBlurShader);

  VertBlurShader =
      compileComputePipeline("./Assets/Shaders/ssao_compute.hlsl", "VMain");
  FAIL_NULL_RETURN(VertBlurShader);

  return true;
}

bool RSPass_KBBlur::createViews() {
  GeoBufferSrv =
      G_RSRoot->getResourceManager()->getResource("mrt-geo-buffer")->Srv;
  DepthMapSrv = G_RSRoot->getResourceManager()->getResource("mrt-depth")->Srv;
  SsaoTexUav = G_RSRoot->getResourceManager()
                   ->getResource("ssao-tex-compress-ssao")
                   ->Uav;

  return true;
}

RSPass_Shadow::RSPass_Shadow(std::string &Name,
                             PASS_TYPE Type,
                             RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      AniVertexShader(nullptr), StaticPipeline(nullptr), AniPipeline(nullptr),
      DepthStencilView({nullptr}), DrawCallType(DRAWCALL_TYPE::OPACITY),
      DrawCallPipe(nullptr), ViewProjStructedBuffer(nullptr),
      ViewProjStructedBufferSrv(nullptr), InstanceStructedBuffer(nullptr),
      InstanceStructedBufferSrv(nullptr), BonesStructedBuffer(nullptr),
      BonesStructedBufferSrv(nullptr) {}

RSPass_Shadow::RSPass_Shadow(const RSPass_Shadow &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      AniVertexShader(Source.AniVertexShader),
      StaticPipeline(Source.StaticPipeline), AniPipeline(Source.AniPipeline),
      DepthStencilView(Source.DepthStencilView),
      DrawCallType(Source.DrawCallType), DrawCallPipe(Source.DrawCallPipe),
      ViewProjStructedBuffer(Source.ViewProjStructedBuffer),
      ViewProjStructedBufferSrv(Source.ViewProjStructedBufferSrv),
      InstanceStructedBuffer(Source.InstanceStructedBuffer),
      InstanceStructedBufferSrv(Source.InstanceStructedBufferSrv),
      BonesStructedBuffer(Source.BonesStructedBuffer),
      BonesStructedBufferSrv(Source.BonesStructedBufferSrv) {
  if (HasBeenInited) {
    RS_ADDREF(VertexShader);
    RS_ADDREF(AniVertexShader);
    RS_ADDREF(StaticPipeline);
    RS_ADDREF(AniPipeline);
    RS_ADDREF(ViewProjStructedBuffer);
    RS_ADDREF(ViewProjStructedBufferSrv);
    RS_ADDREF(InstanceStructedBuffer);
    RS_ADDREF(InstanceStructedBufferSrv);
    RS_ADDREF(BonesStructedBuffer);
    RS_ADDREF(BonesStructedBufferSrv);
  }
}

RSPass_Shadow::~RSPass_Shadow() {}

RSPass_Shadow *RSPass_Shadow::clonePass() { return new RSPass_Shadow(*this); }

bool RSPass_Shadow::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createStates()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }

  DrawCallType = DRAWCALL_TYPE::OPACITY;
  DrawCallPipe = G_RSRoot->getDrawCallsPool()->getDrawCallsPipe(DrawCallType);

  HasBeenInited = true;

  return true;
}

void RSPass_Shadow::releasePass() {
  RS_RELEASE(VertexShader);
  RS_RELEASE(AniVertexShader);
  RS_RELEASE(StaticPipeline);
  RS_RELEASE(AniPipeline);
  RS_RELEASE(ViewProjStructedBufferSrv);
  RS_RELEASE(ViewProjStructedBuffer);
  RS_RELEASE(InstanceStructedBufferSrv);
  RS_RELEASE(InstanceStructedBuffer);
  RS_RELEASE(BonesStructedBufferSrv);
  RS_RELEASE(BonesStructedBuffer);

  G_RSRoot->getResourceManager()->deleteResource("light-depth-light-other");
  G_RSRoot->getResourceManager()->deleteResource("light-depth-light-dep0");
  G_RSRoot->getResourceManager()->deleteResource("light-depth-light-dep1");
  G_RSRoot->getResourceManager()->deleteResource("light-depth-light-dep2");
  G_RSRoot->getResourceManager()->deleteResource("light-depth-light-dep3");
}

void RSPass_Shadow::execuatePass() {
  dx::XMMATRIX Mat = {};
  auto ShadowLights = G_RSRoot->getLightsContainer()->getShadowLightsArray();
  UINT ShadowSize = static_cast<UINT>(ShadowLights->size());

  static std::string A_NAME = "AnimationVertex";
  static const auto ANIMAT_LAYOUT =
      getRSDX11RootInstance()->getStaticResources()->getStaticInputLayout(
          A_NAME);

  for (UINT I = 0; I < ShadowSize; I++) {
    context()->setRenderTargets(0, nullptr, DepthStencilView[I]);
    context()->setViewport(G_ViewPort);
    context()->clearDepthStencilView(DepthStencilView[I], 1.f);

    ViewProj *VpData = static_cast<ViewProj *>(ViewProjStructedBuffer->map());
    auto Light = (*ShadowLights)[I];
    auto LCam = Light->getRSLightCamera();
    Mat = dx::XMLoadFloat4x4(&(LCam->getRSCameraInfo()->ViewMatrix));
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&VpData[0].ViewMat, Mat);
    Mat = dx::XMLoadFloat4x4(&(LCam->getRSCameraInfo()->ProjMatrix));
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&VpData[0].ProjMat, Mat);
    ViewProjStructedBuffer->unmap();

    context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 0, 1,
                                  &ViewProjStructedBufferSrv);

    for (auto &Draw : DrawCallPipe->Data) {
      if (Draw.MeshData.InputLayout == ANIMAT_LAYOUT) {
        context()->setPipelineState(AniPipeline);
        context()->setVertexBuffer(Draw.MeshData.VertexBuffer);

        dx::XMFLOAT4X4 *BData =
            static_cast<dx::XMFLOAT4X4 *>(BonesStructedBuffer->map());
        void *RawBoneData = Draw.InstanceData.BonesArrayPtr;
        std::vector<std::vector<RS_SUBMESH_BONE_DATA>> *Bones = nullptr;
        Bones = static_cast<decltype(Bones)>(RawBoneData);
        // TEMP-----------------------
        auto BoneInsSize = Bones->size();
        // TEMP-----------------------
        for (size_t I = 0; I < BoneInsSize; I++) {
          for (size_t J = 0; J < MAX_STRUCTURED_BUFFER_SIZE; J++) {
            if (J < (*Bones)[I].size()) {
              dx::XMMATRIX Trans =
                  dx::XMLoadFloat4x4(&((*Bones)[I][J].BoneTransform));
              Trans = dx::XMMatrixTranspose(Trans);
              dx::XMStoreFloat4x4(BData + I * MAX_STRUCTURED_BUFFER_SIZE + J,
                                  Trans);
            } else {
              dx::XMStoreFloat4x4(BData + I * MAX_STRUCTURED_BUFFER_SIZE + J,
                                  dx::XMMatrixIdentity());
            }
          }
        }
        BonesStructedBuffer->unmap(BoneInsSize * MAX_STRUCTURED_BUFFER_SIZE *
                                   sizeof(dx::XMFLOAT4X4));

        context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 3, 1,
                                      &BonesStructedBufferSrv);
      } else {
        context()->setPipelineState(StaticPipeline);
        context()->setVertexBuffer(Draw.MeshData.VertexBuffer);
      }

      auto InsVecPtr = Draw.InstanceData.DataArrayPtr;
      auto Size = InsVecPtr->size();
      RS_INSTANCE_DATA *InsData =
          static_cast<RS_INSTANCE_DATA *>(InstanceStructedBuffer->map());
      for (size_t I = 0; I < Size; I++) {
        Mat = dx::XMLoadFloat4x4(&(*InsVecPtr)[I].WorldMatrix);
        Mat = dx::XMMatrixTranspose(Mat);
        dx::XMStoreFloat4x4(&InsData[I].WorldMatrix, Mat);
        InsData[I].MaterialData = (*InsVecPtr)[I].MaterialData;
        InsData[I].CustomizedData1 = (*InsVecPtr)[I].CustomizedData1;
        InsData[I].CustomizedData2 = (*InsVecPtr)[I].CustomizedData2;
      }
      InstanceStructedBuffer->unmap(Size * sizeof(RS_INSTANCE_DATA));

      context()->setPrimitiveTopology(Draw.MeshData.TopologyType);
      context()->setIndexBuffer(Draw.MeshData.IndexBuffer);
      context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 1, 1,
                                    &InstanceStructedBufferSrv);

      context()->drawIndexedInstanced(
          Draw.MeshData.IndexSize,
          static_cast<UINT>(Draw.InstanceData.DataArrayPtr->size()), 0, 0, 0);

      if (Draw.MeshData.InputLayout == ANIMAT_LAYOUT) {
        RSShaderResourceView *NullSrv = nullptr;
        context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 3, 1, &NullSrv);
      }
    }
  }

  context()->setRenderTargets(0, nullptr, nullptr);
}

bool RSPass_Shadow::createShaders() {
  VertexShader =
      compileShader("./Assets/Shaders/light_vertex.hlsl", "main", "vs_5_0");
  FAIL_NULL_RETURN(VertexShader);

  rs_tool::SHADER_MACRO Macro[] = {{"ANIMATION_VERTEX", "1"},
                                   {nullptr, nullptr}};
  AniVertexShader = compileShader("./Assets/Shaders/light_vertex.hlsl", "main",
                                  "vs_5_0", Macro);
  FAIL_NULL_RETURN(AniVertexShader);

  return true;
}

bool RSPass_Shadow::createStates() {
  // the rasterizer state is a part of the pipeline state objects, the bias
  // values keep their d3d11 meaning on the float depth buffer
  auto Static = G_RSRoot->getStaticResources();

  RS_GRAPHICS_PIPELINE_DESC Desc = {};
  Desc.VertexShader = VertexShader;
  Desc.InputLayout = Static->getStaticInputLayout("TangentVertex");
  Desc.FillMode = VK_POLYGON_MODE_FILL;
  Desc.CullMode = VK_CULL_MODE_BACK_BIT;
  Desc.FrontFace = VK_FRONT_FACE_CLOCKWISE;
  Desc.DepthBiasEnable = true;
  Desc.DepthBiasConstant = 50000.f;
  Desc.DepthBiasSlope = 1.f;
  Desc.DepthBiasClamp = 0.f;
  Desc.DepthClipEnable = true;
  Desc.DepthFormat = DEPTH_FORMAT;
  StaticPipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(StaticPipeline);

  Desc.VertexShader = AniVertexShader;
  Desc.InputLayout = Static->getStaticInputLayout("AnimationVertex");
  AniPipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(AniPipeline);

  return true;
}

bool RSPass_Shadow::createBuffers() {
  InstanceStructedBuffer = createDynamicStructuredBuffer(
      sizeof(RS_INSTANCE_DATA), MAX_INSTANCE_SIZE);
  FAIL_NULL_RETURN(InstanceStructedBuffer);

  BonesStructedBuffer = createDynamicStructuredBuffer(
      sizeof(dx::XMFLOAT4X4),
      MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE);
  FAIL_NULL_RETURN(BonesStructedBuffer);

  ViewProjStructedBuffer = createDynamicStructuredBuffer(sizeof(ViewProj), 1);
  FAIL_NULL_RETURN(ViewProjStructedBuffer);

  return true;
}

bool RSPass_Shadow::createViews() {
  auto Devices = G_RSRoot->getDevices();

  RS_TEXTURE_DESC TexDesc = {};
  TexDesc.Format = DEPTH_FORMAT;
  TexDesc.Width = Devices->getCurrWndWidth();
  TexDesc.Height = Devices->getCurrWndHeight();
  TexDesc.ArrayLayers = MAX_SHADOW_SIZE;
  TexDesc.Usage =
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  RSTexture *DepthTex = Devices->createTexture(TexDesc);
  FAIL_NULL_RETURN(DepthTex);

  RS_TEXTURE_VIEW_DESC DsvDesc = {};
  DsvDesc.ViewType = VK_IMAGE_VIEW_TYPE_2D;
  DsvDesc.LayerCount = 1;
  for (UINT I = 0; I < MAX_SHADOW_SIZE; I++) {
    DsvDesc.BaseLayer = I;
    DepthStencilView[I] = Devices->createDepthStencilView(DepthTex, &DsvDesc);
    FAIL_NULL_RETURN(DepthStencilView[I]);
  }

  RS_TEXTURE_VIEW_DESC SrvDesc = {};
  SrvDesc.ViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  SrvDesc.LayerCount = MAX_SHADOW_SIZE;
  RSShaderResourceView *Srv =
      Devices->createShaderResourceView(DepthTex, &SrvDesc);
  FAIL_NULL_RETURN(Srv);

  RS_RESOURCE_INFO Dti = {};
  Dti.Type = RS_RESOURCE_TYPE::TEXTURE2D;
  Dti.Resource.Texture2D = DepthTex;
  Dti.Srv = Srv;
  G_RSRoot->getResourceManager()->addResource("light-depth-light-other", Dti);

  Dti = {};
  Dti.Dsv = DepthStencilView[0];
  G_RSRoot->getResourceManager()->addResource("light-depth-light-dep0", Dti);
  Dti.Dsv = DepthStencilView[1];
  G_RSRoot->getResourceManager()->addResource("light-depth-light-dep1", Dti);
  Dti.Dsv = DepthStencilView[2];
  G_RSRoot->getResourceManager()->addResource("light-depth-light-dep2", Dti);
  Dti.Dsv = DepthStencilView[3];
  G_RSRoot->getResourceManager()->addResource("light-depth-light-dep3", Dti);

  InstanceStructedBufferSrv = createStructuredSrv(
      InstanceStructedBuffer, sizeof(RS_INSTANCE_DATA), MAX_INSTANCE_SIZE);
  FAIL_NULL_RETURN(InstanceStructedBufferSrv);

  BonesStructedBufferSrv = createStructuredSrv(
      BonesStructedBuffer, sizeof(dx::XMFLOAT4X4),
      MAX_STRUCTURED_BUFFER_SIZE * MAX_STRUCTURED_BUFFER_SIZE);
  FAIL_NULL_RETURN(BonesStructedBufferSrv);

  ViewProjStructedBufferSrv =
      createStructuredSrv(ViewProjStructedBuffer, sizeof(ViewProj), 1);
  FAIL_NULL_RETURN(ViewProjStructedBufferSrv);

  return true;
}

bool RSPass_Shadow::createSamplers() { return true; }

RSPass_Defered::RSPass_Defered(std::string &Name,
                               PASS_TYPE Type,
                               RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      PixelShader(nullptr), Pipeline(nullptr), RenderTargetView(nullptr),
      LinearWrapSampler(nullptr), PointClampSampler(nullptr),
      ShadowTexSampler(nullptr), LightInfoStructedBuffer(nullptr),
      LightInfoStructedBufferSrv(nullptr), LightStructedBuffer(nullptr),
      LightStructedBufferSrv(nullptr), AmbientStructedBuffer(nullptr),
      AmbientStructedBufferSrv(nullptr), ShadowStructedBuffer(nullptr),
      ShadowStructedBufferSrv(nullptr), CameraStructedBuffer(nullptr),
      CameraStructedBufferSrv(nullptr), GeoBufferSrv(nullptr),
      AnisotropicSrv(nullptr), SsaoSrv(nullptr), ShadowDepthSrv(nullptr),
      VertexBuffer(nullptr), IndexBuffer(nullptr), RSCameraInfo(nullptr) {}

RSPass_Defered::RSPass_Defered(const RSPass_Defered &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      PixelShader(Source.PixelShader), Pipeline(Source.Pipeline),
      RenderTargetView(Source.RenderTargetView),
      LinearWrapSampler(Source.LinearWrapSampler),
      PointClampSampler(Source.PointClampSampler),
      ShadowTexSampler(Source.ShadowTexSampler),
      LightInfoStructedBuffer(Source.LightInfoStructedBuffer),
      LightInfoStructedBufferSrv(Source.LightInfoStructedBufferSrv),
      LightStructedBuffer(Source.LightStructedBuffer),
      LightStructedBufferSrv(Source.LightStructedBufferSrv),
      AmbientStructedBuffer(Source.AmbientStructedBuffer),
      AmbientStructedBufferSrv(Source.AmbientStructedBufferSrv),
      ShadowStructedBuffer(Source.ShadowStructedBuffer),
      ShadowStructedBufferSrv(Source.ShadowStructedBufferSrv),
      CameraStructedBuffer(Source.CameraStructedBuffer),
      CameraStructedBufferSrv(Source.CameraStructedBufferSrv),
      GeoBufferSrv(Source.GeoBufferSrv), AnisotropicSrv(Source.AnisotropicSrv),
      SsaoSrv(Source.SsaoSrv), ShadowDepthSrv(Source.ShadowDepthSrv),
      VertexBuffer(Source.VertexBuffer), IndexBuffer(Source.IndexBuffer),
      RSCameraInfo(Source.RSCameraInfo) {
  if (HasBeenInited) {
    RS_ADDREF(VertexShader);
    RS_ADDREF(PixelShader);
    RS_ADDREF(Pipeline);
    RS_ADDREF(LinearWrapSampler);
    RS_ADDREF(PointClampSampler);
    RS_ADDREF(ShadowTexSampler);
    RS_ADDREF(LightInfoStructedBuffer);
    RS_ADDREF(LightInfoStructedBufferSrv);
    RS_ADDREF(LightStructedBuffer);
    RS_ADDREF(LightStructedBufferSrv);
    RS_ADDREF(AmbientStructedBuffer);
    RS_ADDREF(AmbientStructedBufferSrv);
    RS_ADDREF(ShadowStructedBuffer);
    RS_ADDREF(ShadowStructedBufferSrv);
    RS_ADDREF(CameraStructedBuffer);
    RS_ADDREF(CameraStructedBufferSrv);
    RS_ADDREF(VertexBuffer);
    RS_ADDREF(IndexBuffer);
  }
}

RSPass_Defered::~RSPass_Defered() {}

RSPass_Defered *RSPass_Defered::clonePass() {
  return new RSPass_Defered(*this);
}

bool RSPass_Defered::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }
  if (!createPipelines()) {
    return false;
  }

  RSCameraInfo = G_RSRoot->getCamerasContainer()->getRSCameraInfo("temp-cam");

  HasBeenInited = true;

  return true;
}

void RSPass_Defered::releasePass() {
  RS_RELEASE(VertexShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(Pipeline);
  RS_RELEASE(LinearWrapSampler);
  RS_RELEASE(PointClampSampler);
  RS_RELEASE(ShadowTexSampler);
  RS_RELEASE(LightInfoStructedBuffer);
  RS_RELEASE(LightInfoStructedBufferSrv);
  RS_RELEASE(LightStructedBuffer);
  RS_RELEASE(LightStructedBufferSrv);
  RS_RELEASE(AmbientStructedBuffer);
  RS_RELEASE(AmbientStructedBufferSrv);
  RS_RELEASE(ShadowStructedBuffer);
  RS_RELEASE(ShadowStructedBufferSrv);
  RS_RELEASE(CameraStructedBuffer);
  RS_RELEASE(CameraStructedBufferSrv);
  RS_RELEASE(VertexBuffer);
  RS_RELEASE(IndexBuffer);
}

void RSPass_Defered::execuatePass() {
  context()->setRenderTargets(1, &RenderTargetView, nullptr);
  context()->setViewport(G_ViewPort);
  context()->clearRenderTargetView(RenderTargetView, dx::Colors::DarkGreen);
  context()->setPipelineState(Pipeline);

  dx::XMMATRIX Mat = {};

  Ambient *AmbData = static_cast<Ambient *>(AmbientStructedBuffer->map());
  dx::XMFLOAT4 AmbientL =
      getRSDX11RootInstance()->getLightsContainer()->getCurrentAmbientLight();
  AmbData[0].AmbientV = AmbientL;
  AmbientStructedBuffer->unmap();

  static auto Lights = G_RSRoot->getLightsContainer()->getLightsArray();
  LightInfo *LiData = static_cast<LightInfo *>(LightInfoStructedBuffer->map());
  LiData[0].CameraPos = RSCameraInfo->EyePosition;
  UINT DNum = 0;
  UINT SNum = 0;
  UINT PNum = 0;
  for (auto &L : *Lights) {
    auto Type = L->getRSLightType();
    switch (Type) {
    case LIGHT_TYPE::DIRECT:
      ++DNum;
      break;
    case LIGHT_TYPE::POINT:
      ++PNum;
      break;
    case LIGHT_TYPE::SPOT:
      ++SNum;
      break;
    default:
      break;
    }
  }
  LiData[0].DirectLightNum = DNum;
  LiData[0].PointLightNum = PNum;
  LiData[0].SpotLightNum = SNum;
  LiData[0].ShadowLightNum = static_cast<UINT>(
      G_RSRoot->getLightsContainer()->getShadowLightsArray()->size());
  LiData[0].ShadowLightIndex[0] = -1;
  LiData[0].ShadowLightIndex[1] = -1;
  LiData[0].ShadowLightIndex[2] = -1;
  LiData[0].ShadowLightIndex[3] = -1;
  auto ShadowIndeices =
      G_RSRoot->getLightsContainer()->getShadowLightIndeicesArray();
  for (UINT I = 0, E = LiData[0].ShadowLightNum; I < E; I++) {
    LiData[0].ShadowLightIndex[I] = (*ShadowIndeices)[I];
    if (I >= 3) {
      break;
    }
  }
  LightInfoStructedBuffer->unmap();

  RS_LIGHT_INFO *LData = static_cast<RS_LIGHT_INFO *>(LightStructedBuffer->map());
  UINT LightIndex = 0;
  for (auto &L : *Lights) {
    LData[LightIndex++] = *(L->getRSLightInfo());
  }
  LightStructedBuffer->unmap(LightIndex * sizeof(RS_LIGHT_INFO));

  ShadowInfo *SData = static_cast<ShadowInfo *>(ShadowStructedBuffer->map());
  auto ShadowLights = G_RSRoot->getLightsContainer()->getShadowLightsArray();
  UINT ShadowSize = static_cast<UINT>(ShadowLights->size());
  for (UINT I = 0; I < ShadowSize; I++) {
    auto LCam = (*ShadowLights)[I]->getRSLightCamera();
    Mat = dx::XMLoadFloat4x4(&(LCam->getRSCameraInfo()->ViewMatrix));
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&SData[I].ShadowViewMat, Mat);
    Mat = dx::XMLoadFloat4x4(&(LCam->getRSCameraInfo()->ProjMatrix));
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&SData[I].ShadowProjMat, Mat);
  }

  static dx::XMMATRIX T(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
  Mat =
      dx::XMMatrixTranspose(dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix) *
                            dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix) * T);
  dx::XMStoreFloat4x4(&SData[0].SSAOMat, Mat);
  ShadowStructedBuffer->unmap();

  ViewProj *VpData = static_cast<ViewProj *>(CameraStructedBuffer->map());
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvViewMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ViewMat, Mat);
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvProjMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ProjMat, Mat);
  CameraStructedBuffer->unmap();

  static auto DepthSrv =
      G_RSRoot->getResourceManager()->getResource("mrt-depth")->Srv;
  static auto MaterialSrv = G_RSRoot->getStaticResources()->getMaterialSrv();
  RSShaderResourceView *Srvs[] = {AmbientStructedBufferSrv,
                                  LightInfoStructedBufferSrv,
                                  LightStructedBufferSrv,
                                  ShadowStructedBufferSrv,
                                  CameraStructedBufferSrv,
                                  MaterialSrv,
                                  GeoBufferSrv,
                                  AnisotropicSrv,
                                  SsaoSrv,
                                  ShadowDepthSrv,
                                  G_IblBrdfSrv,
                                  G_DiffMapSrv,
                                  G_SpecMapSrv,
                                  DepthSrv};
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 0, 14, Srvs);

  RSSampler *Samps[] = {PointClampSampler, LinearWrapSampler, ShadowTexSampler};
  context()->setSamplers(RS_SHADER_STAGE::PIXEL, 0, 3, Samps);

  context()->setPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  context()->setVertexBuffer(VertexBuffer);
  context()->setIndexBuffer(IndexBuffer);

  context()->drawIndexedInstanced(6, 1, 0, 0, 0);

  context()->setRenderTargets(0, nullptr, nullptr);
  static RSShaderResourceView *Nullsrvs[] = {
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
      nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 0, 14, Nullsrvs);
}

bool RSPass_Defered::createShaders() {
  VertexShader =
      compileShader("./Assets/Shaders/defered_vertex.hlsl", "main", "vs_5_0");
  FAIL_NULL_RETURN(VertexShader);

  std::string ModelName = G_RenderEffectConfig.LightModel;
  std::transform(ModelName.begin(), ModelName.end(), ModelName.begin(),
                 static_cast<int (*)(int)>(std::toupper));
  rs_tool::SHADER_MACRO Macro[] = {{ModelName.c_str(), "1"},
                                   {nullptr, nullptr}};
  PixelShader = compileShader("./Assets/Shaders/defered_pixel.hlsl", "main",
                              "ps_5_0", Macro);
  FAIL_NULL_RETURN(PixelShader);

  return true;
}

bool RSPass_Defered::createBuffers() {
  if (!createScreenQuad(&VertexBuffer, &IndexBuffer)) {
    return false;
  }

  LightStructedBuffer =
      createDynamicStructuredBuffer(sizeof(RS_LIGHT_INFO), MAX_LIGHT_SIZE);
  FAIL_NULL_RETURN(LightStructedBuffer);

  AmbientStructedBuffer = createDynamicStructuredBuffer(sizeof(Ambient), 1);
  FAIL_NULL_RETURN(AmbientStructedBuffer);

  LightInfoStructedBuffer = createDynamicStructuredBuffer(sizeof(LightInfo), 1);
  FAIL_NULL_RETURN(LightInfoStructedBuffer);

  ShadowStructedBuffer =
      createDynamicStructuredBuffer(sizeof(ShadowInfo), MAX_SHADOW_SIZE);
  FAIL_NULL_RETURN(ShadowStructedBuffer);

  CameraStructedBuffer = createDynamicStructuredBuffer(sizeof(ViewProj), 1);
  FAIL_NULL_RETURN(CameraStructedBuffer);

  return true;
}

bool RSPass_Defered::createViews() {
  RenderTargetView = G_RSRoot->getDevices()->getHighDynamicRtv();

  GeoBufferSrv =
      G_RSRoot->getResourceManager()->getResource("mrt-geo-buffer")->Srv;
  AnisotropicSrv =
      G_RSRoot->getResourceManager()->getResource("mrt-anisotropic")->Srv;
  SsaoSrv = G_RSRoot->getResourceManager()
                ->getResource("ssao-tex-compress-ssao")
                ->Srv;
  ShadowDepthSrv = G_RSRoot->getResourceManager()
                       ->getResource("light-depth-light-other")
                       ->Srv;

  LightStructedBufferSrv = createStructuredSrv(
      LightStructedBuffer, sizeof(RS_LIGHT_INFO), MAX_LIGHT_SIZE);
  FAIL_NULL_RETURN(LightStructedBufferSrv);

  LightInfoStructedBufferSrv =
      createStructuredSrv(LightInfoStructedBuffer, sizeof(LightInfo), 1);
  FAIL_NULL_RETURN(LightInfoStructedBufferSrv);

  AmbientStructedBufferSrv =
      createStructuredSrv(AmbientStructedBuffer, sizeof(Ambient), 1);
  FAIL_NULL_RETURN(AmbientStructedBufferSrv);

  CameraStructedBufferSrv =
      createStructuredSrv(CameraStructedBuffer, sizeof(ViewProj), 1);
  FAIL_NULL_RETURN(CameraStructedBufferSrv);

  ShadowStructedBufferSrv = createStructuredSrv(
      ShadowStructedBuffer, sizeof(ShadowInfo), MAX_SHADOW_SIZE);
  FAIL_NULL_RETURN(ShadowStructedBufferSrv);

  return true;
}

bool RSPass_Defered::createSamplers() {
  auto Devices = G_RSRoot->getDevices();

  LinearWrapSampler = Devices->createSampler(
      makeConfigSamplerInfo(VK_SAMPLER_ADDRESS_MODE_REPEAT));
  FAIL_NULL_RETURN(LinearWrapSampler);

  PointClampSampler = Devices->createSampler(
      makePointSamplerInfo(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE));
  FAIL_NULL_RETURN(PointClampSampler);

  VkSamplerCreateInfo ShadowInfo =
      makeSamplerInfo(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST,
                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
  ShadowInfo.compareEnable = VK_TRUE;
  ShadowInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  ShadowTexSampler = Devices->createSampler(fitDepthSamplerInfo(ShadowInfo));
  FAIL_NULL_RETURN(ShadowTexSampler);

  return true;
}

bool RSPass_Defered::createPipelines() {
  RS_GRAPHICS_PIPELINE_DESC Desc = {};
  Desc.VertexShader = VertexShader;
  Desc.PixelShader = PixelShader;
  Desc.ColorTargetCount = 1;
  Desc.ColorFormats[0] =
      G_RSRoot->getDevices()->getHighDynamicTexture()->getDesc().Format;
  Pipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(Pipeline);

  return true;
}

RSPass_SkyShpere::RSPass_SkyShpere(std::string &Name,
                                   PASS_TYPE Type,
                                   RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      PixelShader(nullptr), Pipeline(nullptr), LinearWrapSampler(nullptr),
      RenderTargerView(nullptr), DepthStencilView(nullptr),
      SkyShpereInfoStructedBuffer(nullptr),
      SkyShpereInfoStructedBufferSrv(nullptr), SkySphereMesh({}),
      RSCameraInfo(nullptr) {}

RSPass_SkyShpere::RSPass_SkyShpere(const RSPass_SkyShpere &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      PixelShader(Source.PixelShader), Pipeline(Source.Pipeline),
      LinearWrapSampler(Source.LinearWrapSampler),
      RenderTargerView(Source.RenderTargerView),
      DepthStencilView(Source.DepthStencilView),
      SkyShpereInfoStructedBuffer(Source.SkyShpereInfoStructedBuffer),
      SkyShpereInfoStructedBufferSrv(Source.SkyShpereInfoStructedBufferSrv),
      SkySphereMesh(Source.SkySphereMesh), RSCameraInfo(Source.RSCameraInfo) {}

RSPass_SkyShpere::~RSPass_SkyShpere() {}

RSPass_SkyShpere *RSPass_SkyShpere::clonePass() {
  return new RSPass_SkyShpere(*this);
}

bool RSPass_SkyShpere::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createStates()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }

  SkySphereMesh =
      G_RSRoot->getMeshHelper()->getGeoGenerator()->createGeometrySphere(
          10.f, 0, LAYOUT_TYPE::NORMAL_TANGENT_TEX, false, {},
          "this is not a bug about loading skybox texture failed :)");
  if (!rs_tool::createDDSTextureFromFile(
          G_RSRoot->getDevices(),
          "./RenderSystem_StaticResources/Textures/ibl_brdf.dds", nullptr,
          &G_IblBrdfSrv)) {
    return false;
  }

  RSCameraInfo = G_RSRoot->getCamerasContainer()->getRSCameraInfo("temp-cam");

  HasBeenInited = true;

  return true;
}

void RSPass_SkyShpere::releasePass() {
  RS_RELEASE(VertexShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(Pipeline);
  RS_RELEASE(LinearWrapSampler);
  RS_RELEASE(SkyShpereInfoStructedBuffer);
  RS_RELEASE(SkyShpereInfoStructedBufferSrv);
  RS_RELEASE(G_IblBrdfSrv);

  G_RSRoot->getMeshHelper()->releaseSubMesh(SkySphereMesh);
}

void RSPass_SkyShpere::execuatePass() {
  context()->setRenderTargets(1, &RenderTargerView, DepthStencilView);
  context()->setViewport(G_ViewPort);
  context()->setPipelineState(Pipeline);

  dx::XMMATRIX Mat = {};

  SkyShpereInfo *SpData =
      static_cast<SkyShpereInfo *>(SkyShpereInfoStructedBuffer->map());
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&SpData[0].ViewMat, Mat);

  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&SpData[0].ProjMat, Mat);

  SpData[0].EyePosition = RSCameraInfo->EyePosition;

  Mat = dx::XMMatrixScaling(1000.f, 1000.f, 1000.f);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&SpData[0].WorldMat, Mat);
  SkyShpereInfoStructedBuffer->unmap();

  context()->setPrimitiveTopology(SkySphereMesh.TopologyType);
  context()->setVertexBuffer(SkySphereMesh.VertexBuffer);
  context()->setIndexBuffer(SkySphereMesh.IndexBuffer);
  context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 0, 1,
                                &SkyShpereInfoStructedBufferSrv);
  context()->setShaderResources(RS_SHADER_STAGE::PIXEL, 0, 1, &G_EnviMapSrv);
  context()->setSamplers(RS_SHADER_STAGE::PIXEL, 0, 1, &LinearWrapSampler);

  context()->drawIndexedInstanced(SkySphereMesh.IndexSize, 1, 0, 0, 0);

  context()->setRenderTargets(0, nullptr, nullptr);
}

bool RSPass_SkyShpere::createShaders() {
  VertexShader =
      compileShader("./Assets/Shaders/skysphere_vertex.hlsl", "main", "vs_5_0");
  FAIL_NULL_RETURN(VertexShader);

  PixelShader =
      compileShader("./Assets/Shaders/skysphere_pixel.hlsl", "main", "ps_5_0");
  FAIL_NULL_RETURN(PixelShader);

  return true;
}

bool RSPass_SkyShpere::createStates() {
  // zeroed d3d11 descs, so depth clip and depth write are disabled too
  RS_GRAPHICS_PIPELINE_DESC Desc = {};
  Desc.VertexShader = VertexShader;
  Desc.PixelShader = PixelShader;
  Desc.InputLayout =
      G_RSRoot->getStaticResources()->getStaticInputLayout("TangentVertex");
  Desc.CullMode = VK_CULL_MODE_NONE;
  Desc.FillMode = VK_POLYGON_MODE_FILL;
  Desc.DepthClipEnable = false;
  Desc.DepthTestEnable = true;
  Desc.DepthWriteEnable = false;
  Desc.DepthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  Desc.ColorTargetCount = 1;
  Desc.ColorFormats[0] =
      G_RSRoot->getDevices()->getHighDynamicTexture()->getDesc().Format;
  Desc.DepthFormat = DEPTH_FORMAT;
  Pipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(Pipeline);

  return true;
}

bool RSPass_SkyShpere::createBuffers() {
  SkyShpereInfoStructedBuffer =
      createDynamicStructuredBuffer(sizeof(SkyShpereInfo), 1);
  FAIL_NULL_RETURN(SkyShpereInfoStructedBuffer);

  return true;
}

bool RSPass_SkyShpere::createViews() {
  RenderTargerView = G_RSRoot->getDevices()->getHighDynamicRtv();
  DepthStencilView =
      G_RSRoot->getResourceManager()->getResource("mrt-depth")->Dsv;

  SkyShpereInfoStructedBufferSrv = createStructuredSrv(
      SkyShpereInfoStructedBuffer, sizeof(SkyShpereInfo), 1);
  FAIL_NULL_RETURN(SkyShpereInfoStructedBufferSrv);

  return true;
}

bool RSPass_SkyShpere::createSamplers() {
  LinearWrapSampler = G_RSRoot->getDevices()->createSampler(
      makeLinearSamplerInfo(VK_SAMPLER_ADDRESS_MODE_REPEAT));
  FAIL_NULL_RETURN(LinearWrapSampler);

  return true;
}

RSPass_Bloom::RSPass_Bloom(std::string &Name, PASS_TYPE Type, RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      PixelShader(nullptr), Pipeline(nullptr),
      DrawCallType(DRAWCALL_TYPE::LIGHT), DrawCallPipe(nullptr),
      ViewProjStructedBuffer(nullptr), ViewProjStructedBufferSrv(nullptr),
      InstanceStructedBuffer(nullptr), InstanceStructedBufferSrv(nullptr),
      Rtv(nullptr), DepthDsv(nullptr), RSCameraInfo(nullptr),
      VertexBuffer(nullptr), IndexBuffer(nullptr), Sampler(nullptr) {}

RSPass_Bloom::RSPass_Bloom(const RSPass_Bloom &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      PixelShader(Source.PixelShader), Pipeline(Source.Pipeline),
      DrawCallType(Source.DrawCallType), DrawCallPipe(Source.DrawCallPipe),
      ViewProjStructedBuffer(Source.ViewProjStructedBuffer),
      ViewProjStructedBufferSrv(Source.ViewProjStructedBufferSrv),
      InstanceStructedBuffer(Source.InstanceStructedBuffer),
      InstanceStructedBufferSrv(Source.InstanceStructedBufferSrv),
      Rtv(Source.Rtv), DepthDsv(Source.DepthDsv),
      RSCameraInfo(Source.RSCameraInfo), VertexBuffer(Source.VertexBuffer),
      IndexBuffer(Source.IndexBuffer), Sampler(Source.Sampler) {
  if (HasBeenInited) {
    RS_ADDREF(VertexShader);
    RS_ADDREF(PixelShader);
    RS_ADDREF(Pipeline);
    RS_ADDREF(ViewProjStructedBuffer);
    RS_ADDREF(ViewProjStructedBufferSrv);
    RS_ADDREF(InstanceStructedBuffer);
    RS_ADDREF(InstanceStructedBufferSrv);
    RS_ADDREF(VertexBuffer);
    RS_ADDREF(IndexBuffer);
    RS_ADDREF(Sampler);
  }
}

RSPass_Bloom::~RSPass_Bloom() {}

RSPass_Bloom *RSPass_Bloom::clonePass() { return new RSPass_Bloom(*this); }

bool RSPass_Bloom::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }
  if (!createPipelines()) {
    return false;
  }

  DrawCallPipe = G_RSRoot->getDrawCallsPool()->getDrawCallsPipe(DrawCallType);

  RSCameraInfo = G_RSRoot->getCamerasContainer()->getRSCameraInfo("temp-cam");
  if (!RSCameraInfo) {
    return false;
  }

  HasBeenInited = true;

  return true;
}

void RSPass_Bloom::releasePass() {
  G_RSRoot->getResourceManager()->deleteResource("bloom-light");
  G_RSRoot->getResourceManager()->deleteResource("bloom-compress-light");

  RS_RELEASE(VertexShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(Pipeline);
  RS_RELEASE(ViewProjStructedBufferSrv);
  RS_RELEASE(InstanceStructedBufferSrv);
  RS_RELEASE(ViewProjStructedBuffer);
  RS_RELEASE(InstanceStructedBuffer);
  RS_RELEASE(VertexBuffer);
  RS_RELEASE(IndexBuffer);
  RS_RELEASE(Sampler);
}

void RSPass_Bloom::execuatePass() {
  context()->setRenderTargets(1, &Rtv, DepthDsv);
  context()->setViewport(G_ViewPort);
  context()->setPipelineState(Pipeline);

  dx::XMMATRIX Mat = {};

  ViewProj *VpData = static_cast<ViewProj *>(ViewProjStructedBuffer->map());
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ViewMat, Mat);
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ProjMat, Mat);
  ViewProjStructedBuffer->unmap();

  context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 0, 1,
                                &ViewProjStructedBufferSrv);

  for (auto &Draw : DrawCallPipe->Data) {
    auto InsVecPtr = Draw.InstanceData.DataArrayPtr;
    auto Size = InsVecPtr->size();
    RS_INSTANCE_DATA *InsData =
        static_cast<RS_INSTANCE_DATA *>(InstanceStructedBuffer->map());
    for (size_t I = 0; I < Size; I++) {
      Mat = dx::XMLoadFloat4x4(&(*InsVecPtr)[I].WorldMatrix);
      Mat = dx::XMMatrixTranspose(Mat);
      dx::XMStoreFloat4x4(&InsData[I].WorldMatrix, Mat);
      InsData[I].MaterialData = (*InsVecPtr)[I].MaterialData;
      InsData[I].CustomizedData1 = (*InsVecPtr)[I].CustomizedData1;
      InsData[I].CustomizedData2 = (*InsVecPtr)[I].CustomizedData2;
    }
    InstanceStructedBuffer->unmap(Size * sizeof(RS_INSTANCE_DATA));

    context()->setPrimitiveTopology(Draw.MeshData.TopologyType);
    context()->setVertexBuffer(Draw.MeshData.VertexBuffer);
    context()->setIndexBuffer(Draw.MeshData.IndexBuffer);
    context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 1, 1,
                                  &InstanceStructedBufferSrv);

    context()->drawIndexedInstanced(
        Draw.MeshData.IndexSize,
        static_cast<UINT>(Draw.InstanceData.DataArrayPtr->size()), 0, 0, 0);
  }

  static RSShaderResourceView *NullSrv[] = {nullptr};
  context()->setRenderTargets(0, nullptr, nullptr);
  context()->setShaderResources(RS_SHADER_STAGE::VERTEX, 0, 1, NullSrv);
}

bool RSPass_Bloom::createShaders() {
  VertexShader =
      compileShader("./Assets/Shaders/bloom_vertex.hlsl", "main", "vs_5_0");
  FAIL_NULL_RETURN(VertexShader);

  const std::string PixelFactor =
      hyc::str::sFormat("{:+f}f", G_RenderEffectConfig.BloomLightPixelFactor);
  if (G_RenderEffectConfig.BloomOff) {
    rs_tool::SHADER_MACRO Macro[] = {{"PIXEL_FACTOR", PixelFactor.c_str()},
                                     {nullptr, nullptr}};
    PixelShader = compileShader("./Assets/Shaders/bloom_pixel.hlsl", "main",
                                "ps_5_0", Macro);
  } else {
    rs_tool::SHADER_MACRO Macro[] = {{"BLOOM_ON", "1"},
                                     {"PIXEL_FACTOR", PixelFactor.c_str()},
                                     {nullptr, nullptr}};
    PixelShader = compileShader("./Assets/Shaders/bloom_pixel.hlsl", "main",
                                "ps_5_0", Macro);
  }
  FAIL_NULL_RETURN(PixelShader);

  return true;
}

bool RSPass_Bloom::createSamplers() {
  Sampler = G_RSRoot->getDevices()->createSampler(
      makeLinearSamplerInfo(VK_SAMPLER_ADDRESS_MODE_REPEAT));
  FAIL_NULL_RETURN(Sampler);

  return true;
}

bool RSPass_Bloom::createBuffers() {
  InstanceStructedBuffer = createDynamicStructuredBuffer(
      sizeof(RS_INSTANCE_DATA), MAX_INSTANCE_SIZE);
  FAIL_NULL_RETURN(InstanceStructedBuffer);

  ViewProjStructedBuffer = createDynamicStructuredBuffer(sizeof(ViewProj), 1);
  FAIL_NULL_RETURN(ViewProjStructedBuffer);

  return createScreenQuad(&VertexBuffer, &IndexBuffer);
}

bool RSPass_Bloom::createViews() {
  Rtv = G_RSRoot->getDevices()->getHighDynamicRtv();

  DepthDsv = G_RSRoot->getResourceManager()->getResource("mrt-depth")->Dsv;
  if (!DepthDsv) {
    return false;
  }

  InstanceStructedBufferSrv = createStructuredSrv(
      InstanceStructedBuffer, sizeof(RS_INSTANCE_DATA), MAX_INSTANCE_SIZE);
  FAIL_NULL_RETURN(InstanceStructedBufferSrv);

  ViewProjStructedBufferSrv =
      createStructuredSrv(ViewProjStructedBuffer, sizeof(ViewProj), 1);
  FAIL_NULL_RETURN(ViewProjStructedBufferSrv);

  return true;
}

bool RSPass_Bloom::createPipelines() {
  RS_GRAPHICS_PIPELINE_DESC Desc = {};
  Desc.VertexShader = VertexShader;
  Desc.PixelShader = PixelShader;
  Desc.InputLayout =
      G_RSRoot->getStaticResources()->getStaticInputLayout("ColorVertex");
  Desc.ColorTargetCount = 1;
  Desc.ColorFormats[0] =
      G_RSRoot->getDevices()->getHighDynamicTexture()->getDesc().Format;
  Desc.DepthFormat = DEPTH_FORMAT;
  Pipeline = G_RSRoot->getDevices()->createGraphicsPipeline(Desc);
  FAIL_NULL_RETURN(Pipeline);

  return true;
}

RSPass_PriticleSetUp::RSPass_PriticleSetUp(std::string &Name,
                                           PASS_TYPE Type,
                                           RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), TilingConstant({}),
      ParticleRenderBuffer(nullptr), ParticleRender_Srv(nullptr),
      ParticleRender_Uav(nullptr), ParticlePartA(nullptr), PartA_Srv(nullptr),
      PartA_Uav(nullptr), ParticlePartB(nullptr), PartB_Uav(nullptr),
      ViewspacePosBuffer(nullptr), ViewSpacePos_Srv(nullptr),
      ViewSpacePos_Uav(nullptr), MaxRadiusBuffer(nullptr),
      MaxRadius_Srv(nullptr), MaxRadius_Uav(nullptr),
      StridedCoarseCullBuffer(nullptr), StridedCoarseCull_Srv(nullptr),
      StridedCoarseCull_Uav(nullptr), StridedCoarseCullCounterBuffer(nullptr),
      StridedCoarseCullCounter_Srv(nullptr),
      StridedCoarseCullCounter_Uav(nullptr), TiledIndexBuffer(nullptr),
      TiledIndex_Srv(nullptr), TiledIndex_Uav(nullptr), DeadListBuffer(nullptr),
      DeadList_Uav(nullptr), AliveIndexBuffer(nullptr), AliveIndex_Srv(nullptr),
      AliveIndex_Uav(nullptr), DeadListConstantBuffer(nullptr),
      ActiveListConstantBuffer(nullptr), EmitterConstantBuffer(nullptr),
      CameraConstantBuffer(nullptr), TilingConstantBuffer(nullptr),
      TimeConstantBuffer(nullptr), DebugCounterBuffer(nullptr),
      ParticleRandomTexture(nullptr), ParticleRandom_Srv(nullptr),
      SimulEmitterStructedBuffer(nullptr),
      SimulEmitterStructedBuffer_Srv(nullptr) {
  G_ParticleSetUpPass = this;
}

RSPass_PriticleSetUp::RSPass_PriticleSetUp(const RSPass_PriticleSetUp &Source)
    : RSPass_Base(Source), TilingConstant(Source.TilingConstant),
      ParticleRenderBuffer(Source.ParticleRenderBuffer),
      ParticleRender_Srv(Source.ParticleRender_Srv),
      ParticleRender_Uav(Source.ParticleRender_Uav),
      ParticlePartA(Source.ParticlePartA), PartA_Srv(Source.PartA_Srv),
      PartA_Uav(Source.PartA_Uav), ParticlePartB(Source.ParticlePartB),
      PartB_Uav(Source.PartB_Uav),
      ViewspacePosBuffer(Source.ViewspacePosBuffer),
      ViewSpacePos_Srv(Source.ViewSpacePos_Srv),
      ViewSpacePos_Uav(Source.ViewSpacePos_Uav),
      MaxRadiusBuffer(Source.MaxRadiusBuffer),
      MaxRadius_Srv(Source.MaxRadius_Srv), MaxRadius_Uav(Source.MaxRadius_Uav),
      StridedCoarseCullBuffer(Source.StridedCoarseCullBuffer),
      StridedCoarseCull_Srv(Source.StridedCoarseCull_Srv),
      StridedCoarseCull_Uav(Source.StridedCoarseCull_Uav),
      StridedCoarseCullCounterBuffer(Source.StridedCoarseCullCounterBuffer),
      StridedCoarseCullCounter_Srv(Source.StridedCoarseCullCounter_Srv),
      StridedCoarseCullCounter_Uav(Source.StridedCoarseCullCounter_Uav),
      TiledIndexBuffer(Source.TiledIndexBuffer),
      TiledIndex_Srv(Source.TiledIndex_Srv),
      TiledIndex_Uav(Source.TiledIndex_Uav),
      DeadListBuffer(Source.DeadListBuffer), DeadList_Uav(Source.DeadList_Uav),
      AliveIndexBuffer(Source.AliveIndexBuffer),
      AliveIndex_Srv(Source.AliveIndex_Srv),
      AliveIndex_Uav(Source.AliveIndex_Uav),
      DeadListConstantBuffer(Source.DeadListConstantBuffer),
      ActiveListConstantBuffer(Source.ActiveListConstantBuffer),
      EmitterConstantBuffer(Source.EmitterConstantBuffer),
      CameraConstantBuffer(Source.CameraConstantBuffer),
      TilingConstantBuffer(Source.TilingConstantBuffer),
      TimeConstantBuffer(Source.TimeConstantBuffer),
      DebugCounterBuffer(Source.DebugCounterBuffer),
      ParticleRandomTexture(Source.ParticleRandomTexture),
      ParticleRandom_Srv(Source.ParticleRandom_Srv),
      SimulEmitterStructedBuffer(Source.SimulEmitterStructedBuffer),
      SimulEmitterStructedBuffer_Srv(Source.SimulEmitterStructedBuffer_Srv) {
  G_ParticleSetUpPass = this;
}

RSPass_PriticleSetUp::~RSPass_PriticleSetUp() {}

const RS_TILING_CONSTANT &RSPass_PriticleSetUp::getTilingConstantInfo() const {
  return TilingConstant;
}

RSPass_PriticleSetUp *RSPass_PriticleSetUp::clonePass() {
  return new RSPass_PriticleSetUp(*this);
}

bool RSPass_PriticleSetUp::initPass() {
  if (HasBeenInited) {
    return true;
  }

  int Width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
  int Height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();

  TilingConstant.NumTilesX =
      rs_tool::align(Width, PTC_TILE_X_SIZE) / PTC_TILE_X_SIZE;
  TilingConstant.NumTilesY =
      rs_tool::align(Height, PTC_TILE_Y_SIZE) / PTC_TILE_Y_SIZE;
  TilingConstant.NumCoarseCullingTilesX = PTC_MAX_COARSE_CULL_TILE_X;
  TilingConstant.NumCoarseCullingTilesY = PTC_MAX_COARSE_CULL_TILE_Y;
  TilingConstant.NumCullingTilesPerCoarseTileX =
      rs_tool::align(TilingConstant.NumTilesX,
                     TilingConstant.NumCoarseCullingTilesX) /
      TilingConstant.NumCoarseCullingTilesX;
  TilingConstant.NumCullingTilesPerCoarseTileY =
      rs_tool::align(TilingConstant.NumTilesY,
                     TilingConstant.NumCoarseCullingTilesY) /
      TilingConstant.NumCoarseCullingTilesY;

  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }

  auto ResourceManager = getRSDX11RootInstance()->getResourceManager();
  RS_RESOURCE_INFO ResInfo;

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = ParticleRenderBuffer;
  ResInfo.Srv = ParticleRender_Srv;
  ResInfo.Uav = ParticleRender_Uav;
  ResourceManager->addResource(PTC_RENDER_BUFFER_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = ParticlePartA;
  ResInfo.Srv = PartA_Srv;
  ResInfo.Uav = PartA_Uav;
  ResourceManager->addResource(PTC_A_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = ParticlePartB;
  ResInfo.Uav = PartB_Uav;
  ResourceManager->addResource(PTC_B_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = ViewspacePosBuffer;
  ResInfo.Srv = ViewSpacePos_Srv;
  ResInfo.Uav = ViewSpacePos_Uav;
  ResourceManager->addResource(PTC_VIEW_SPCACE_POS_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = MaxRadiusBuffer;
  ResInfo.Srv = MaxRadius_Srv;
  ResInfo.Uav = MaxRadius_Uav;
  ResourceManager->addResource(PTC_MAX_RADIUS_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = StridedCoarseCullBuffer;
  ResInfo.Srv = StridedCoarseCull_Srv;
  ResInfo.Uav = StridedCoarseCull_Uav;
  ResourceManager->addResource(PTC_COARSE_CULL_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = StridedCoarseCullCounterBuffer;
  ResInfo.Srv = StridedCoarseCullCounter_Srv;
  ResInfo.Uav = StridedCoarseCullCounter_Uav;
  ResourceManager->addResource(PTC_COARSE_CULL_COUNTER_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = TiledIndexBuffer;
  ResInfo.Srv = TiledIndex_Srv;
  ResInfo.Uav = TiledIndex_Uav;
  ResourceManager->addResource(PTC_TILED_INDEX_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = DeadListBuffer;
  ResInfo.Uav = DeadList_Uav;
  ResourceManager->addResource(PTC_DEAD_LIST_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = AliveIndexBuffer;
  ResInfo.Srv = AliveIndex_Srv;
  ResInfo.Uav = AliveIndex_Uav;
  ResourceManager->addResource(PTC_ALIVE_INDEX_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = DeadListConstantBuffer;
  ResourceManager->addResource(PTC_DEAD_LIST_CONSTANT_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = ActiveListConstantBuffer;
  ResourceManager->addResource(PTC_ALIVE_LIST_CONSTANT_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = EmitterConstantBuffer;
  ResourceManager->addResource(PTC_EMITTER_CONSTANT_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = CameraConstantBuffer;
  ResourceManager->addResource(PTC_CAMERA_CONSTANT_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = TilingConstantBuffer;
  ResourceManager->addResource(PTC_TILING_CONSTANT_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = DebugCounterBuffer;
  ResourceManager->addResource(PTC_DEBUG_COUNTER_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::TEXTURE2D;
  ResInfo.Resource.Texture2D = ParticleRandomTexture;
  ResInfo.Srv = ParticleRandom_Srv;
  ResourceManager->addResource(PTC_RAMDOM_TEXTURE_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = SimulEmitterStructedBuffer;
  ResInfo.Srv = SimulEmitterStructedBuffer_Srv;
  ResourceManager->addResource(PTC_SIMU_EMITTER_STRU_NAME, ResInfo);

  ResInfo = {};
  ResInfo.Type = RS_RESOURCE_TYPE::BUFFER;
  ResInfo.Resource.Buffer = TimeConstantBuffer;
  ResourceManager->addResource(PTC_TIME_CONSTANT_NAME, ResInfo);

  HasBeenInited = true;

  return true;
}

void RSPass_PriticleSetUp::releasePass() {
  auto ResourceManager = getRSDX11RootInstance()->getResourceManager();
  ResourceManager->deleteResource(PTC_RENDER_BUFFER_NAME);
  ResourceManager->deleteResource(PTC_A_NAME);
  ResourceManager->deleteResource(PTC_B_NAME);
  ResourceManager->deleteResource(PTC_VIEW_SPCACE_POS_NAME);
  ResourceManager->deleteResource(PTC_MAX_RADIUS_NAME);
  ResourceManager->deleteResource(PTC_COARSE_CULL_NAME);
  ResourceManager->deleteResource(PTC_COARSE_CULL_COUNTER_NAME);
  ResourceManager->deleteResource(PTC_TILED_INDEX_NAME);
  ResourceManager->deleteResource(PTC_DEAD_LIST_NAME);
  ResourceManager->deleteResource(PTC_ALIVE_INDEX_NAME);
  ResourceManager->deleteResource(PTC_DEAD_LIST_CONSTANT_NAME);
  ResourceManager->deleteResource(PTC_ALIVE_LIST_CONSTANT_NAME);
  ResourceManager->deleteResource(PTC_EMITTER_CONSTANT_NAME);
  ResourceManager->deleteResource(PTC_CAMERA_CONSTANT_NAME);
  ResourceManager->deleteResource(PTC_TILING_CONSTANT_NAME);
  ResourceManager->deleteResource(PTC_DEBUG_COUNTER_NAME);
  ResourceManager->deleteResource(PTC_RAMDOM_TEXTURE_NAME);
  ResourceManager->deleteResource(PTC_SIMU_EMITTER_STRU_NAME);
  ResourceManager->deleteResource(PTC_TIME_CONSTANT_NAME);
}

void RSPass_PriticleSetUp::execuatePass() {}


bool RSPass_PriticleSetUp::createBuffers() {
  auto Devices = G_RSRoot->getDevices();
  RS_BUFFER_DESC BufDesc = {};

  BufDesc.Size = sizeof(RS_PARTICLE_PART_A) * PTC_MAX_PARTICLE_SIZE;
  BufDesc.Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  BufDesc.Memory = RS_MEMORY_USAGE::GPU_ONLY;
  BufDesc.StructureStride = sizeof(RS_PARTICLE_PART_A);
  ParticlePartA = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(ParticlePartA);

  BufDesc.Size = sizeof(RS_PARTICLE_PART_B) * PTC_MAX_PARTICLE_SIZE;
  BufDesc.StructureStride = sizeof(RS_PARTICLE_PART_B);
  ParticlePartB = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(ParticlePartB);

  BufDesc.Size = sizeof(dx::XMFLOAT4) * PTC_MAX_PARTICLE_SIZE;
  BufDesc.StructureStride = sizeof(dx::XMFLOAT4);
  ViewspacePosBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(ViewspacePosBuffer);

  BufDesc.Size = sizeof(float) * PTC_MAX_PARTICLE_SIZE;
  BufDesc.StructureStride = sizeof(float);
  MaxRadiusBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(MaxRadiusBuffer);

  BufDesc.Size = sizeof(UINT) * PTC_MAX_PARTICLE_SIZE;
  BufDesc.StructureStride = sizeof(UINT);
  DeadListBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(DeadListBuffer);

  BufDesc.Size = sizeof(RS_ALIVE_INDEX_BUFFER_ELEMENT) * PTC_MAX_PARTICLE_SIZE;
  BufDesc.StructureStride = sizeof(RS_ALIVE_INDEX_BUFFER_ELEMENT);
  AliveIndexBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(AliveIndexBuffer);

  // typed buffers become texel buffers
  BufDesc.Usage = VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
                  VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
  BufDesc.StructureStride = 0;
  BufDesc.Size =
      sizeof(UINT) * PTC_MAX_PARTICLE_SIZE * PTC_MAX_COARSE_CULL_TILE_SIZE;
  StridedCoarseCullBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(StridedCoarseCullBuffer);

  BufDesc.Size = sizeof(UINT) * PTC_MAX_COARSE_CULL_TILE_SIZE;
  StridedCoarseCullCounterBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(StridedCoarseCullCounterBuffer);

  UINT numElements = TilingConstant.NumTilesX * TilingConstant.NumTilesY *
                     PTC_TILE_BUFFER_SIZE;
  BufDesc.Size = sizeof(UINT) * numElements;
  TiledIndexBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(TiledIndexBuffer);

  numElements = TilingConstant.NumTilesX * TilingConstant.NumTilesY *
                PTC_TILE_X_SIZE * PTC_TILE_Y_SIZE;
  BufDesc.Size = 8 * numElements; // VK_FORMAT_R16G16B16A16_SFLOAT
  ParticleRenderBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(ParticleRenderBuffer);

  // written by copyStructureCount
  BufDesc = {};
  BufDesc.Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  BufDesc.Memory = RS_MEMORY_USAGE::GPU_ONLY;
  BufDesc.Size = 4 * sizeof(UINT); // one for record and three for pad
  DeadListConstantBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(DeadListConstantBuffer);
  ActiveListConstantBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(ActiveListConstantBuffer);

  BufDesc = {};
  BufDesc.Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  BufDesc.Memory = RS_MEMORY_USAGE::DYNAMIC;
  BufDesc.Size = sizeof(RS_PARTICLE_EMITTER_INFO);
  EmitterConstantBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(EmitterConstantBuffer);

  BufDesc.Size = sizeof(CAMERA_STATUS);
  CameraConstantBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(CameraConstantBuffer);

  BufDesc.Size = sizeof(RS_TILING_CONSTANT);
  TilingConstantBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(TilingConstantBuffer);

  BufDesc.Size = sizeof(PTC_TIME_CONSTANT);
  TimeConstantBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(TimeConstantBuffer);

  // a staging buffer in d3d11 which nothing reads back
  BufDesc = {};
  BufDesc.Size = sizeof(UINT);
  BufDesc.Memory = RS_MEMORY_USAGE::GPU_ONLY;
  DebugCounterBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(DebugCounterBuffer);

  SimulEmitterStructedBuffer = createDynamicStructuredBuffer(
      sizeof(SIMULATE_EMITTER_INFO), MAX_PARTICLE_EMITTER_SIZE);
  FAIL_NULL_RETURN(SimulEmitterStructedBuffer);

  RS_TEXTURE_DESC TexDesc = {};
  TexDesc.Width = 1024;
  TexDesc.Height = 1024;
  TexDesc.Format = VK_FORMAT_R32G32B32A32_SFLOAT;
  TexDesc.Usage = VK_IMAGE_USAGE_SAMPLED_BIT;

  std::vector<float> Values(TexDesc.Width * TexDesc.Height * 4);
  float *Ptr = Values.data();
  for (UINT I = 0, E = TexDesc.Width * TexDesc.Height; I < E; I++) {
    Ptr[0] = rs_tool::randomVariance(0.0f, 1.0f);
    Ptr[1] = rs_tool::randomVariance(0.0f, 1.0f);
    Ptr[2] = rs_tool::randomVariance(0.0f, 1.0f);
    Ptr[3] = rs_tool::randomVariance(0.0f, 1.0f);
    Ptr += 4;
  }

  RS_SUBRESOURCE_DATA Data = {};
  Data.Data = Values.data();
  Data.RowPitch = TexDesc.Width * 16;
  Data.SlicePitch = Data.RowPitch * TexDesc.Height;

  ParticleRandomTexture = Devices->createTexture(TexDesc, &Data);
  FAIL_NULL_RETURN(ParticleRandomTexture);

  return true;
}

bool RSPass_PriticleSetUp::createViews() {
  auto Devices = G_RSRoot->getDevices();
  RS_BUFFER_VIEW_DESC ViewDesc = {};

  auto Structured = [](UINT ElementSize, UINT NumElements,
                       bool WithCounter = false) {
    RS_BUFFER_VIEW_DESC Desc = {};
    Desc.ElementSize = ElementSize;
    Desc.NumElements = NumElements;
    Desc.WithCounter = WithCounter;
    return Desc;
  };
  auto Typed = [](VkFormat Format, UINT ElementSize, UINT NumElements) {
    RS_BUFFER_VIEW_DESC Desc = {};
    Desc.Format = Format;
    Desc.ElementSize = ElementSize;
    Desc.NumElements = NumElements;
    return Desc;
  };

  ViewDesc = Structured(sizeof(RS_PARTICLE_PART_A), PTC_MAX_PARTICLE_SIZE);
  PartA_Srv = Devices->createShaderResourceView(ParticlePartA, ViewDesc);
  FAIL_NULL_RETURN(PartA_Srv);
  PartA_Uav = Devices->createUnorderedAccessView(ParticlePartA, ViewDesc);
  FAIL_NULL_RETURN(PartA_Uav);

  ViewDesc = Structured(sizeof(RS_PARTICLE_PART_B), PTC_MAX_PARTICLE_SIZE);
  PartB_Uav = Devices->createUnorderedAccessView(ParticlePartB, ViewDesc);
  FAIL_NULL_RETURN(PartB_Uav);

  ViewDesc = Structured(sizeof(dx::XMFLOAT4), PTC_MAX_PARTICLE_SIZE);
  ViewSpacePos_Srv =
      Devices->createShaderResourceView(ViewspacePosBuffer, ViewDesc);
  FAIL_NULL_RETURN(ViewSpacePos_Srv);
  ViewSpacePos_Uav =
      Devices->createUnorderedAccessView(ViewspacePosBuffer, ViewDesc);
  FAIL_NULL_RETURN(ViewSpacePos_Uav);

  ViewDesc = Structured(sizeof(float), PTC_MAX_PARTICLE_SIZE);
  MaxRadius_Srv = Devices->createShaderResourceView(MaxRadiusBuffer, ViewDesc);
  FAIL_NULL_RETURN(MaxRadius_Srv);
  MaxRadius_Uav = Devices->createUnorderedAccessView(MaxRadiusBuffer, ViewDesc);
  FAIL_NULL_RETURN(MaxRadius_Uav);

  // D3D11_BUFFER_UAV_FLAG_APPEND
  ViewDesc = Structured(sizeof(UINT), PTC_MAX_PARTICLE_SIZE, true);
  DeadList_Uav = Devices->createUnorderedAccessView(DeadListBuffer, ViewDesc);
  FAIL_NULL_RETURN(DeadList_Uav);

  ViewDesc = Typed(VK_FORMAT_R32_UINT, sizeof(UINT),
                   PTC_MAX_PARTICLE_SIZE * PTC_MAX_COARSE_CULL_TILE_SIZE);
  StridedCoarseCull_Srv =
      Devices->createShaderResourceView(StridedCoarseCullBuffer, ViewDesc);
  FAIL_NULL_RETURN(StridedCoarseCull_Srv);
  StridedCoarseCull_Uav =
      Devices->createUnorderedAccessView(StridedCoarseCullBuffer, ViewDesc);
  FAIL_NULL_RETURN(StridedCoarseCull_Uav);

  ViewDesc =
      Typed(VK_FORMAT_R32_UINT, sizeof(UINT), PTC_MAX_COARSE_CULL_TILE_SIZE);
  StridedCoarseCullCounter_Srv = Devices->createShaderResourceView(
      StridedCoarseCullCounterBuffer, ViewDesc);
  FAIL_NULL_RETURN(StridedCoarseCullCounter_Srv);
  StridedCoarseCullCounter_Uav = Devices->createUnorderedAccessView(
      StridedCoarseCullCounterBuffer, ViewDesc);
  FAIL_NULL_RETURN(StridedCoarseCullCounter_Uav);

  // D3D11_BUFFER_UAV_FLAG_COUNTER
  ViewDesc = Structured(sizeof(RS_ALIVE_INDEX_BUFFER_ELEMENT),
                        PTC_MAX_PARTICLE_SIZE);
  AliveIndex_Srv = Devices->createShaderResourceView(AliveIndexBuffer, ViewDesc);
  FAIL_NULL_RETURN(AliveIndex_Srv);
  ViewDesc.WithCounter = true;
  AliveIndex_Uav =
      Devices->createUnorderedAccessView(AliveIndexBuffer, ViewDesc);
  FAIL_NULL_RETURN(AliveIndex_Uav);

  UINT NumElements = TilingConstant.NumTilesX * TilingConstant.NumTilesY *
                     PTC_TILE_BUFFER_SIZE;
  ViewDesc = Typed(VK_FORMAT_R32_UINT, sizeof(UINT), NumElements);
  TiledIndex_Srv = Devices->createShaderResourceView(TiledIndexBuffer, ViewDesc);
  FAIL_NULL_RETURN(TiledIndex_Srv);
  TiledIndex_Uav =
      Devices->createUnorderedAccessView(TiledIndexBuffer, ViewDesc);
  FAIL_NULL_RETURN(TiledIndex_Uav);

  NumElements = TilingConstant.NumTilesX * TilingConstant.NumTilesY *
                PTC_TILE_X_SIZE * PTC_TILE_Y_SIZE;
  ViewDesc = Typed(VK_FORMAT_R16G16B16A16_SFLOAT, 8, NumElements);
  ParticleRender_Srv =
      Devices->createShaderResourceView(ParticleRenderBuffer, ViewDesc);
  FAIL_NULL_RETURN(ParticleRender_Srv);
  ParticleRender_Uav =
      Devices->createUnorderedAccessView(ParticleRenderBuffer, ViewDesc);
  FAIL_NULL_RETURN(ParticleRender_Uav);

  ParticleRandom_Srv = Devices->createShaderResourceView(ParticleRandomTexture);
  FAIL_NULL_RETURN(ParticleRandom_Srv);

  SimulEmitterStructedBuffer_Srv =
      createStructuredSrv(SimulEmitterStructedBuffer,
                          sizeof(SIMULATE_EMITTER_INFO), MAX_PARTICLE_EMITTER_SIZE);
  FAIL_NULL_RETURN(SimulEmitterStructedBuffer_Srv);

  return true;
}

RSPass_PriticleEmitSimulate::RSPass_PriticleEmitSimulate(std::string &Name,
                                                         PASS_TYPE Type,
                                                         RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), RSParticleContainerPtr(nullptr),
      InitDeadListShader(nullptr), ResetParticlesShader(nullptr),
      EmitParticleShader(nullptr), SimulateShader(nullptr),
      DepthTex_Srv(nullptr), RandomTex_Srv(nullptr),
      SimulEmitterStructedBuffer_Srv(nullptr), DeadList_Uav(nullptr),
      PartA_Uav(nullptr), PartB_Uav(nullptr), AliveIndex_Uav(nullptr),
      ViewSpacePos_Uav(nullptr), MaxRadius_Uav(nullptr),
      EmitterConstantBuffer(nullptr), CameraConstantBuffer(nullptr),
      DeadListConstantBuffer(nullptr), SimulEmitterStructedBuffer(nullptr),
      TimeConstantBuffer(nullptr), LinearWrapSampler(nullptr),
      RSCameraInfo(nullptr) {}

RSPass_PriticleEmitSimulate::RSPass_PriticleEmitSimulate(
    const RSPass_PriticleEmitSimulate &Source)
    : RSPass_Base(Source),
      RSParticleContainerPtr(Source.RSParticleContainerPtr),
      InitDeadListShader(Source.InitDeadListShader),
      ResetParticlesShader(Source.ResetParticlesShader),
      EmitParticleShader(Source.EmitParticleShader),
      SimulateShader(Source.SimulateShader), DepthTex_Srv(Source.DepthTex_Srv),
      RandomTex_Srv(Source.RandomTex_Srv),
      SimulEmitterStructedBuffer_Srv(Source.SimulEmitterStructedBuffer_Srv),
      DeadList_Uav(Source.DeadList_Uav), PartA_Uav(Source.PartA_Uav),
      PartB_Uav(Source.PartB_Uav), AliveIndex_Uav(Source.AliveIndex_Uav),
      ViewSpacePos_Uav(Source.ViewSpacePos_Uav),
      MaxRadius_Uav(Source.MaxRadius_Uav),
      EmitterConstantBuffer(Source.EmitterConstantBuffer),
      CameraConstantBuffer(Source.CameraConstantBuffer),
      DeadListConstantBuffer(Source.DeadListConstantBuffer),
      SimulEmitterStructedBuffer(Source.SimulEmitterStructedBuffer),
      TimeConstantBuffer(Source.TimeConstantBuffer),
      LinearWrapSampler(Source.LinearWrapSampler),
      RSCameraInfo(Source.RSCameraInfo) {
  if (HasBeenInited) {
    RS_ADDREF(InitDeadListShader);
    RS_ADDREF(ResetParticlesShader);
    RS_ADDREF(EmitParticleShader);
    RS_ADDREF(SimulateShader);
    RS_ADDREF(LinearWrapSampler);
  }
}

RSPass_PriticleEmitSimulate::~RSPass_PriticleEmitSimulate() {}

RSPass_PriticleEmitSimulate *RSPass_PriticleEmitSimulate::clonePass() {
  return new RSPass_PriticleEmitSimulate(*this);
}

bool RSPass_PriticleEmitSimulate::initPass() {
  if (HasBeenInited) {
    return true;
  }

  RSParticleContainerPtr = getRSDX11RootInstance()->getParticlesContainer();
  if (!RSParticleContainerPtr) {
    return false;
  }

  RSCameraInfo =
      getRSDX11RootInstance()->getCamerasContainer()->getRSCameraInfo(
          "temp-cam");
  if (!RSCameraInfo) {
    return false;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createSampler()) {
    return false;
  }
  if (!checkResources()) {
    return false;
  }

  RSParticleContainerPtr->resetRSParticleSystem();

  HasBeenInited = true;

  return true;
}

void RSPass_PriticleEmitSimulate::releasePass() {
  RS_RELEASE(SimulateShader);
  RS_RELEASE(EmitParticleShader);
  RS_RELEASE(ResetParticlesShader);
  RS_RELEASE(InitDeadListShader);
  RS_RELEASE(LinearWrapSampler);
}

void RSPass_PriticleEmitSimulate::execuatePass() {
  if (!RSParticleContainerPtr->getAllParticleEmitters()->size()) {
    return;
  }

  if (RSParticleContainerPtr->getResetFlg()) {
    {
      context()->setPipelineState(InitDeadListShader);
      RSUnorderedAccessView *Uav[] = {DeadList_Uav};
      UINT InitialCount[] = {0};
      context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0,
                                         static_cast<UINT>(std::size(Uav)), Uav,
                                         InitialCount);

      context()->dispatch(rs_tool::align(PTC_MAX_PARTICLE_SIZE, 256) / 256, 1,
                          1);

      std::fill(std::begin(Uav), std::end(Uav), nullptr);
      context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0,
                                         static_cast<UINT>(std::size(Uav)),
                                         Uav);
    }

    {
      context()->setPipelineState(ResetParticlesShader);
      RSUnorderedAccessView *Uav[] = {PartA_Uav, PartB_Uav};
      UINT InitialCount[] = {static_cast<UINT>(-1), static_cast<UINT>(-1)};
      context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0,
                                         static_cast<UINT>(std::size(Uav)), Uav,
                                         InitialCount);

      context()->dispatch(rs_tool::align(PTC_MAX_PARTICLE_SIZE, 256) / 256, 1,
                          1);

      std::fill(std::begin(Uav), std::end(Uav), nullptr);
      context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0,
                                         static_cast<UINT>(std::size(Uav)),
                                         Uav);
    }

    RSParticleContainerPtr->finishResetRSParticleSystem();
  }

  {
    context()->setPipelineState(EmitParticleShader);
    RSUnorderedAccessView *Uav[] = {PartA_Uav, PartB_Uav, DeadList_Uav};
    RSShaderResourceView *Srv[] = {RandomTex_Srv};
    RSBuffer *CBuffer[] = {EmitterConstantBuffer, DeadListConstantBuffer,
                           TimeConstantBuffer};
    RSSampler *Sam[] = {LinearWrapSampler};
    UINT InitialCount[] = {static_cast<UINT>(-1), static_cast<UINT>(-1),
                           static_cast<UINT>(-1)};
    context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0,
                                       static_cast<UINT>(std::size(Uav)), Uav,
                                       InitialCount);
    context()->setShaderResources(RS_SHADER_STAGE::COMPUTE, 0,
                                  static_cast<UINT>(std::size(Srv)), Srv);
    context()->setConstantBuffers(RS_SHADER_STAGE::COMPUTE, 0,
                                  static_cast<UINT>(std::size(CBuffer)),
                                  CBuffer);
    context()->setSamplers(RS_SHADER_STAGE::COMPUTE, 0,
                           static_cast<UINT>(std::size(Sam)), Sam);

    auto Emitters = RSParticleContainerPtr->getAllParticleEmitters();
    PTC_TIME_CONSTANT *Time =
        static_cast<PTC_TIME_CONSTANT *>(TimeConstantBuffer->map());
    static float Timer = 0.f;
    Time->DeltaTime = G_DeltaTimeInSecond;
    Timer += G_DeltaTimeInSecond;
    Time->TotalTime = Timer;
    TimeConstantBuffer->unmap();
    for (auto &Emitter : *Emitters) {
      auto &RSInfo = Emitter->getRSParticleEmitterInfo();
      RSInfo.Accumulation += RSInfo.EmitNumPerSecond * G_DeltaTimeInSecond;
      if (RSInfo.Accumulation > 1.f) {
        float IntegerPart = 0.0f;
        float Fraction = std::modf(RSInfo.Accumulation, &IntegerPart);
        RSInfo.EmitSize = (int)IntegerPart;
        RSInfo.Accumulation = Fraction;
      }

      RS_PARTICLE_EMITTER_INFO *EmitterCon =
          static_cast<RS_PARTICLE_EMITTER_INFO *>(EmitterConstantBuffer->map());
      EmitterCon->EmitterIndex = RSInfo.EmitterIndex;
      EmitterCon->EmitNumPerSecond = RSInfo.EmitNumPerSecond;
      EmitterCon->EmitSize = RSInfo.EmitSize;
      EmitterCon->Accumulation = RSInfo.Accumulation;
      EmitterCon->Position = RSInfo.Position;
      EmitterCon->Velocity = RSInfo.Velocity;
      EmitterCon->PosVariance = RSInfo.PosVariance;
      EmitterCon->VelVariance = RSInfo.VelVariance;
      EmitterCon->Acceleration = RSInfo.Acceleration;
      EmitterCon->ParticleMass = RSInfo.ParticleMass;
      EmitterCon->LifeSpan = RSInfo.LifeSpan;
      EmitterCon->StartSize = RSInfo.StartSize;
      EmitterCon->EndSize = RSInfo.EndSize;
      EmitterCon->StartColor = RSInfo.StartColor;
      EmitterCon->EndColor = RSInfo.EndColor;
      EmitterCon->TextureID = RSInfo.TextureID;
      EmitterCon->StreakFlag = RSInfo.StreakFlag;
      EmitterCon->MiscFlag = RSInfo.MiscFlag;
      EmitterConstantBuffer->unmap();
      context()->copyStructureCount(DeadListConstantBuffer, 0, DeadList_Uav);

      int ThreadGroupNum = rs_tool::align(RSInfo.EmitSize, 1024) / 1024;
      context()->dispatch(ThreadGroupNum, 1, 1);
    }

    std::fill(std::begin(Uav), std::end(Uav), nullptr);
    context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0,
                                       static_cast<UINT>(std::size(Uav)), Uav);
    std::fill(std::begin(Srv), std::end(Srv), nullptr);
    context()->setShaderResources(RS_SHADER_STAGE::COMPUTE, 0,
                                  static_cast<UINT>(std::size(Srv)), Srv);
  }

  {
    dx::XMMATRIX Mat = {};

    CAMERA_STATUS *CamStatus =
        static_cast<CAMERA_STATUS *>(CameraConstantBuffer->map());
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->View), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvViewMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->InvView), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->Proj), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->InvProj), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->ViewProj), Mat);
    CamStatus->EyePosition = RSCameraInfo->EyePosition;
    CameraConstantBuffer->unmap();

    static auto EmitterArray = RSParticleContainerPtr->getAllParticleEmitters();
    auto Size = EmitterArray->size();
    SIMULATE_EMITTER_INFO *Emitter =
        static_cast<SIMULATE_EMITTER_INFO *>(SimulEmitterStructedBuffer->map());
    for (size_t I = 0; I < Size; I++) {
      Emitter[I].WorldPosition =
          (*(*EmitterArray)[I]).getRSParticleEmitterInfo().Position;
    }
    SimulEmitterStructedBuffer->unmap(Size * sizeof(SIMULATE_EMITTER_INFO));

    RSBuffer *CBuf[] = {CameraConstantBuffer, TimeConstantBuffer};
    RSShaderResourceView *Srv[] = {DepthTex_Srv,
                                   SimulEmitterStructedBuffer_Srv};
    RSUnorderedAccessView *Uav[] = {PartA_Uav,        PartB_Uav,
                                    DeadList_Uav,     AliveIndex_Uav,
                                    ViewSpacePos_Uav, MaxRadius_Uav};
    UINT InitialCount[] = {static_cast<UINT>(-1), static_cast<UINT>(-1),
                           static_cast<UINT>(-1), 0,
                           static_cast<UINT>(-1), static_cast<UINT>(-1)};

    context()->setPipelineState(SimulateShader);
    context()->setConstantBuffers(RS_SHADER_STAGE::COMPUTE, 0,
                                  static_cast<UINT>(std::size(CBuf)), CBuf);
    context()->setShaderResources(RS_SHADER_STAGE::COMPUTE, 0,
                                  static_cast<UINT>(std::size(Srv)), Srv);
    context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0,
                                       static_cast<UINT>(std::size(Uav)), Uav,
                                       InitialCount);
    static int ThreadGroupNum =
        rs_tool::align(PTC_MAX_PARTICLE_SIZE, 256) / 256;
    context()->dispatch(ThreadGroupNum, 1, 1);

    std::fill(std::begin(Uav), std::end(Uav), nullptr);
    context()->setUnorderedAccessViews(RS_SHADER_STAGE::COMPUTE, 0,
                                       static_cast<UINT>(std::size(Uav)), Uav);
    std::fill(std::begin(Srv), std::end(Srv), nullptr);
    context()->setShaderResources(RS_SHADER_STAGE::COMPUTE, 0,
                                  static_cast<UINT>(std::size(Srv)), Srv);
  }
}

bool RSPass_PriticleEmitSimulate::createSampler() {
  LinearWrapSampler = G_RSRoot->getDevices()->createSampler(
      makeLinearSamplerInfo(VK_SAMPLER_ADDRESS_MODE_REPEAT));
  FAIL_NULL_RETURN(LinearWrapSampler);

  return true;
}

bool RSPass_PriticleEmitSimulate::createShaders() {
  InitDeadListShader =
      compileComputePipeline("./Assets/Shaders/ptc_init_compute.hlsl", "Main");
  FAIL_NULL_RETURN(InitDeadListShader);

  ResetParticlesShader =
      compileComputePipeline("./Assets/Shaders/ptc_reset_compute.hlsl", "Main");
  FAIL_NULL_RETURN(ResetParticlesShader);

  EmitParticleShader =
      compileComputePipeline("./Assets/Shaders/ptc_emit_compute.hlsl", "Main");
  FAIL_NULL_RETURN(EmitParticleShader);

  SimulateShader = compileComputePipeline(
      "./Assets/Shaders/ptc_simulate_compute.hlsl", "Main");
  FAIL_NULL_RETURN(SimulateShader);

  return true;
}

bool RSPass_PriticleEmitSimulate::checkResources() {
  auto ResourceManager = getRSDX11RootInstance()->getResourceManager();
  if (!ResourceManager) {
    return false;
  }

  DeadList_Uav = ResourceManager->getResource(PTC_DEAD_LIST_NAME)->Uav;
  if (!DeadList_Uav) {
    return false;
  }

  PartA_Uav = ResourceManager->getResource(PTC_A_NAME)->Uav;
  if (!PartA_Uav) {
    return false;
  }

  PartB_Uav = ResourceManager->getResource(PTC_B_NAME)->Uav;
  if (!PartB_Uav) {
    return false;
  }

  RandomTex_Srv = ResourceManager->getResource(PTC_RAMDOM_TEXTURE_NAME)->Srv;
  if (!RandomTex_Srv) {
    return false;
  }

  EmitterConstantBuffer =
      ResourceManager->getResource(PTC_EMITTER_CONSTANT_NAME)->Resource.Buffer;
  if (!EmitterConstantBuffer) {
    return false;
  }

  DeadListConstantBuffer =
      ResourceManager->getResource(PTC_DEAD_LIST_CONSTANT_NAME)
          ->Resource.Buffer;
  if (!DeadListConstantBuffer) {
    return false;
  }

  CameraConstantBuffer =
      ResourceManager->getResource(PTC_CAMERA_CONSTANT_NAME)->Resource.Buffer;
  if (!CameraConstantBuffer) {
    return false;
  }

  SimulEmitterStructedBuffer_Srv =
      ResourceManager->getResource(PTC_SIMU_EMITTER_STRU_NAME)->Srv;
  SimulEmitterStructedBuffer =
      ResourceManager->getResource(PTC_SIMU_EMITTER_STRU_NAME)->Resource.Buffer;
  if (!SimulEmitterStructedBuffer_Srv || !SimulEmitterStructedBuffer) {
    return false;
  }

  AliveIndex_Uav = ResourceManager->getResource(PTC_ALIVE_INDEX_NAME)->Uav;
  if (!AliveIndex_Uav) {
    return false;
  }

  ViewSpacePos_Uav =
      ResourceManager->getResource(PTC_VIEW_SPCACE_POS_NAME)->Uav;
  if (!ViewSpacePos_Uav) {
    return false;
  }

  MaxRadius_Uav = ResourceManager->getResource(PTC_MAX_RADIUS_NAME)->Uav;
  if (!MaxRadius_Uav) {
    return false;
  }

  TimeConstantBuffer =
      ResourceManager->getResource(PTC_TIME_CONSTANT_NAME)->Resource.Buffer;
  if (!TimeConstantBuffer) {
    return false;
  }

  DepthTex_Srv = ResourceManager->getResource("mrt-depth")->Srv;
  if (!DepthTex_Srv) {
    return false;
  }

  return true;
}

RSPass_PriticleTileRender::RSPass_PriticleTileRender(std::string &Name,
                                                     PASS_TYPE Type,
                                                     RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), CoarseCullingShader(nullptr),
      TileCullingShader(nullptr), TileRenderShader(nullptr),
      BlendVertexShader(nullptr), BlendPixelShader(nullptr),
      CameraConstantBuffer(nullptr), TilingConstantBuffer(nullptr),
      ActiveListConstantBuffer(nullptr), DepthTex_Srv(nullptr),
      ViewSpacePos_Srv(nullptr), MaxRadius_Srv(nullptr), PartA_Srv(nullptr),
      AliveIndex_Srv(nullptr), AliveIndex_Uav(nullptr),
      CoarseTileIndex_Srv(nullptr), CoarseTileIndex_Uav(nullptr),
      CoarseTileIndexCounter_Srv(nullptr), CoarseTileIndexCounter_Uav(nullptr),
      TiledIndex_Srv(nullptr), TiledIndex_Uav(nullptr),
      ParticleRender_Srv(nullptr), ParticleRender_Uav(nullptr),
      LinearClampSampler(nullptr), ParticleBlendState(nullptr),
      ParticleTex_Srv(nullptr), RSCameraInfo(nullptr) {}

RSPass_PriticleTileRender::RSPass_PriticleTileRender(
    const RSPass_PriticleTileRender &Source)
    : RSPass_Base(Source), CoarseCullingShader(Source.CoarseCullingShader),
      TileCullingShader(Source.TileCullingShader),
      TileRenderShader(Source.TileRenderShader),
      BlendVertexShader(Source.BlendVertexShader),
      BlendPixelShader(Source.BlendPixelShader),
      CameraConstantBuffer(Source.CameraConstantBuffer),
      TilingConstantBuffer(Source.TilingConstantBuffer),
      ActiveListConstantBuffer(Source.ActiveListConstantBuffer),
      DepthTex_Srv(Source.DepthTex_Srv),
      ViewSpacePos_Srv(Source.ViewSpacePos_Srv),
      MaxRadius_Srv(Source.MaxRadius_Srv), PartA_Srv(Source.PartA_Srv),
      AliveIndex_Srv(Source.AliveIndex_Srv),
      AliveIndex_Uav(Source.AliveIndex_Uav),
      CoarseTileIndex_Srv(Source.CoarseTileIndex_Srv),
      CoarseTileIndex_Uav(Source.CoarseTileIndex_Uav),
      CoarseTileIndexCounter_Srv(Source.CoarseTileIndexCounter_Srv),
      CoarseTileIndexCounter_Uav(Source.CoarseTileIndexCounter_Uav),
      TiledIndex_Srv(Source.TiledIndex_Srv),
      TiledIndex_Uav(Source.TiledIndex_Uav),
      ParticleRender_Srv(Source.ParticleRender_Srv),
      ParticleRender_Uav(Source.ParticleRender_Uav),
      LinearClampSampler(Source.LinearClampSampler),
      ParticleBlendState(Source.ParticleBlendState),
      ParticleTex_Srv(Source.ParticleTex_Srv),
      RSCameraInfo(Source.RSCameraInfo) {}

RSPass_PriticleTileRender::~RSPass_PriticleTileRender() {}

RSPass_PriticleTileRender *RSPass_PriticleTileRender::clonePass() {
  return new RSPass_PriticleTileRender(*this);
}

bool RSPass_PriticleTileRender::initPass() {
  if (HasBeenInited) {
    return true;
  }

  RSCameraInfo =
      getRSDX11RootInstance()->getCamerasContainer()->getRSCameraInfo(
          "temp-cam");
  if (!RSCameraInfo) {
    return false;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSampler()) {
    return false;
  }
  if (!createBlend()) {
    return false;
  }
  if (!checkResources()) {
    return false;
  }

  HasBeenInited = true;

  return true;
}

void RSPass_PriticleTileRender::releasePass() {
  RS_RELEASE(CoarseCullingShader);
  RS_RELEASE(TileCullingShader);
  RS_RELEASE(TileRenderShader);
  RS_RELEASE(BlendVertexShader);
  RS_RELEASE(BlendPixelShader);
  RS_RELEASE(ParticleBlendState);

  RS_RELEASE(LinearClampSampler);

  RS_RELEASE(ParticleTex_Srv);
}

void RSPass_PriticleTileRender::execuatePass() {
  if (!G_RSRoot->getParticlesContainer()->getAllParticleEmitters()->size()) {
    return;
  }

  {
    D3D11_MAPPED_SUBRESOURCE Msr = {};
    dx::XMMATRIX Mat = {};
    context()->Map(CameraConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    CAMERA_STATUS *CamStatus = static_cast<CAMERA_STATUS *>(Msr.pData);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->View), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvViewMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->InvView), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->Proj), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->InvProj), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->ViewProj), Mat);
    CamStatus->EyePosition = RSCameraInfo->EyePosition;
    context()->Unmap(CameraConstantBuffer, 0);
    context()->Map(TilingConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    RS_TILING_CONSTANT *Tiling = static_cast<RS_TILING_CONSTANT *>(Msr.pData);
    Tiling->NumTilesX = G_ParticleSetUpPass->getTilingConstantInfo().NumTilesX;
    Tiling->NumTilesY = G_ParticleSetUpPass->getTilingConstantInfo().NumTilesY;
    Tiling->NumCoarseCullingTilesX =
        G_ParticleSetUpPass->getTilingConstantInfo().NumCoarseCullingTilesX;
    Tiling->NumCoarseCullingTilesY =
        G_ParticleSetUpPass->getTilingConstantInfo().NumCoarseCullingTilesY;
    Tiling->NumCullingTilesPerCoarseTileX =
        G_ParticleSetUpPass->getTilingConstantInfo()
            .NumCullingTilesPerCoarseTileX;
    Tiling->NumCullingTilesPerCoarseTileY =
        G_ParticleSetUpPass->getTilingConstantInfo()
            .NumCullingTilesPerCoarseTileY;
    context()->Unmap(TilingConstantBuffer, 0);
    context()->CopyStructureCount(ActiveListConstantBuffer, 0, AliveIndex_Uav);

    ID3D11Buffer *CBuf[] = {CameraConstantBuffer, TilingConstantBuffer,
                            ActiveListConstantBuffer};
    ID3D11ShaderResourceView *Srv[] = {ViewSpacePos_Srv, MaxRadius_Srv,
                                       AliveIndex_Srv};
    ID3D11UnorderedAccessView *Uav[] = {CoarseTileIndex_Uav,
                                        CoarseTileIndexCounter_Uav};
    UINT Initial[] = {static_cast<UINT>(-1), static_cast<UINT>(-1)};

    context()->CSSetShader(CoarseCullingShader, nullptr, 0);
    context()->CSSetConstantBuffers(0, ARRAYSIZE(CBuf), CBuf);
    context()->CSSetShaderResources(0, ARRAYSIZE(Srv), Srv);
    context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(Uav), Uav, Initial);

    static int ThreadGroupNum =
        rs_tool::align(PTC_MAX_PARTICLE_SIZE, PTC_COARSE_CULLING_THREADS) /
        PTC_COARSE_CULLING_THREADS;
    context()->Dispatch(ThreadGroupNum, 1, 1);

    ZeroMemory(CBuf, sizeof(CBuf));
    ZeroMemory(Srv, sizeof(Srv));
    ZeroMemory(Uav, sizeof(Uav));
    context()->CSSetConstantBuffers(0, ARRAYSIZE(CBuf), CBuf);
    context()->CSSetShaderResources(0, ARRAYSIZE(Srv), Srv);
    context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(Uav), Uav, nullptr);
  }

  {
    D3D11_MAPPED_SUBRESOURCE Msr = {};
    dx::XMMATRIX Mat = {};
    context()->Map(CameraConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    CAMERA_STATUS *CamStatus = static_cast<CAMERA_STATUS *>(Msr.pData);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->View), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvViewMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->InvView), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->Proj), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->InvProj), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->ViewProj), Mat);
    CamStatus->EyePosition = RSCameraInfo->EyePosition;
    context()->Unmap(CameraConstantBuffer, 0);
    context()->Map(TilingConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    RS_TILING_CONSTANT *Tiling = static_cast<RS_TILING_CONSTANT *>(Msr.pData);
    Tiling->NumTilesX = G_ParticleSetUpPass->getTilingConstantInfo().NumTilesX;
    Tiling->NumTilesY = G_ParticleSetUpPass->getTilingConstantInfo().NumTilesY;
    Tiling->NumCoarseCullingTilesX =
        G_ParticleSetUpPass->getTilingConstantInfo().NumCoarseCullingTilesX;
    Tiling->NumCoarseCullingTilesY =
        G_ParticleSetUpPass->getTilingConstantInfo().NumCoarseCullingTilesY;
    Tiling->NumCullingTilesPerCoarseTileX =
        G_ParticleSetUpPass->getTilingConstantInfo()
            .NumCullingTilesPerCoarseTileX;
    Tiling->NumCullingTilesPerCoarseTileY =
        G_ParticleSetUpPass->getTilingConstantInfo()
            .NumCullingTilesPerCoarseTileY;
    context()->Unmap(TilingConstantBuffer, 0);

    ID3D11Buffer *CBuf[] = {CameraConstantBuffer, TilingConstantBuffer,
                            ActiveListConstantBuffer};
    ID3D11ShaderResourceView *Srv[] = {
        ViewSpacePos_Srv, MaxRadius_Srv,       AliveIndex_Srv,
        DepthTex_Srv,     CoarseTileIndex_Srv, CoarseTileIndexCounter_Srv};
    ID3D11UnorderedAccessView *Uav[] = {TiledIndex_Uav};
    UINT initial[] = {static_cast<UINT>(-1)};

    context()->CSSetShader(TileCullingShader, nullptr, 0);
    context()->CSSetConstantBuffers(0, ARRAYSIZE(CBuf), CBuf);
    context()->CSSetShaderResources(0, ARRAYSIZE(Srv), Srv);
    context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(Uav), Uav, initial);

    context()->Dispatch(G_ParticleSetUpPass->getTilingConstantInfo().NumTilesX,
                        G_ParticleSetUpPass->getTilingConstantInfo().NumTilesY,
                        1);

    ZeroMemory(CBuf, sizeof(CBuf));
    ZeroMemory(Srv, sizeof(Srv));
    ZeroMemory(Uav, sizeof(Uav));
    context()->CSSetConstantBuffers(0, ARRAYSIZE(CBuf), CBuf);
    context()->CSSetShaderResources(0, ARRAYSIZE(Srv), Srv);
    context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(Uav), Uav, nullptr);
  }

  {
    D3D11_MAPPED_SUBRESOURCE Msr = {};
    dx::XMMATRIX Mat = {};
    context()->Map(CameraConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    CAMERA_STATUS *CamStatus = static_cast<CAMERA_STATUS *>(Msr.pData);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->View), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvViewMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->InvView), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->Proj), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->InvProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->InvProj), Mat);
    Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewProjMatrix);
    Mat = dx::XMMatrixTranspose(Mat);
    dx::XMStoreFloat4x4(&(CamStatus->ViewProj), Mat);
    CamStatus->EyePosition = RSCameraInfo->EyePosition;
    context()->Unmap(CameraConstantBuffer, 0);
    context()->Map(TilingConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    RS_TILING_CONSTANT *Tiling = static_cast<RS_TILING_CONSTANT *>(Msr.pData);
    Tiling->NumTilesX = G_ParticleSetUpPass->getTilingConstantInfo().NumTilesX;
    Tiling->NumTilesY = G_ParticleSetUpPass->getTilingConstantInfo().NumTilesY;
    Tiling->NumCoarseCullingTilesX =
        G_ParticleSetUpPass->getTilingConstantInfo().NumCoarseCullingTilesX;
    Tiling->NumCoarseCullingTilesY =
        G_ParticleSetUpPass->getTilingConstantInfo().NumCoarseCullingTilesY;
    Tiling->NumCullingTilesPerCoarseTileX =
        G_ParticleSetUpPass->getTilingConstantInfo()
            .NumCullingTilesPerCoarseTileX;
    Tiling->NumCullingTilesPerCoarseTileY =
        G_ParticleSetUpPass->getTilingConstantInfo()
            .NumCullingTilesPerCoarseTileY;
    context()->Unmap(TilingConstantBuffer, 0);

    ID3D11Buffer *CBuf[] = {CameraConstantBuffer, TilingConstantBuffer};
    ID3D11ShaderResourceView *Srv[] = {
        PartA_Srv,      ViewSpacePos_Srv,           DepthTex_Srv,
        TiledIndex_Srv, CoarseTileIndexCounter_Srv, ParticleTex_Srv};
    ID3D11UnorderedAccessView *Uav[] = {ParticleRender_Uav};
    UINT Initial[] = {static_cast<UINT>(-1)};
    ID3D11SamplerState *Sam[] = {LinearClampSampler};

    context()->CSSetShader(TileRenderShader, nullptr, 0);
    context()->CSSetConstantBuffers(0, ARRAYSIZE(CBuf), CBuf);
    context()->CSSetShaderResources(0, ARRAYSIZE(Srv), Srv);
    context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(Uav), Uav, Initial);
    context()->CSSetSamplers(0, ARRAYSIZE(Sam), Sam);

    context()->Dispatch(G_ParticleSetUpPass->getTilingConstantInfo().NumTilesX,
                        G_ParticleSetUpPass->getTilingConstantInfo().NumTilesY,
                        1);

    ZeroMemory(CBuf, sizeof(CBuf));
    ZeroMemory(Srv, sizeof(Srv));
    ZeroMemory(Uav, sizeof(Uav));
    context()->CSSetConstantBuffers(0, ARRAYSIZE(CBuf), CBuf);
    context()->CSSetShaderResources(0, ARRAYSIZE(Srv), Srv);
    context()->CSSetUnorderedAccessViews(0, ARRAYSIZE(Uav), Uav, nullptr);
    context()->CSSetShader(nullptr, nullptr, 0);
  }

  {
    static auto Rtv =
        getRSDX11RootInstance()->getDevices()->getHighDynamicRtv();
    static D3D11_VIEWPORT VP = {};
    VP.Width = 1280.f;
    VP.Height = 720.f;
    VP.MinDepth = 0.f;
    VP.MaxDepth = 1.f;
    VP.TopLeftX = 0.f;
    VP.TopLeftY = 0.f;
    context()->VSSetShader(BlendVertexShader, nullptr, 0);
    context()->PSSetShader(BlendPixelShader, nullptr, 0);
    context()->OMSetBlendState(ParticleBlendState, nullptr, 0xFFFFFFFF);
    context()->OMSetRenderTargets(1, &Rtv, nullptr);
    context()->RSSetViewports(1, &VP);
    context()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
    context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11ShaderResourceView *Srv[] = {ParticleRender_Srv};
    context()->PSSetShaderResources(0, ARRAYSIZE(Srv), Srv);

    context()->Draw(3, 0);

    ZeroMemory(Srv, sizeof(Srv));
    context()->PSSetShaderResources(0, ARRAYSIZE(Srv), Srv);
    context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    static ID3D11RenderTargetView *NullRtv = nullptr;
    context()->OMSetRenderTargets(1, &NullRtv, nullptr);
  }
}

bool RSPass_PriticleTileRender::createShaders() {
  ID3DBlob *ShaderBlob = nullptr;
  HRESULT Hr = S_OK;

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/ptc_coarse_compute.hlsl", "Main", "cs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &CoarseCullingShader);
  RS_RELEASE(ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/ptc_cull_compute.hlsl", "Main", "cs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &TileCullingShader);
  RS_RELEASE(ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/ptc_render_compute.hlsl", "Main", "cs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &TileRenderShader);
  RS_RELEASE(ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/ptc_blend_vertex.hlsl", "Main", "vs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateVertexShader(ShaderBlob->GetBufferPointer(),
                                    ShaderBlob->GetBufferSize(), nullptr,
                                    &BlendVertexShader);
  RS_RELEASE(ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/ptc_blend_pixel.hlsl", "Main", "ps_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreatePixelShader(ShaderBlob->GetBufferPointer(),
                                   ShaderBlob->GetBufferSize(), nullptr,
                                   &BlendPixelShader);
  RS_RELEASE(ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_PriticleTileRender::createViews() {
  HRESULT Hr = S_OK;

  Hr = dx::CreateDDSTextureFromFile(device(),
                                    L"./Assets/Textures/particle_atlas.dds",
                                    nullptr, &ParticleTex_Srv);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_PriticleTileRender::createSampler() {
  HRESULT Hr = S_OK;
  D3D11_SAMPLER_DESC SampDesc = {};
  ZeroMemory(&SampDesc, sizeof(SampDesc));
  SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  SampDesc.MinLOD = 0;
  SampDesc.MaxLOD = D3D11_FLOAT32_MAX;

  Hr = device()->CreateSamplerState(&SampDesc, &LinearClampSampler);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_PriticleTileRender::createBlend() {
  HRESULT Hr = S_OK;

  D3D11_BLEND_DESC BldDesc = {};
  ZeroMemory(&BldDesc, sizeof(D3D11_BLEND_DESC));
  BldDesc.AlphaToCoverageEnable = false;
  BldDesc.IndependentBlendEnable = false;
  BldDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  BldDesc.RenderTarget[0].BlendEnable = true;
  BldDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
  BldDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  BldDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  BldDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
  BldDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  BldDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  Hr = device()->CreateBlendState(&BldDesc, &ParticleBlendState);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_PriticleTileRender::checkResources() {
  auto ResourceManager = getRSDX11RootInstance()->getResourceManager();
  if (!ResourceManager) {
    return false;
  }

  CameraConstantBuffer =
      ResourceManager->getResource(PTC_CAMERA_CONSTANT_NAME)->Resource.Buffer;
  if (!CameraConstantBuffer) {
    return false;
  }

  TilingConstantBuffer =
      ResourceManager->getResource(PTC_TILING_CONSTANT_NAME)->Resource.Buffer;
  if (!TilingConstantBuffer) {
    return false;
  }

  ActiveListConstantBuffer =
      ResourceManager->getResource(PTC_ALIVE_LIST_CONSTANT_NAME)
          ->Resource.Buffer;
  if (!ActiveListConstantBuffer) {
    return false;
  }

  DepthTex_Srv = ResourceManager->getResource("mrt-depth")->Srv;
  if (!DepthTex_Srv) {
    return false;
  }

  ViewSpacePos_Srv =
      ResourceManager->getResource(PTC_VIEW_SPCACE_POS_NAME)->Srv;
  if (!ViewSpacePos_Srv) {
    return false;
  }

  MaxRadius_Srv = ResourceManager->getResource(PTC_MAX_RADIUS_NAME)->Srv;
  if (!MaxRadius_Srv) {
    return false;
  }

  AliveIndex_Srv = ResourceManager->getResource(PTC_ALIVE_INDEX_NAME)->Srv;
  AliveIndex_Uav = ResourceManager->getResource(PTC_ALIVE_INDEX_NAME)->Uav;
  if (!AliveIndex_Srv) {
    return false;
  }
  if (!AliveIndex_Uav) {
    return false;
  }

  PartA_Srv = ResourceManager->getResource(PTC_A_NAME)->Srv;
  if (!PartA_Srv) {
    return false;
  }

  CoarseTileIndex_Srv = ResourceManager->getResource(PTC_COARSE_CULL_NAME)->Srv;
  CoarseTileIndex_Uav = ResourceManager->getResource(PTC_COARSE_CULL_NAME)->Uav;
  if (!CoarseTileIndex_Srv) {
    return false;
  }
  if (!CoarseTileIndex_Uav) {
    return false;
  }

  CoarseTileIndexCounter_Srv =
      ResourceManager->getResource(PTC_COARSE_CULL_COUNTER_NAME)->Srv;
  CoarseTileIndexCounter_Uav =
      ResourceManager->getResource(PTC_COARSE_CULL_COUNTER_NAME)->Uav;
  if (!CoarseTileIndexCounter_Srv) {
    return false;
  }
  if (!CoarseTileIndexCounter_Uav) {
    return false;
  }

  TiledIndex_Srv = ResourceManager->getResource(PTC_TILED_INDEX_NAME)->Srv;
  TiledIndex_Uav = ResourceManager->getResource(PTC_TILED_INDEX_NAME)->Uav;
  if (!TiledIndex_Srv) {
    return false;
  }
  if (!TiledIndex_Uav) {
    return false;
  }

  ParticleRender_Srv =
      ResourceManager->getResource(PTC_RENDER_BUFFER_NAME)->Srv;
  ParticleRender_Uav =
      ResourceManager->getResource(PTC_RENDER_BUFFER_NAME)->Uav;
  if (!ParticleRender_Srv) {
    return false;
  }
  if (!ParticleRender_Uav) {
    return false;
  }

  return true;
}

RSPass_Sprite::RSPass_Sprite(std::string &Name,
                             PASS_TYPE Type,
                             RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      PixelShader(nullptr), DepthStencilState(nullptr), BlendState(nullptr),
      RenderTargetView(nullptr), DrawCallType(DRAWCALL_TYPE::MAX),
      DrawCallPipe(nullptr), ProjStructedBuffer(nullptr),
      ProjStructedBufferSrv(nullptr), InstanceStructedBuffer(nullptr),
      InstanceStructedBufferSrv(nullptr), LinearSampler(nullptr),
      RSCameraInfo(nullptr) {}

RSPass_Sprite::RSPass_Sprite(const RSPass_Sprite &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      PixelShader(Source.PixelShader),
      DepthStencilState(Source.DepthStencilState),
      BlendState(Source.BlendState), RenderTargetView(Source.RenderTargetView),
      DrawCallType(Source.DrawCallType), DrawCallPipe(Source.DrawCallPipe),
      ProjStructedBuffer(Source.ProjStructedBuffer),
      ProjStructedBufferSrv(Source.ProjStructedBufferSrv),
      InstanceStructedBuffer(Source.InstanceStructedBuffer),
      InstanceStructedBufferSrv(Source.InstanceStructedBufferSrv),
      LinearSampler(Source.LinearSampler), RSCameraInfo(Source.RSCameraInfo) {
  if (HasBeenInited) {
    RS_ADDREF(VertexShader);
    RS_ADDREF(PixelShader);
    RS_ADDREF(DepthStencilState);
    RS_ADDREF(LinearSampler);
    RS_ADDREF(ProjStructedBufferSrv);
    RS_ADDREF(ProjStructedBuffer);
    RS_ADDREF(InstanceStructedBufferSrv);
    RS_ADDREF(InstanceStructedBuffer);
  }
}

RSPass_Sprite::~RSPass_Sprite() {}

RSPass_Sprite *RSPass_Sprite::clonePass() { return new RSPass_Sprite(*this); }

bool RSPass_Sprite::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createStates()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }

  DrawCallType = DRAWCALL_TYPE::UI_SPRITE;
  DrawCallPipe = G_RSRoot->getDrawCallsPool()->getDrawCallsPipe(DrawCallType);

  RSCameraInfo =
      G_RSRoot->getCamerasContainer()->getRSCameraInfo("temp-ui-cam");

  HasBeenInited = true;

  return true;
}

void RSPass_Sprite::releasePass() {
  RS_RELEASE(VertexShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(DepthStencilState);
  RS_RELEASE(LinearSampler);
  RS_RELEASE(ProjStructedBufferSrv);
  RS_RELEASE(ProjStructedBuffer);
  RS_RELEASE(InstanceStructedBufferSrv);
  RS_RELEASE(InstanceStructedBuffer);
}

void RSPass_Sprite::execuatePass() {
  ID3D11RenderTargetView *NullRtv = nullptr;
  context()->OMSetRenderTargets(1, &RenderTargetView, nullptr);
  context()->RSSetViewports(1, &G_ViewPort);
  context()->OMSetDepthStencilState(DepthStencilState, 0);
  static float Factor[4] = {0.f, 0.f, 0.f, 0.f};
  context()->OMSetBlendState(BlendState, Factor, 0xFFFFFFFF);
  context()->VSSetShader(VertexShader, nullptr, 0);
  context()->PSSetShader(PixelShader, nullptr, 0);
  context()->PSSetSamplers(0, 1, &LinearSampler);

  dx::XMMATRIX Mat = {};
  UINT Stride = sizeof(vertex_type::TangentVertex);
  UINT Offset = 0;

  D3D11_MAPPED_SUBRESOURCE Msr = {};
  context()->Map(ProjStructedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
  OnlyProj *VpData = static_cast<OnlyProj *>(Msr.pData);
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ProjMat, Mat);
  context()->Unmap(ProjStructedBuffer, 0);

  context()->VSSetShaderResources(0, 1, &ProjStructedBufferSrv);

  for (auto &Draw : DrawCallPipe->Data) {
    auto InsArrayPtr = Draw.InstanceData.DataArrayPtr;
    auto Size = InsArrayPtr->size();
    context()->Map(InstanceStructedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    RS_INSTANCE_DATA *InsData = static_cast<RS_INSTANCE_DATA *>(Msr.pData);
    for (size_t I = 0; I < Size; I++) {
      Mat = dx::XMLoadFloat4x4(&(*InsArrayPtr)[I].WorldMatrix);
      Mat = dx::XMMatrixTranspose(Mat);
      dx::XMStoreFloat4x4(&InsData[I].WorldMatrix, Mat);
      InsData[I].MaterialData = (*InsArrayPtr)[I].MaterialData;
      InsData[I].CustomizedData1 = (*InsArrayPtr)[I].CustomizedData1;
      InsData[I].CustomizedData2 = (*InsArrayPtr)[I].CustomizedData2;
    }
    context()->Unmap(InstanceStructedBuffer, 0);

    context()->IASetInputLayout(Draw.MeshData.InputLayout);
    context()->IASetPrimitiveTopology(Draw.MeshData.TopologyType);
    context()->IASetVertexBuffers(0, 1, &Draw.MeshData.VertexBuffer, &Stride,
                                  &Offset);
    context()->IASetIndexBuffer(Draw.MeshData.IndexBuffer, DXGI_FORMAT_R32_UINT,
                                0);
    context()->VSSetShaderResources(1, 1, &InstanceStructedBufferSrv);
    context()->PSSetShaderResources(0, 1, &(Draw.TextureData[0].Srv));

    context()->DrawIndexedInstanced(
        Draw.MeshData.IndexSize,
        static_cast<UINT>(Draw.InstanceData.DataArrayPtr->size()), 0, 0, 0);
  }

  context()->OMSetRenderTargets(1, &NullRtv, nullptr);
  context()->OMSetDepthStencilState(nullptr, 0);
  context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
}

bool RSPass_Sprite::createShaders() {
  ID3DBlob *ShaderBlob = nullptr;
  HRESULT Hr = S_OK;

  Hr = rs_tool::compileShaderFromFile(L"./Assets/Shaders/sprite_vertex.hlsl",
                                      "main", "vs_5_0", &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateVertexShader(ShaderBlob->GetBufferPointer(),
                                    ShaderBlob->GetBufferSize(), nullptr,
                                    &VertexShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(L"./Assets/Shaders/sprite_pixel.hlsl",
                                      "main", "ps_5_0", &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreatePixelShader(ShaderBlob->GetBufferPointer(),
                                   ShaderBlob->GetBufferSize(), nullptr,
                                   &PixelShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Sprite::createStates() {
  HRESULT Hr = S_OK;

  D3D11_DEPTH_STENCIL_DESC DepDesc = {};
  DepDesc.DepthEnable = FALSE;
  DepDesc.StencilEnable = FALSE;
  Hr = device()->CreateDepthStencilState(&DepDesc, &DepthStencilState);
  if (FAILED(Hr)) {
    return false;
  }

  D3D11_BLEND_DESC BldDesc = {};
  BldDesc.AlphaToCoverageEnable = FALSE;
  BldDesc.IndependentBlendEnable = FALSE;
  BldDesc.RenderTarget[0].BlendEnable = TRUE;
  BldDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  BldDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  BldDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  BldDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  BldDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  BldDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  BldDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  Hr = device()->CreateBlendState(&BldDesc, &BlendState);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Sprite::createBuffers() {
  HRESULT Hr = S_OK;
  D3D11_BUFFER_DESC BufDesc = {};

  ZeroMemory(&BufDesc, sizeof(BufDesc));
  BufDesc.Usage = D3D11_USAGE_DYNAMIC;
  BufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  BufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  BufDesc.ByteWidth = MAX_INSTANCE_SIZE * sizeof(RS_INSTANCE_DATA);
  BufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  BufDesc.StructureByteStride = sizeof(RS_INSTANCE_DATA);
  Hr = device()->CreateBuffer(&BufDesc, nullptr, &InstanceStructedBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  BufDesc.ByteWidth = sizeof(OnlyProj);
  BufDesc.StructureByteStride = sizeof(OnlyProj);
  Hr = device()->CreateBuffer(&BufDesc, nullptr, &ProjStructedBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Sprite::createViews() {
  RenderTargetView = G_RSRoot->getDevices()->getSwapChainRtv();

  D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
  HRESULT Hr = S_OK;
  ZeroMemory(&SrvDesc, sizeof(SrvDesc));
  SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
  SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  SrvDesc.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
  Hr = device()->CreateShaderResourceView(InstanceStructedBuffer, &SrvDesc,
                                          &InstanceStructedBufferSrv);
  if (FAILED(Hr)) {
    return false;
  }

  SrvDesc.Buffer.ElementWidth = 1;
  Hr = device()->CreateShaderResourceView(ProjStructedBuffer, &SrvDesc,
                                          &ProjStructedBufferSrv);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Sprite::createSamplers() {
  HRESULT Hr = S_OK;
  D3D11_SAMPLER_DESC SampDesc = {};
  ZeroMemory(&SampDesc, sizeof(SampDesc));
  auto FilterLevel = G_RenderEffectConfig.SamplerLevel;
  switch (FilterLevel) {
  case SAMPLER_LEVEL::POINT:
    SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    break;
  default:
    SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    break;
  }
  SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  SampDesc.MinLOD = 0;
  SampDesc.MaxLOD = D3D11_FLOAT32_MAX;

  Hr = device()->CreateSamplerState(&SampDesc, &LinearSampler);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

RSPass_SimpleLight::RSPass_SimpleLight(std::string &Name,
                                       PASS_TYPE Type,
                                       RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      PixelShader(nullptr), RenderTargetView(nullptr),
      LinearWrapSampler(nullptr), GeoBufferSrv(nullptr), SsaoSrv(nullptr),
      VertexBuffer(nullptr), IndexBuffer(nullptr) {}

RSPass_SimpleLight::RSPass_SimpleLight(const RSPass_SimpleLight &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      PixelShader(Source.PixelShader),
      RenderTargetView(Source.RenderTargetView),
      LinearWrapSampler(Source.LinearWrapSampler),
      GeoBufferSrv(Source.GeoBufferSrv), SsaoSrv(Source.SsaoSrv),
      VertexBuffer(Source.VertexBuffer), IndexBuffer(Source.IndexBuffer) {}

RSPass_SimpleLight::~RSPass_SimpleLight() {}

RSPass_SimpleLight *RSPass_SimpleLight::clonePass() {
  return new RSPass_SimpleLight(*this);
}

bool RSPass_SimpleLight::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }

  HasBeenInited = true;

  return true;
}

void RSPass_SimpleLight::releasePass() {
  RS_RELEASE(VertexShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(LinearWrapSampler);
  RS_RELEASE(VertexBuffer);
  RS_RELEASE(IndexBuffer);
}

void RSPass_SimpleLight::execuatePass() {
  context()->OMSetRenderTargets(1, &RenderTargetView, nullptr);
  context()->RSSetViewports(1, &G_ViewPort);
  context()->ClearRenderTargetView(RenderTargetView, dx::Colors::DarkGreen);
  context()->VSSetShader(VertexShader, nullptr, 0);
  context()->PSSetShader(PixelShader, nullptr, 0);

  UINT Stride = sizeof(vertex_type::TangentVertex);
  UINT Offset = 0;

  static ID3D11ShaderResourceView *Srvs[] = {GeoBufferSrv, SsaoSrv};
  context()->PSSetShaderResources(0, 2, Srvs);

  static ID3D11SamplerState *Samps[] = {
      LinearWrapSampler,
  };
  context()->PSSetSamplers(0, 1, Samps);

  context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context()->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
  context()->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

  context()->DrawIndexedInstanced(6, 1, 0, 0, 0);

  ID3D11RenderTargetView *NullRtv = nullptr;
  context()->OMSetRenderTargets(1, &NullRtv, nullptr);
  static ID3D11ShaderResourceView *NullSrv[] = {nullptr, nullptr};
  context()->PSSetShaderResources(0, 2, NullSrv);
}

bool RSPass_SimpleLight::createShaders() {
  ID3DBlob *ShaderBlob = nullptr;
  HRESULT Hr = S_OK;

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/simplylit_vertex.hlsl", "main", "vs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateVertexShader(ShaderBlob->GetBufferPointer(),
                                    ShaderBlob->GetBufferSize(), nullptr,
                                    &VertexShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/simplylit_pixel.hlsl", "main", "ps_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreatePixelShader(ShaderBlob->GetBufferPointer(),
                                   ShaderBlob->GetBufferSize(), nullptr,
                                   &PixelShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_SimpleLight::createBuffers() {
  HRESULT Hr = S_OK;
  D3D11_BUFFER_DESC BufDesc = {};

  vertex_type::TangentVertex V[4] = {};
  V[0].Position = dx::XMFLOAT3(-1.0f, -1.0f, 0.0f);
  V[1].Position = dx::XMFLOAT3(-1.0f, +1.0f, 0.0f);
  V[2].Position = dx::XMFLOAT3(+1.0f, +1.0f, 0.0f);
  V[3].Position = dx::XMFLOAT3(+1.0f, -1.0f, 0.0f);
  V[0].TexCoord = dx::XMFLOAT2(0.0f, 1.0f);
  V[1].TexCoord = dx::XMFLOAT2(0.0f, 0.0f);
  V[2].TexCoord = dx::XMFLOAT2(1.0f, 0.0f);
  V[3].TexCoord = dx::XMFLOAT2(1.0f, 1.0f);
  ZeroMemory(&BufDesc, sizeof(BufDesc));
  BufDesc.Usage = D3D11_USAGE_IMMUTABLE;
  BufDesc.ByteWidth = sizeof(vertex_type::TangentVertex) * 4;
  BufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  BufDesc.CPUAccessFlags = 0;
  BufDesc.MiscFlags = 0;
  BufDesc.StructureByteStride = 0;
  D3D11_SUBRESOURCE_DATA VInitData = {};
  ZeroMemory(&VInitData, sizeof(VInitData));
  VInitData.pSysMem = V;
  Hr = device()->CreateBuffer(&BufDesc, &VInitData, &VertexBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  UINT Indices[6] = {0, 1, 2, 0, 2, 3};
  ZeroMemory(&BufDesc, sizeof(BufDesc));
  BufDesc.Usage = D3D11_USAGE_IMMUTABLE;
  BufDesc.ByteWidth = sizeof(UINT) * 6;
  BufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  BufDesc.CPUAccessFlags = 0;
  BufDesc.StructureByteStride = 0;
  BufDesc.MiscFlags = 0;
  D3D11_SUBRESOURCE_DATA IInitData = {};
  ZeroMemory(&IInitData, sizeof(IInitData));
  IInitData.pSysMem = Indices;
  Hr = device()->CreateBuffer(&BufDesc, &IInitData, &IndexBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_SimpleLight::createViews() {
  RenderTargetView = G_RSRoot->getDevices()->getSwapChainRtv();

  GeoBufferSrv =
      G_RSRoot->getResourceManager()->getResource("mrt-geo-buffer")->Srv;
  SsaoSrv = G_RSRoot->getResourceManager()
                ->getResource("ssao-tex-compress-ssao")
                ->Srv;

  return true;
}

bool RSPass_SimpleLight::createSamplers() {
  HRESULT Hr = S_OK;
  D3D11_SAMPLER_DESC SampDesc = {};
  ZeroMemory(&SampDesc, sizeof(SampDesc));
  auto FilterLevel = G_RenderEffectConfig.SamplerLevel;
  switch (FilterLevel) {
  case SAMPLER_LEVEL::POINT:
    SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    break;
  case SAMPLER_LEVEL::BILINEAR:
    SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    break;
  case SAMPLER_LEVEL::ANISO_8X:
    SampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SampDesc.MaxAnisotropy = 8;
    break;
  case SAMPLER_LEVEL::ANISO_16X:
    SampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SampDesc.MaxAnisotropy = 16;
    break;
  default:
    return false;
  }
  SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  SampDesc.MinLOD = 0;
  SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  Hr = device()->CreateSamplerState(&SampDesc, &LinearWrapSampler);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

RSPass_Billboard::RSPass_Billboard(std::string &Name,
                                   PASS_TYPE Type,
                                   RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      GeometryShader(nullptr), PixelShader(nullptr), BlendState(nullptr),
      RenderTargetView(nullptr), DepthStencilView(nullptr),
      LinearWrapSampler(nullptr), ViewProjStructedBuffer(nullptr),
      ViewProjStructedBufferSrv(nullptr), InstanceStructedBuffer(nullptr),
      InstanceStructedBufferSrv(nullptr),
      DrawCallType(DRAWCALL_TYPE::TRANSPARENCY), DrawCallPipe(nullptr),
      RSCameraInfo(nullptr), RSCamera(nullptr) {}

RSPass_Billboard::RSPass_Billboard(const RSPass_Billboard &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      GeometryShader(Source.GeometryShader), PixelShader(Source.PixelShader),
      BlendState(Source.BlendState), RenderTargetView(Source.RenderTargetView),
      DepthStencilView(Source.DepthStencilView),
      LinearWrapSampler(Source.LinearWrapSampler),
      ViewProjStructedBuffer(Source.ViewProjStructedBuffer),
      ViewProjStructedBufferSrv(Source.ViewProjStructedBufferSrv),
      InstanceStructedBuffer(Source.InstanceStructedBuffer),
      InstanceStructedBufferSrv(Source.InstanceStructedBufferSrv),
      DrawCallType(Source.DrawCallType), DrawCallPipe(Source.DrawCallPipe),
      RSCameraInfo(Source.RSCameraInfo), RSCamera(Source.RSCamera) {
  if (HasBeenInited) {
    RS_ADDREF(VertexShader);
    RS_ADDREF(GeometryShader);
    RS_ADDREF(PixelShader);
    RS_ADDREF(LinearWrapSampler);
    RS_ADDREF(BlendState);
    RS_ADDREF(ViewProjStructedBuffer);
    RS_ADDREF(ViewProjStructedBufferSrv);
    RS_ADDREF(InstanceStructedBuffer);
    RS_ADDREF(InstanceStructedBufferSrv);
  }
}

RSPass_Billboard::~RSPass_Billboard() {}

RSPass_Billboard *RSPass_Billboard::clonePass() {
  return new RSPass_Billboard(*this);
}

bool RSPass_Billboard::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createStates()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }

  DrawCallType = DRAWCALL_TYPE::TRANSPARENCY;
  DrawCallPipe = G_RSRoot->getDrawCallsPool()->getDrawCallsPipe(DrawCallType);

  RSCameraInfo = G_RSRoot->getCamerasContainer()->getRSCameraInfo("temp-cam");
  RSCamera = G_RSRoot->getCamerasContainer()->getRSCamera("temp-cam");

  HasBeenInited = true;

  return true;
}

void RSPass_Billboard::releasePass() {
  RS_RELEASE(VertexShader);
  RS_RELEASE(GeometryShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(BlendState);
  RS_RELEASE(LinearWrapSampler);
  RS_RELEASE(ViewProjStructedBuffer);
  RS_RELEASE(ViewProjStructedBufferSrv);
  RS_RELEASE(InstanceStructedBuffer);
  RS_RELEASE(InstanceStructedBufferSrv);
}

void RSPass_Billboard::execuatePass() {
  context()->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
  context()->RSSetViewports(1, &G_ViewPort);
  static float Factor[4] = {0.f, 0.f, 0.f, 0.f};
  context()->OMSetBlendState(BlendState, Factor, 0xFFFFFFFF);
  context()->VSSetShader(VertexShader, nullptr, 0);
  context()->GSSetShader(GeometryShader, nullptr, 0);
  context()->PSSetShader(PixelShader, nullptr, 0);
  context()->PSSetSamplers(0, 1, &LinearWrapSampler);

  dx::XMMATRIX Mat = {};
  UINT Stride = sizeof(vertex_type::TangentVertex);
  UINT Offset = 0;

  D3D11_MAPPED_SUBRESOURCE Msr = {};
  context()->Map(ViewProjStructedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
  ViewProjCamUpPos *VpData = static_cast<ViewProjCamUpPos *>(Msr.pData);
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ViewMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ViewMat, Mat);
  Mat = dx::XMLoadFloat4x4(&RSCameraInfo->ProjMatrix);
  Mat = dx::XMMatrixTranspose(Mat);
  dx::XMStoreFloat4x4(&VpData[0].ProjMat, Mat);
  VpData->CamUpVec = RSCamera->getRSCameraUpVector();
  VpData->CamPos = RSCamera->getRSCameraPosition();
  context()->Unmap(ViewProjStructedBuffer, 0);

  context()->GSSetShaderResources(0, 1, &ViewProjStructedBufferSrv);

  for (auto &Draw : DrawCallPipe->Data) {
    auto InsArrayPtr = Draw.InstanceData.DataArrayPtr;
    auto Size = InsArrayPtr->size();
    context()->Map(InstanceStructedBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    RS_INSTANCE_DATA *InsData = static_cast<RS_INSTANCE_DATA *>(Msr.pData);
    for (size_t I = 0; I < Size; I++) {
      Mat = dx::XMLoadFloat4x4(&(*InsArrayPtr)[I].WorldMatrix);
      Mat = dx::XMMatrixTranspose(Mat);
      dx::XMStoreFloat4x4(&InsData[I].WorldMatrix, Mat);
      InsData[I].MaterialData = (*InsArrayPtr)[I].MaterialData;
      InsData[I].CustomizedData1 = (*InsArrayPtr)[I].CustomizedData1;
      InsData[I].CustomizedData2 = (*InsArrayPtr)[I].CustomizedData2;
    }
    context()->Unmap(InstanceStructedBuffer, 0);

    context()->IASetInputLayout(Draw.MeshData.InputLayout);
    context()->IASetPrimitiveTopology(Draw.MeshData.TopologyType);
    context()->IASetVertexBuffers(0, 1, &Draw.MeshData.VertexBuffer, &Stride,
                                  &Offset);
    context()->IASetIndexBuffer(Draw.MeshData.IndexBuffer, DXGI_FORMAT_R32_UINT,
                                0);
    context()->VSSetShaderResources(0, 1, &InstanceStructedBufferSrv);
    context()->PSSetShaderResources(0, 1, &Draw.TextureData[0].Srv);

    context()->DrawIndexedInstanced(
        Draw.MeshData.IndexSize,
        static_cast<UINT>(Draw.InstanceData.DataArrayPtr->size()), 0, 0, 0);
  }

  static ID3D11RenderTargetView *NullRtv[] = {nullptr};
  static ID3D11ShaderResourceView *NullSrv[] = {nullptr};
  context()->OMSetRenderTargets(1, NullRtv, nullptr);
  context()->OMSetBlendState(nullptr, Factor, 0xFFFFFFFF);
  context()->GSSetShader(nullptr, nullptr, 0);
  context()->VSSetShaderResources(0, 1, NullSrv);
  context()->GSSetShaderResources(0, 1, NullSrv);
  context()->PSSetShaderResources(0, 1, NullSrv);
}

bool RSPass_Billboard::createShaders() {
  ID3DBlob *ShaderBlob = nullptr;
  HRESULT Hr = S_OK;

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/billboard_vertex.hlsl", "main", "vs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateVertexShader(ShaderBlob->GetBufferPointer(),
                                    ShaderBlob->GetBufferSize(), nullptr,
                                    &VertexShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/billboard_geometry.hlsl", "main", "gs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateGeometryShader(ShaderBlob->GetBufferPointer(),
                                      ShaderBlob->GetBufferSize(), nullptr,
                                      &GeometryShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/billboard_pixel.hlsl", "main", "ps_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreatePixelShader(ShaderBlob->GetBufferPointer(),
                                   ShaderBlob->GetBufferSize(), nullptr,
                                   &PixelShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Billboard::createStates() {
  HRESULT Hr = S_OK;

  D3D11_BLEND_DESC BldDesc = {};
  BldDesc.AlphaToCoverageEnable = FALSE;
  BldDesc.IndependentBlendEnable = FALSE;
  BldDesc.RenderTarget[0].BlendEnable = TRUE;
  BldDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  BldDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  BldDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  BldDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  BldDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  BldDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  BldDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  Hr = device()->CreateBlendState(&BldDesc, &BlendState);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Billboard::createBuffers() {
  HRESULT Hr = S_OK;
  D3D11_BUFFER_DESC BufDesc = {};

  ZeroMemory(&BufDesc, sizeof(BufDesc));
  BufDesc.Usage = D3D11_USAGE_DYNAMIC;
  BufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  BufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  BufDesc.ByteWidth = MAX_INSTANCE_SIZE * sizeof(RS_INSTANCE_DATA);
  BufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  BufDesc.StructureByteStride = sizeof(RS_INSTANCE_DATA);
  Hr = device()->CreateBuffer(&BufDesc, nullptr, &InstanceStructedBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  BufDesc.ByteWidth = sizeof(ViewProjCamUpPos);
  BufDesc.StructureByteStride = sizeof(ViewProjCamUpPos);
  Hr = device()->CreateBuffer(&BufDesc, nullptr, &ViewProjStructedBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Billboard::createViews() {
  RenderTargetView = G_RSRoot->getDevices()->getHighDynamicRtv();

  DepthStencilView =
      G_RSRoot->getResourceManager()->getResource("mrt-depth")->Dsv;
  if (!DepthStencilView) {
    return false;
  }

  HRESULT Hr = S_OK;
  D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};

  ZeroMemory(&SrvDesc, sizeof(SrvDesc));
  SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
  SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  SrvDesc.Buffer.ElementWidth = MAX_INSTANCE_SIZE;
  Hr = device()->CreateShaderResourceView(InstanceStructedBuffer, &SrvDesc,
                                          &InstanceStructedBufferSrv);
  if (FAILED(Hr)) {
    return false;
  }

  SrvDesc.Buffer.ElementWidth = 1;
  Hr = device()->CreateShaderResourceView(ViewProjStructedBuffer, &SrvDesc,
                                          &ViewProjStructedBufferSrv);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Billboard::createSamplers() {
  HRESULT Hr = S_OK;
  D3D11_SAMPLER_DESC SamDesc = {};
  ZeroMemory(&SamDesc, sizeof(SamDesc));
  auto FilterLevel = G_RenderEffectConfig.SamplerLevel;
  switch (FilterLevel) {
  case SAMPLER_LEVEL::POINT:
    SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    break;
  case SAMPLER_LEVEL::BILINEAR:
    SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    break;
  case SAMPLER_LEVEL::ANISO_8X:
    SamDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SamDesc.MaxAnisotropy = 8;
    break;
  case SAMPLER_LEVEL::ANISO_16X:
    SamDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SamDesc.MaxAnisotropy = 16;
    break;
  default:
    return false;
  }
  SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  SamDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  SamDesc.MinLOD = 0;
  SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
  Hr = device()->CreateSamplerState(&SamDesc, &LinearWrapSampler);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

RSPass_Tonemapping::RSPass_Tonemapping(std::string &Name,
                                       PASS_TYPE Type,
                                       RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), AverLuminShader(nullptr),
      DynamicExposureShader(nullptr), ToneMapShader(nullptr), HdrUav(nullptr),
      HdrSrv(nullptr), AverageLuminBufferArray({nullptr, nullptr}),
      AverageLuminSrvArray({nullptr, nullptr}),
      AverageLuminUavArray({nullptr, nullptr}) {}

RSPass_Tonemapping::RSPass_Tonemapping(const RSPass_Tonemapping &Source)
    : RSPass_Base(Source), AverLuminShader(Source.AverLuminShader),
      DynamicExposureShader(Source.DynamicExposureShader),
      ToneMapShader(Source.ToneMapShader), HdrUav(Source.HdrUav),
      HdrSrv(Source.HdrSrv),
      AverageLuminBufferArray(Source.AverageLuminBufferArray),
      AverageLuminSrvArray(Source.AverageLuminSrvArray),
      AverageLuminUavArray(Source.AverageLuminUavArray) {
  if (HasBeenInited) {
    RS_ADDREF(DynamicExposureShader);
    RS_ADDREF(AverLuminShader);
    RS_ADDREF(ToneMapShader);
    RS_ADDREF(AverageLuminBufferArray[0]);
    RS_ADDREF(AverageLuminBufferArray[1]);
    RS_ADDREF(AverageLuminSrvArray[0]);
    RS_ADDREF(AverageLuminSrvArray[1]);
    RS_ADDREF(AverageLuminUavArray[0]);
    RS_ADDREF(AverageLuminUavArray[1]);
  }
}

RSPass_Tonemapping::~RSPass_Tonemapping() {}

RSPass_Tonemapping *RSPass_Tonemapping::clonePass() {
  return new RSPass_Tonemapping(*this);
}

bool RSPass_Tonemapping::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }

  HasBeenInited = true;

  return true;
}

void RSPass_Tonemapping::releasePass() {
  RS_RELEASE(DynamicExposureShader);
  RS_RELEASE(AverLuminShader);
  RS_RELEASE(ToneMapShader);
  RS_RELEASE(AverageLuminBufferArray[0]);
  RS_RELEASE(AverageLuminBufferArray[1]);
  RS_RELEASE(AverageLuminSrvArray[0]);
  RS_RELEASE(AverageLuminSrvArray[1]);
  RS_RELEASE(AverageLuminUavArray[0]);
  RS_RELEASE(AverageLuminUavArray[1]);
}

void RSPass_Tonemapping::execuatePass() {
  static ID3D11UnorderedAccessView *NullUav = nullptr;
  static ID3D11ShaderResourceView *NullSrv = nullptr;

  UINT Width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
  UINT Height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
  UINT DispatchX = rs_tool::align(Width, 32) / 32;
  UINT DispatchY = rs_tool::align(Height, 32) / 32;

  static int CIndex = 0;
  int PIndex = CIndex;
  CIndex = (CIndex + 1) % 2;

  context()->CSSetShader(AverLuminShader, nullptr, 0);
  context()->CSSetShaderResources(0, 1, &HdrSrv);
  context()->CSSetUnorderedAccessViews(0, 1, &AverageLuminUavArray[CIndex],
                                       nullptr);

  context()->Dispatch(DispatchX, DispatchY, 1);

  context()->CSSetShaderResources(0, 1, &NullSrv);
  context()->CSSetUnorderedAccessViews(0, 1, &NullUav, nullptr);

  context()->CSSetShader(DynamicExposureShader, nullptr, 0);
  context()->CSSetShaderResources(0, 1, &AverageLuminSrvArray[PIndex]);
  context()->CSSetUnorderedAccessViews(0, 1, &AverageLuminUavArray[CIndex],
                                       nullptr);

  context()->Dispatch(1, 1, 1);

  context()->CSSetShaderResources(0, 1, &NullSrv);
  context()->CSSetUnorderedAccessViews(0, 1, &NullUav, nullptr);

  DispatchX = rs_tool::align(Width, 256) / 256;
  context()->CSSetShader(ToneMapShader, nullptr, 0);
  context()->CSSetShaderResources(0, 1, &AverageLuminSrvArray[CIndex]);
  context()->CSSetUnorderedAccessViews(0, 1, &HdrUav, nullptr);

  context()->Dispatch(DispatchX, Height, 1);

  context()->CSSetUnorderedAccessViews(0, 1, &NullUav, nullptr);
}

bool RSPass_Tonemapping::createShaders() {
  ID3DBlob *ShaderBlob = nullptr;
  HRESULT Hr = S_OK;

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/average_lumin_compute.hlsl", "main", "cs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &AverLuminShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  std::string TransSpdStr =
      hyc::str::sFormat("({:+f}f)", G_RenderEffectConfig.ExpoTransSpeed);
  std::string ExpoMinStr =
      hyc::str::sFormat("({:+f}f)", G_RenderEffectConfig.ExpoMin);
  std::string ExpoMaxStr =
      hyc::str::sFormat("({:+f}f)", G_RenderEffectConfig.ExpoMax);
  std::string InvFactorStr =
      hyc::str::sFormat("({:+f}f)", G_RenderEffectConfig.ExpoInvFactor);
  D3D_SHADER_MACRO ExpoMacro[] = {{"TRANS_SPEED", TransSpdStr.c_str()},
                                  {"EXPO_MIN", ExpoMinStr.c_str()},
                                  {"EXPO_MAX", ExpoMaxStr.c_str()},
                                  {"INV_FACTOR", InvFactorStr.c_str()},
                                  {nullptr, nullptr}};
  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/dynamic_exposure_compute.hlsl", "main", "cs_5_0",
      &ShaderBlob, ExpoMacro);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &DynamicExposureShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  std::string StaticV = "(";
  D3D_SHADER_MACRO Macro[2] = {{nullptr, nullptr}, {nullptr, nullptr}};
  if (!G_RenderEffectConfig.DynamicExpoOff) {
    Macro[0] = {"DYNAMIC_EXPOSURE", "1"};
  } else {
    StaticV += hyc::str::toString(G_RenderEffectConfig.StaticExpo);
    StaticV += "f)";
    Macro[0] = {"STATIC_EXPOSURE", StaticV.c_str()};
  }
  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/tonemap_compute.hlsl", "main", "cs_5_0",
      &ShaderBlob, Macro);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &ToneMapShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_Tonemapping::createViews() {
  HdrUav = G_RSRoot->getDevices()->getHighDynamicUav();
  HdrSrv = G_RSRoot->getDevices()->getHighDynamicSrv();

  HRESULT Hr = S_OK;
  D3D11_BUFFER_DESC BufDesc = {};
  D3D11_SUBRESOURCE_DATA InitData = {};
  D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
  D3D11_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};

  UINT Width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
  UINT Height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
  // TODO shader hasn't been supported for other sreen size
  UINT ExpoSize =
      rs_tool::align(Width, 32) / 32 * rs_tool::align(Height, 32) / 32;

  ZeroMemory(&BufDesc, sizeof(BufDesc));
  ZeroMemory(&InitData, sizeof(InitData));
  BufDesc.Usage = D3D11_USAGE_DEFAULT;
  BufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  BufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
  BufDesc.ByteWidth = ExpoSize * static_cast<UINT>(sizeof(float));
  BufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  BufDesc.StructureByteStride = sizeof(float);
  float *Data = new float[ExpoSize];
  Data[0] = G_RenderEffectConfig.StaticExpo;
  InitData.pSysMem = Data;
  Hr = device()->CreateBuffer(&BufDesc, &InitData, &AverageLuminBufferArray[0]);
  if (FAILED(Hr)) {
    delete[] Data;
    return false;
  }
  Hr = device()->CreateBuffer(&BufDesc, &InitData, &AverageLuminBufferArray[1]);
  if (FAILED(Hr)) {
    delete[] Data;
    return false;
  }
  delete[] Data;

  ZeroMemory(&SrvDesc, sizeof(SrvDesc));
  SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
  SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  SrvDesc.Buffer.ElementWidth = ExpoSize;
  Hr = device()->CreateShaderResourceView(AverageLuminBufferArray[0], &SrvDesc,
                                          &AverageLuminSrvArray[0]);
  if (FAILED(Hr)) {
    return false;
  }
  Hr = device()->CreateShaderResourceView(AverageLuminBufferArray[1], &SrvDesc,
                                          &AverageLuminSrvArray[1]);
  if (FAILED(Hr)) {
    return false;
  }

  ZeroMemory(&UavDesc, sizeof(UavDesc));
  UavDesc.Format = DXGI_FORMAT_UNKNOWN;
  UavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
  UavDesc.Buffer.NumElements = ExpoSize;
  Hr = device()->CreateUnorderedAccessView(AverageLuminBufferArray[0], &UavDesc,
                                           &AverageLuminUavArray[0]);
  if (FAILED(Hr)) {
    return false;
  }
  Hr = device()->CreateUnorderedAccessView(AverageLuminBufferArray[1], &UavDesc,
                                           &AverageLuminUavArray[1]);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

RSPass_BloomHdr::RSPass_BloomHdr(std::string &Name,
                                 PASS_TYPE Type,
                                 RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), FilterPixelShader(nullptr),
      KABlurHoriShader(nullptr), KABlurVertShader(nullptr),
      BlurHoriShader(nullptr), BlurVertShader(nullptr), UpSampleShader(nullptr),
      BlendShader(nullptr), LinearBorderSampler(nullptr),
      BlurConstBuffer(nullptr), IntensityConstBuffer(nullptr), HdrSrv(nullptr),
      HdrUav(nullptr), NeedBloomTexture(nullptr), NeedBloomSrv(nullptr),
      NeedBloomUavArray({nullptr}), UpSampleTexture(nullptr),
      UpSampleSrv(nullptr), UpSampleUavArray({nullptr}) {}

RSPass_BloomHdr::RSPass_BloomHdr(const RSPass_BloomHdr &Source)
    : RSPass_Base(Source), FilterPixelShader(Source.FilterPixelShader),
      KABlurHoriShader(Source.KABlurHoriShader),
      KABlurVertShader(Source.KABlurVertShader),
      BlurHoriShader(Source.BlurHoriShader),
      BlurVertShader(Source.BlurVertShader),
      UpSampleShader(Source.UpSampleShader), BlendShader(Source.BlendShader),
      LinearBorderSampler(Source.LinearBorderSampler),
      BlurConstBuffer(Source.BlurConstBuffer),
      IntensityConstBuffer(Source.IntensityConstBuffer), HdrSrv(Source.HdrSrv),
      HdrUav(Source.HdrUav), NeedBloomTexture(Source.NeedBloomTexture),
      NeedBloomSrv(Source.NeedBloomSrv),
      NeedBloomUavArray({Source.NeedBloomUavArray}),
      UpSampleTexture(Source.UpSampleTexture), UpSampleSrv(Source.UpSampleSrv),
      UpSampleUavArray({Source.UpSampleUavArray}) {
  if (HasBeenInited) {
    RS_ADDREF(UpSampleShader);
    RS_ADDREF(BlurHoriShader);
    RS_ADDREF(BlurVertShader);
    RS_ADDREF(KABlurHoriShader);
    RS_ADDREF(KABlurVertShader);
    RS_ADDREF(FilterPixelShader);
    RS_ADDREF(BlendShader);
    RS_ADDREF(BlurConstBuffer);
    RS_ADDREF(IntensityConstBuffer);
    RS_ADDREF(LinearBorderSampler);
    RS_ADDREF(NeedBloomTexture);
    RS_ADDREF(NeedBloomSrv);
    for (auto Uav : NeedBloomUavArray) {
      RS_ADDREF(Uav);
    }
    RS_ADDREF(UpSampleTexture);
    RS_ADDREF(UpSampleSrv);
    for (auto Uav : UpSampleUavArray) {
      RS_ADDREF(Uav);
    }
  }
}

RSPass_BloomHdr::~RSPass_BloomHdr() {}

RSPass_BloomHdr *RSPass_BloomHdr::clonePass() {
  return new RSPass_BloomHdr(*this);
}

bool RSPass_BloomHdr::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createBuffers()) {
    return false;
  }
  if (!createSampler()) {
    return false;
  }

  HasBeenInited = true;

  return true;
}

void RSPass_BloomHdr::releasePass() {
  RS_RELEASE(FilterPixelShader);
  RS_RELEASE(UpSampleShader);
  RS_RELEASE(BlurHoriShader);
  RS_RELEASE(BlurVertShader);
  RS_RELEASE(KABlurHoriShader);
  RS_RELEASE(KABlurVertShader);
  RS_RELEASE(BlendShader);
  RS_RELEASE(BlurConstBuffer);
  RS_RELEASE(IntensityConstBuffer);
  RS_RELEASE(LinearBorderSampler);
  RS_RELEASE(NeedBloomTexture);
  RS_RELEASE(NeedBloomSrv);
  for (auto Uav : NeedBloomUavArray) {
    RS_RELEASE(Uav);
  }
  RS_RELEASE(UpSampleTexture);
  RS_RELEASE(UpSampleSrv);
  for (auto Uav : UpSampleUavArray) {
    RS_RELEASE(Uav);
  }
}

void RSPass_BloomHdr::execuatePass() {
  static ID3D11UnorderedAccessView *NullUav = nullptr;
  static ID3D11ShaderResourceView *NullSrv = nullptr;
  UINT Width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
  UINT Height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
  UINT DispatchX = rs_tool::align(Width, 16) / 16;
  UINT DispatchY = rs_tool::align(Height, 16) / 16;

  context()->CSSetShader(FilterPixelShader, nullptr, 0);
  context()->CSSetShaderResources(0, 1, &HdrSrv);
  context()->CSSetUnorderedAccessViews(0, 1, NeedBloomUavArray.data(), nullptr);

  context()->Dispatch(DispatchX, DispatchY, 1);

  context()->CSSetUnorderedAccessViews(0, 1, &NullUav, nullptr);
  context()->CSSetShaderResources(0, 1, &NullSrv);

  context()->GenerateMips(NeedBloomSrv);

  D3D11_MAPPED_SUBRESOURCE Msr = {};

  static const size_t DOWN_COUNT = G_RenderEffectConfig.BloomDownSamplingCount;
  static const size_t UP_COUNT = DOWN_COUNT - 1;
  static const size_t BLUR_COUNT = G_RenderEffectConfig.BloomBlurCount;

  {
    auto Index = 1;
    UINT BlurTexWidth = Width / (1 << Index);
    UINT BlurTexHeight = Height / (1 << Index);
    context()->Map(BlurConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    BLM_BLUR_INFO *Info = static_cast<BLM_BLUR_INFO *>(Msr.pData);
    Info->TexWidth = BlurTexWidth;
    Info->TexHeight = BlurTexHeight;
    context()->Unmap(BlurConstBuffer, 0);

    context()->CSSetUnorderedAccessViews(0, 1, &NeedBloomUavArray[Index],
                                         nullptr);
    context()->CSSetConstantBuffers(0, 1, &BlurConstBuffer);

    for (size_t J = 0; J < BLUR_COUNT; J++) {
      context()->CSSetShader(KABlurHoriShader, nullptr, 0);
      DispatchX = rs_tool::align(BlurTexWidth, 256) / 256;
      DispatchY = BlurTexHeight;
      context()->Dispatch(DispatchX, DispatchY, 1);

      context()->CSSetShader(KABlurVertShader, nullptr, 0);
      DispatchX = BlurTexWidth;
      DispatchY = rs_tool::align(BlurTexHeight, 256) / 256;
      context()->Dispatch(DispatchX, DispatchY, 1);
    }
  }

  for (size_t I = 1; I < DOWN_COUNT; I++) {
    auto Index = I + 1;
    UINT BlurTexWidth = Width / (1 << Index);
    UINT BlurTexHeight = Height / (1 << Index);
    context()->Map(BlurConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    BLM_BLUR_INFO *Info = static_cast<BLM_BLUR_INFO *>(Msr.pData);
    Info->TexWidth = BlurTexWidth;
    Info->TexHeight = BlurTexHeight;
    context()->Unmap(BlurConstBuffer, 0);

    context()->CSSetUnorderedAccessViews(0, 1, &NeedBloomUavArray[Index],
                                         nullptr);
    context()->CSSetConstantBuffers(0, 1, &BlurConstBuffer);

    for (size_t J = 0; J < BLUR_COUNT; J++) {
      context()->CSSetShader(BlurHoriShader, nullptr, 0);
      DispatchX = rs_tool::align(BlurTexWidth, 256) / 256;
      DispatchY = BlurTexHeight;
      context()->Dispatch(DispatchX, DispatchY, 1);

      context()->CSSetShader(BlurVertShader, nullptr, 0);
      DispatchX = BlurTexWidth;
      DispatchY = rs_tool::align(BlurTexHeight, 256) / 256;
      context()->Dispatch(DispatchX, DispatchY, 1);
    }
  }
  context()->CSSetUnorderedAccessViews(0, 1, &NullUav, nullptr);

  Width /= 2;
  Height /= 2;
  context()->CSSetShader(UpSampleShader, nullptr, 0);
  context()->CSSetShaderResources(0, 1, &NeedBloomSrv);
  context()->CSSetSamplers(0, 1, &LinearBorderSampler);
  for (size_t I = 0; I < UP_COUNT; I++) {
    auto InvI = static_cast<int>(UP_COUNT - 1 - I);
    InvI = (InvI < 0) ? -InvI : InvI;
    UINT TexWidth = Width / (1 << InvI);
    UINT TexHeight = Height / (1 << InvI);

    context()->Map(BlurConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
    BLM_BLUR_INFO *Info = static_cast<BLM_BLUR_INFO *>(Msr.pData);
    Info->TexWidth = TexWidth;
    Info->TexHeight = TexHeight;
    Info->Pads[0] = static_cast<UINT>(InvI);
    context()->Unmap(BlurConstBuffer, 0);

    context()->CSSetUnorderedAccessViews(0, 2, &UpSampleUavArray[InvI],
                                         nullptr);

    DispatchX = rs_tool::align(TexWidth, 16) / 16;
    DispatchY = rs_tool::align(TexHeight, 16) / 16;
    context()->Dispatch(DispatchX, DispatchY, 1);
  }
  context()->CSSetUnorderedAccessViews(0, 1, &NullUav, nullptr);
  context()->CSSetUnorderedAccessViews(1, 1, &NullUav, nullptr);

  context()->Map(IntensityConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Msr);
  BLM_INTENSITY_INFO *Info = static_cast<BLM_INTENSITY_INFO *>(Msr.pData);
  Info->IntensityFactor = G_RenderEffectConfig.BloomIntensityFactor;
  context()->Unmap(IntensityConstBuffer, 0);
  context()->CSSetShader(BlendShader, nullptr, 0);
  context()->CSSetConstantBuffers(0, 1, &IntensityConstBuffer);
  context()->CSSetShaderResources(0, 1, &UpSampleSrv);
  context()->CSSetUnorderedAccessViews(0, 1, &HdrUav, nullptr);
  DispatchX = rs_tool::align(Width * 2, 16) / 16;
  DispatchY = rs_tool::align(Height * 2, 16) / 16;
  context()->Dispatch(DispatchX, DispatchY, 1);

  context()->CSSetShaderResources(0, 1, &NullSrv);
  context()->CSSetUnorderedAccessViews(0, 1, &NullUav, nullptr);
}

bool RSPass_BloomHdr::createShaders() {
  ID3DBlob *ShaderBlob = nullptr;
  HRESULT Hr = S_OK;

  const std::string PickMinValue =
      hyc::str::sFormat("{:+f}f", G_RenderEffectConfig.BloomMinValue);
  D3D_SHADER_MACRO PickMacro[] = {{"MIN_VALUE", PickMinValue.c_str()},
                                  {nullptr, nullptr}};
  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/bloom_pick_compute.hlsl", "main", "cs_5_0",
      &ShaderBlob, PickMacro);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &FilterPixelShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  std::string KernelSize = "";
  std::string KernelHalf = "";
  std::string WeightArray = "";
  {
    UINT Kernel = G_RenderEffectConfig.BloomBlurKernel;
    UINT Half = Kernel / 2;
    float Sigma = G_RenderEffectConfig.BloomBlurSigma;

    std::vector<float> WEights = {};
    rs_tool::calcGaussWeight1D(Kernel, Sigma, WEights);

    KernelSize = hyc::str::toString(Kernel);
    KernelHalf = hyc::str::toString(Half);
    WeightArray = "static const float gBlurWeight[] = {";
    for (const auto &W : WEights) {
      WeightArray += hyc::str::sFormat("{:+f}f,", W);
    }
    WeightArray += "}";
  }
  D3D_SHADER_MACRO BlurMacro[] = {{"KERNEL_SIZE", KernelSize.c_str()},
                                  {"KERNEL_HALF", KernelHalf.c_str()},
                                  {"WEIGHT_ARRAY", WeightArray.c_str()},
                                  {nullptr, nullptr}};
  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/gauss_blur_compute.hlsl", "HMain", "cs_5_0",
      &ShaderBlob, BlurMacro);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &BlurHoriShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/gauss_blur_compute.hlsl", "VMain", "cs_5_0",
      &ShaderBlob, BlurMacro);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &BlurVertShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/karis_average_compute.hlsl", "HMain", "cs_5_0",
      &ShaderBlob, BlurMacro);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &KABlurHoriShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/karis_average_compute.hlsl", "VMain", "cs_5_0",
      &ShaderBlob, BlurMacro);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &KABlurVertShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/bloom_upsample_compute.hlsl", "main", "cs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &UpSampleShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/bloom_blend_compute.hlsl", "main", "cs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &BlendShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_BloomHdr::createViews() {
  HdrUav = G_RSRoot->getDevices()->getHighDynamicUav();
  HdrSrv = G_RSRoot->getDevices()->getHighDynamicSrv();

  HRESULT Hr = S_OK;
  D3D11_TEXTURE2D_DESC TexDesc = {};
  D3D11_RENDER_TARGET_VIEW_DESC RtvDesc = {};
  D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
  D3D11_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};
  ZeroMemory(&TexDesc, sizeof(TexDesc));
  ZeroMemory(&RtvDesc, sizeof(RtvDesc));
  ZeroMemory(&SrvDesc, sizeof(SrvDesc));
  ZeroMemory(&UavDesc, sizeof(UavDesc));

  UINT DownMips = G_RenderEffectConfig.BloomDownSamplingCount + 1;
  UINT UpMips = DownMips - 2;

  TexDesc.Width = G_RSRoot->getDevices()->getCurrWndWidth();
  TexDesc.Height = G_RSRoot->getDevices()->getCurrWndHeight();
  TexDesc.MipLevels = DownMips;
  TexDesc.ArraySize = 1;
  TexDesc.SampleDesc.Count = 1;
  TexDesc.Usage = D3D11_USAGE_DEFAULT;
  TexDesc.CPUAccessFlags = 0;
  TexDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
  TexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE |
                      D3D11_BIND_UNORDERED_ACCESS;
  Hr = device()->CreateTexture2D(&TexDesc, nullptr, &NeedBloomTexture);
  if (FAILED(Hr)) {
    return false;
  }

  SrvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  SrvDesc.Texture2D.MostDetailedMip = 0;
  SrvDesc.Texture2D.MipLevels = DownMips;
  Hr = device()->CreateShaderResourceView(NeedBloomTexture, &SrvDesc,
                                          &NeedBloomSrv);
  if (FAILED(Hr)) {
    return false;
  }

  for (size_t I = 0; I < DownMips; I++) {
    UavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    UavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    UavDesc.Texture2D.MipSlice = static_cast<UINT>(I);
    Hr = device()->CreateUnorderedAccessView(NeedBloomTexture, &UavDesc,
                                             &NeedBloomUavArray[I]);
    if (FAILED(Hr)) {
      return false;
    }
  }

  TexDesc.Width /= 2;
  TexDesc.Height /= 2;
  TexDesc.MipLevels = UpMips;
  TexDesc.MiscFlags = 0;
  TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
  SrvDesc.Texture2D.MipLevels = UpMips;
  Hr = device()->CreateTexture2D(&TexDesc, nullptr, &UpSampleTexture);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateShaderResourceView(UpSampleTexture, &SrvDesc,
                                          &UpSampleSrv);
  if (FAILED(Hr)) {
    return false;
  }

  for (size_t I = 0; I < UpMips; I++) {
    UavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    UavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    UavDesc.Texture2D.MipSlice = static_cast<UINT>(I);
    Hr = device()->CreateUnorderedAccessView(UpSampleTexture, &UavDesc,
                                             &UpSampleUavArray[I]);
    if (FAILED(Hr)) {
      return false;
    }
  }

  return true;
}

bool RSPass_BloomHdr::createBuffers() {
  HRESULT Hr = S_OK;
  D3D11_BUFFER_DESC BufDesc = {};

  ZeroMemory(&BufDesc, sizeof(BufDesc));
  BufDesc.Usage = D3D11_USAGE_DYNAMIC;
  BufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  BufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  BufDesc.ByteWidth = sizeof(BLM_BLUR_INFO);
  Hr = device()->CreateBuffer(&BufDesc, nullptr, &BlurConstBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  BufDesc.ByteWidth = sizeof(BLM_INTENSITY_INFO);
  Hr = device()->CreateBuffer(&BufDesc, nullptr, &IntensityConstBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_BloomHdr::createSampler() {
  HRESULT Hr = S_OK;
  D3D11_SAMPLER_DESC SamDesc = {};
  ZeroMemory(&SamDesc, sizeof(SamDesc));

  SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  SamDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  SamDesc.MinLOD = 0;
  SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
  Hr = device()->CreateSamplerState(&SamDesc, &LinearBorderSampler);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

RSPass_ToSwapChain::RSPass_ToSwapChain(std::string &Name,
                                       PASS_TYPE Type,
                                       RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), VertexShader(nullptr),
      PixelShader(nullptr), VertexBuffer(nullptr), IndexBuffer(nullptr),
      SwapChainRtv(nullptr), HdrSrv(nullptr), LinearWrapSampler(nullptr) {}

RSPass_ToSwapChain::RSPass_ToSwapChain(const RSPass_ToSwapChain &Source)
    : RSPass_Base(Source), VertexShader(Source.VertexShader),
      PixelShader(Source.PixelShader), VertexBuffer(Source.VertexBuffer),
      IndexBuffer(Source.IndexBuffer), SwapChainRtv(Source.SwapChainRtv),
      HdrSrv(Source.HdrSrv), LinearWrapSampler(Source.LinearWrapSampler) {
  if (HasBeenInited) {
    RS_ADDREF(VertexBuffer);
    RS_ADDREF(IndexBuffer);
    RS_ADDREF(VertexShader);
    RS_ADDREF(PixelShader);
    RS_ADDREF(LinearWrapSampler);
  }
}

RSPass_ToSwapChain::~RSPass_ToSwapChain() {}

RSPass_ToSwapChain *RSPass_ToSwapChain::clonePass() {
  return new RSPass_ToSwapChain(*this);
}

bool RSPass_ToSwapChain::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createBuffers()) {
    return false;
  }
  if (!createShaders()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }

  HasBeenInited = true;

  return true;
}

void RSPass_ToSwapChain::releasePass() {
  RS_RELEASE(VertexBuffer);
  RS_RELEASE(IndexBuffer);
  RS_RELEASE(VertexShader);
  RS_RELEASE(PixelShader);
  RS_RELEASE(LinearWrapSampler);
}

void RSPass_ToSwapChain::execuatePass() {
  context()->OMSetRenderTargets(1, &SwapChainRtv, nullptr);
  context()->RSSetViewports(1, &G_ViewPort);
  context()->ClearRenderTargetView(SwapChainRtv, dx::Colors::DarkGreen);
  context()->VSSetShader(VertexShader, nullptr, 0);
  context()->PSSetShader(PixelShader, nullptr, 0);

  UINT Stride = sizeof(vertex_type::TangentVertex);
  UINT Offset = 0;

  static ID3D11ShaderResourceView *Srv[] = {HdrSrv};
  context()->PSSetShaderResources(0, 1, Srv);

  static ID3D11SamplerState *Samp[] = {
      LinearWrapSampler,
  };
  context()->PSSetSamplers(0, 1, Samp);

  context()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context()->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
  context()->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

  context()->DrawIndexedInstanced(6, 1, 0, 0, 0);

  ID3D11RenderTargetView *NullRtv = nullptr;
  context()->OMSetRenderTargets(1, &NullRtv, nullptr);
  static ID3D11ShaderResourceView *NullSrv[] = {nullptr};
  context()->PSSetShaderResources(0, 1, NullSrv);
}

bool RSPass_ToSwapChain::createBuffers() {
  HRESULT Hr = S_OK;
  D3D11_BUFFER_DESC BufDesc = {};

  vertex_type::TangentVertex V[4] = {};
  V[0].Position = dx::XMFLOAT3(-1.0f, -1.0f, 0.0f);
  V[1].Position = dx::XMFLOAT3(-1.0f, +1.0f, 0.0f);
  V[2].Position = dx::XMFLOAT3(+1.0f, +1.0f, 0.0f);
  V[3].Position = dx::XMFLOAT3(+1.0f, -1.0f, 0.0f);
  V[0].TexCoord = dx::XMFLOAT2(0.0f, 1.0f);
  V[1].TexCoord = dx::XMFLOAT2(0.0f, 0.0f);
  V[2].TexCoord = dx::XMFLOAT2(1.0f, 0.0f);
  V[3].TexCoord = dx::XMFLOAT2(1.0f, 1.0f);
  ZeroMemory(&BufDesc, sizeof(BufDesc));
  BufDesc.Usage = D3D11_USAGE_IMMUTABLE;
  BufDesc.ByteWidth = sizeof(vertex_type::TangentVertex) * 4;
  BufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  BufDesc.CPUAccessFlags = 0;
  BufDesc.MiscFlags = 0;
  BufDesc.StructureByteStride = 0;
  D3D11_SUBRESOURCE_DATA VInitData = {};
  ZeroMemory(&VInitData, sizeof(VInitData));
  VInitData.pSysMem = V;
  Hr = device()->CreateBuffer(&BufDesc, &VInitData, &VertexBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  UINT Indices[6] = {0, 1, 2, 0, 2, 3};
  ZeroMemory(&BufDesc, sizeof(BufDesc));
  BufDesc.Usage = D3D11_USAGE_IMMUTABLE;
  BufDesc.ByteWidth = sizeof(UINT) * 6;
  BufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  BufDesc.CPUAccessFlags = 0;
  BufDesc.StructureByteStride = 0;
  BufDesc.MiscFlags = 0;
  D3D11_SUBRESOURCE_DATA IInitData = {};
  ZeroMemory(&IInitData, sizeof(IInitData));
  IInitData.pSysMem = Indices;
  Hr = device()->CreateBuffer(&BufDesc, &IInitData, &IndexBuffer);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_ToSwapChain::createShaders() {
  ID3DBlob *ShaderBlob = nullptr;
  HRESULT Hr = S_OK;

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/copy_texture_vertex.hlsl", "main", "vs_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateVertexShader(ShaderBlob->GetBufferPointer(),
                                    ShaderBlob->GetBufferSize(), nullptr,
                                    &VertexShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  Hr = rs_tool::compileShaderFromFile(
      L"./Assets/Shaders/copy_texture_pixel.hlsl", "main", "ps_5_0",
      &ShaderBlob);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreatePixelShader(ShaderBlob->GetBufferPointer(),
                                   ShaderBlob->GetBufferSize(), nullptr,
                                   &PixelShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_ToSwapChain::createViews() {
  SwapChainRtv = G_RSRoot->getDevices()->getSwapChainRtv();
  HdrSrv = G_RSRoot->getDevices()->getHighDynamicSrv();

  return true;
}

bool RSPass_ToSwapChain::createSamplers() {
  HRESULT Hr = S_OK;
  D3D11_SAMPLER_DESC SamDesc = {};
  ZeroMemory(&SamDesc, sizeof(SamDesc));
  auto FilterLevel = G_RenderEffectConfig.SamplerLevel;
  switch (FilterLevel) {
  case SAMPLER_LEVEL::POINT:
    SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    break;
  case SAMPLER_LEVEL::BILINEAR:
    SamDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    break;
  case SAMPLER_LEVEL::ANISO_8X:
    SamDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SamDesc.MaxAnisotropy = 8;
    break;
  case SAMPLER_LEVEL::ANISO_16X:
    SamDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    SamDesc.MaxAnisotropy = 16;
    break;
  default:
    return false;
  }
  SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  SamDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  SamDesc.MinLOD = 0;
  SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
  Hr = device()->CreateSamplerState(&SamDesc, &LinearWrapSampler);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

RSPass_FXAA::RSPass_FXAA(std::string &Name, PASS_TYPE Type, RSRoot_DX11 *Root)
    : RSPass_Base(Name, Type, Root), FXAAShader(nullptr), HdrUav(nullptr),
      CopyTex(nullptr), CopySrv(nullptr), LinearBorderSampler(nullptr) {}

RSPass_FXAA::RSPass_FXAA(const RSPass_FXAA &Source)
    : RSPass_Base(Source), FXAAShader(Source.FXAAShader), HdrUav(Source.HdrUav),
      CopyTex(Source.CopyTex), CopySrv(Source.CopySrv),
      LinearBorderSampler(Source.LinearBorderSampler) {
  if (HasBeenInited) {
    RS_ADDREF(FXAAShader);
    RS_ADDREF(LinearBorderSampler);
    RS_ADDREF(CopyTex);
    RS_ADDREF(CopySrv);
  }
}

RSPass_FXAA::~RSPass_FXAA() {}

RSPass_FXAA *RSPass_FXAA::clonePass() { return new RSPass_FXAA(*this); }

bool RSPass_FXAA::initPass() {
  if (HasBeenInited) {
    return true;
  }

  if (!createShaders()) {
    return false;
  }
  if (!createViews()) {
    return false;
  }
  if (!createSamplers()) {
    return false;
  }

  HasBeenInited = true;

  return true;
}

void RSPass_FXAA::releasePass() {
  RS_RELEASE(FXAAShader);
  RS_RELEASE(LinearBorderSampler);
  RS_RELEASE(CopyTex);
  RS_RELEASE(CopySrv);
}

void RSPass_FXAA::execuatePass() {
  UINT DispatchX = G_RSRoot->getDevices()->getCurrWndWidth();
  UINT DispatchY = G_RSRoot->getDevices()->getCurrWndHeight();
  DispatchX = rs_tool::align(DispatchX, 16) / 16;
  DispatchY = rs_tool::align(DispatchY, 16) / 16;

  G_RSRoot->getDevices()->copyHighDynamicTexture(context(), CopyTex);

  context()->CSSetShader(FXAAShader, nullptr, 0);
  context()->CSSetSamplers(0, 1, &LinearBorderSampler);
  context()->CSSetShaderResources(0, 1, &CopySrv);
  context()->CSSetUnorderedAccessViews(0, 1, &HdrUav, nullptr);

  context()->Dispatch(DispatchX, DispatchY, 1);

  static ID3D11ShaderResourceView *NullSrv = nullptr;
  static ID3D11UnorderedAccessView *NullUav = nullptr;
  context()->CSSetShaderResources(0, 1, &NullSrv);
  context()->CSSetUnorderedAccessViews(0, 1, &NullUav, nullptr);
}

bool RSPass_FXAA::createShaders() {
  ID3DBlob *ShaderBlob = nullptr;
  HRESULT Hr = S_OK;

  std::string ThreshouldStr =
      hyc::str::sFormat("({:+f}f)", G_RenderEffectConfig.FXAAThreshould);
  std::string MinThreshouldStr =
      hyc::str::sFormat("({:+f}f)", G_RenderEffectConfig.FXAAMinThreshould);
  std::string SearchStepStr =
      hyc::str::sFormat("({})", G_RenderEffectConfig.FXAASearchStep);
  std::string BorderGuessStr =
      hyc::str::sFormat("({})", G_RenderEffectConfig.FXAAGuess);
  D3D_SHADER_MACRO Macro[] = {{"EDGE_THRESHOLD", ThreshouldStr.c_str()},
                              {"MIN_EDGE_THRESHOLD", MinThreshouldStr.c_str()},
                              {"EDGE_SEARCH_STEP", SearchStepStr.c_str()},
                              {"EDGE_GUESS", BorderGuessStr.c_str()},
                              {nullptr, nullptr}};
  Hr = rs_tool::compileShaderFromFile(L"./Assets/Shaders/fxaa_compute.hlsl",
                                      "main", "cs_5_0", &ShaderBlob, Macro);
  if (FAILED(Hr)) {
    return false;
  }

  Hr = device()->CreateComputeShader(ShaderBlob->GetBufferPointer(),
                                     ShaderBlob->GetBufferSize(), nullptr,
                                     &FXAAShader);
  ShaderBlob->Release();
  ShaderBlob = nullptr;
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_FXAA::createViews() {
  HdrUav = G_RSRoot->getDevices()->getHighDynamicUav();

  HRESULT Hr = S_OK;
  D3D11_TEXTURE2D_DESC TexDesc = {};
  D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
  ZeroMemory(&TexDesc, sizeof(TexDesc));
  ZeroMemory(&SrvDesc, sizeof(SrvDesc));

  TexDesc.Width = getRSDX11RootInstance()->getDevices()->getCurrWndWidth();
  TexDesc.Height = getRSDX11RootInstance()->getDevices()->getCurrWndHeight();
  TexDesc.MipLevels = 1;
  TexDesc.ArraySize = 1;
  TexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  TexDesc.SampleDesc.Count = 1;
  TexDesc.SampleDesc.Quality = 0;
  TexDesc.Usage = D3D11_USAGE_DEFAULT;
  TexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  TexDesc.CPUAccessFlags = 0;
  TexDesc.MiscFlags = 0;
  Hr = device()->CreateTexture2D(&TexDesc, nullptr, &CopyTex);
  if (FAILED(Hr)) {
    return false;
  }

  SrvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  SrvDesc.Texture2D.MostDetailedMip = 0;
  SrvDesc.Texture2D.MipLevels = 1;
  Hr = device()->CreateShaderResourceView(CopyTex, &SrvDesc, &CopySrv);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}

bool RSPass_FXAA::createSamplers() {
  HRESULT Hr = S_OK;
  D3D11_SAMPLER_DESC SampDesc = {};
  ZeroMemory(&SampDesc, sizeof(SampDesc));
  SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  SampDesc.MinLOD = 0;
  SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  Hr = device()->CreateSamplerState(&SampDesc, &LinearBorderSampler);
  if (FAILED(Hr)) {
    return false;
  }

  return true;
}
