#pragma once

#include "RSPass_Base.h"

#include <array>
#include <vector>

class RSShader;
class RSSampler;
class RSPipelineState;

bool createBasicPipeline();

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
#endif // __clang__

constexpr auto PTC_RENDER_BUFFER_NAME = "particle-render-buffer";
constexpr auto PTC_A_NAME = "particle-part-a";
constexpr auto PTC_B_NAME = "particle-part-b";
constexpr auto PTC_VIEW_SPCACE_POS_NAME = "particle-view-space-pos";
constexpr auto PTC_MAX_RADIUS_NAME = "particle-max-radius";
constexpr auto PTC_COARSE_CULL_NAME = "particle-coarse-cull";
constexpr auto PTC_COARSE_CULL_COUNTER_NAME = "particle-coarse-cull-counter";
constexpr auto PTC_TILED_INDEX_NAME = "particle-tiled-index";
constexpr auto PTC_DEAD_LIST_NAME = "particle-dead-list";
constexpr auto PTC_ALIVE_INDEX_NAME = "particle-alive-index";
constexpr auto PTC_DEAD_LIST_CONSTANT_NAME = "particle-dead-list-constant";
constexpr auto PTC_ALIVE_LIST_CONSTANT_NAME = "particle-alive-list-constant";
constexpr auto PTC_EMITTER_CONSTANT_NAME = "particle-emitter-constant";
constexpr auto PTC_CAMERA_CONSTANT_NAME = "particle-camera-constant";
constexpr auto PTC_TILING_CONSTANT_NAME = "particle-tiling-constant";
constexpr auto PTC_TIME_CONSTANT_NAME = "particle-time-constant";
constexpr auto PTC_DEBUG_COUNTER_NAME = "particle-debug-counter";
constexpr auto PTC_RAMDOM_TEXTURE_NAME = "particle-ramdom-texture";
constexpr auto PTC_SIMU_EMITTER_STRU_NAME = "particle-simu-emitter-structed";
constexpr UINT PTC_MAX_PARTICLE_SIZE = 400 * 1024;
constexpr UINT PTC_MAX_COARSE_CULL_TILE_X = 16;
constexpr UINT PTC_MAX_COARSE_CULL_TILE_Y = 8;
constexpr UINT PTC_MAX_COARSE_CULL_TILE_SIZE =
    PTC_MAX_COARSE_CULL_TILE_X * PTC_MAX_COARSE_CULL_TILE_Y;
constexpr UINT PTC_NUM_PER_TILE = 1023;
constexpr UINT PTC_TILE_BUFFER_SIZE = PTC_NUM_PER_TILE + 1;
constexpr UINT PTC_TILE_X_SIZE = 32;
constexpr UINT PTC_TILE_Y_SIZE = 32;
constexpr UINT PTC_COARSE_CULLING_THREADS = 256;

#if __clang__
#pragma clang diagnostic pop
#endif // __clang__

struct BLM_BLUR_INFO {
  UINT TexWidth = 0;
  UINT TexHeight = 0;
  UINT Pads[2] = {0};
};

struct BLM_INTENSITY_INFO {
  float IntensityFactor = 0.f;
  float Pads[3] = {0.f};
};

struct RS_PARTICLE_PART_A {
  dx::XMFLOAT4 ColorAndAlpha = {};
  dx::XMFLOAT2 ViewSpaceVelocityXY = {};
  float EmitterNormalDotLight = 0.f;
  UINT EmitterProperties = 0;
  float Rotation = 0.f;
  UINT IsSleeping = 0;
  UINT CollisionCount = 0;
  float Pads[1] = {0.f};
};

struct RS_PARTICLE_PART_B {
  dx::XMFLOAT3 WorldPosition = {};
  float Mass = 0.f;
  dx::XMFLOAT3 WorldSpaceVelocity = {};
  float LifeSpan = 0.f;
  float DistanceToEye = 0.f;
  float Age = 0.f;
  float StartSize = 0.f;
  float EndSize = 0.f;
  dx::XMFLOAT4 StartColor = {};
  dx::XMFLOAT4 EndColor = {};
  dx::XMFLOAT3 Acceleration = {};
  float Pads[1] = {0.f};
};

struct RS_ALIVE_INDEX_BUFFER_ELEMENT {
  float Distance;
  float Index;
};

struct CAMERA_STATUS {
  dx::XMFLOAT4X4 View = {};
  dx::XMFLOAT4X4 InvView = {};
  dx::XMFLOAT4X4 Proj = {};
  dx::XMFLOAT4X4 InvProj = {};
  dx::XMFLOAT4X4 ViewProj = {};
  dx::XMFLOAT3 EyePosition = {};
  float Pad[1] = {0.f};
};

struct SIMULATE_EMITTER_INFO {
  dx::XMFLOAT3 WorldPosition = {};
  float Pads[1] = {0.f};
};

struct RS_TILING_CONSTANT {
  UINT NumTilesX = 0;
  UINT NumTilesY = 0;
  UINT NumCoarseCullingTilesX = 0;
  UINT NumCoarseCullingTilesY = 0;
  UINT NumCullingTilesPerCoarseTileX = 0;
  UINT NumCullingTilesPerCoarseTileY = 0;
  UINT Pads[2] = {0};
};

struct PTC_TIME_CONSTANT {
  float DeltaTime = 0.016f;
  float TotalTime = 0.f;
  float Pads[2] = {0.f};
};

struct ViewProj {
  dx::XMFLOAT4X4 ViewMat = {};
  dx::XMFLOAT4X4 ProjMat = {};
};

struct ViewProjCamUpPos {
  dx::XMFLOAT4X4 ViewMat = {};
  dx::XMFLOAT4X4 ProjMat = {};
  dx::XMFLOAT3 CamUpVec = {};
  float Pad0 = 0.f;
  dx::XMFLOAT3 CamPos = {};
  float Pad1 = 0.f;
};
static_assert(sizeof(ViewProjCamUpPos) == 160,
              "keep same as VIEWPROJ_UPVEC_POS in shaders");

struct Ambient {
  dx::XMFLOAT4 AmbientV = {};
};

struct LightInfo {
  dx::XMFLOAT3 CameraPos = {};
  float Pad0 = 0.f;
  UINT DirectLightNum = 0;
  UINT SpotLightNum = 0;
  UINT PointLightNum = 0;
  UINT ShadowLightNum = 0;
  INT ShadowLightIndex[4] = {-1, -1, -1, -1};
};

struct ShadowInfo {
  dx::XMFLOAT4X4 ShadowViewMat = {};
  dx::XMFLOAT4X4 ShadowProjMat = {};
  dx::XMFLOAT4X4 SSAOMat = {};
};

struct SsaoInfo {
  dx::XMFLOAT4X4 Proj;
  dx::XMFLOAT4X4 View;
  dx::XMFLOAT4X4 InvProj;
  dx::XMFLOAT4X4 TexProj;
  dx::XMFLOAT4 OffsetVec[14];
  float OcclusionRadius;
  float OcclusionFadeStart;
  float OcclusionFadeEnd;
  float SurfaceEpsilon;
};

struct SkyShpereInfo {
  dx::XMFLOAT4X4 WorldMat = {};
  dx::XMFLOAT4X4 ViewMat = {};
  dx::XMFLOAT4X4 ProjMat = {};
  dx::XMFLOAT3 EyePosition = {};
  float Pad = 0.f;
};

struct OnlyProj {
  dx::XMFLOAT4X4 ProjMat = {};
};

class RSPass_MRT : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *AniVertexShader;
  RSShader *PixelShader;
  RSShader *NDPixelShader;
  RSPipelineState *StaticPipeline;
  RSPipelineState *AniPipeline;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  RSBuffer *ViewProjStructedBuffer;
  RSShaderResourceView *ViewProjStructedBufferSrv;
  RSBuffer *InstanceStructedBuffer;
  RSShaderResourceView *InstanceStructedBufferSrv;
  RSBuffer *BonesStructedBuffer;
  RSShaderResourceView *BonesStructedBufferSrv;
  RSSampler *LinearSampler;
  RSRenderTargetView *GeoBufferRtv;
  RSRenderTargetView *AnisotropicRtv;
  RSDepthStencilView *DepthDsv;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_MRT(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_MRT(const RSPass_MRT &Source);
  virtual ~RSPass_MRT();

public:
  virtual RSPass_MRT *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createBuffers();
  bool createViews();
  bool createSamplers();
  bool createPipelines();
};

class RSPass_Ssao : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *PixelShader;
  RSShader *CompressVertexShader;
  RSShader *CompressPixelShader;
  RSPipelineState *SsaoPipeline;
  RSPipelineState *CompressPipeline;
  RSRenderTargetView *RenderTargetView;
  RSShaderResourceView *NotCompressSrv;
  RSRenderTargetView *CompressRtv;
  RSSampler *SamplePointClamp;
  RSSampler *SampleLinearClamp;
  RSSampler *SampleDepthMap;
  RSSampler *SampleLinearWrap;
  RSBuffer *SsaoInfoStructedBuffer;
  RSShaderResourceView *SsaoInfoStructedBufferSrv;
  RSShaderResourceView *GeoBufferSrv;
  RSShaderResourceView *DepthMapSrv;
  RSShaderResourceView *RandomMapSrv;
  std::array<dx::XMFLOAT4, 14> OffsetVec;
  RSBuffer *VertexBuffer;
  RSBuffer *IndexBuffer;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_Ssao(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Ssao(const RSPass_Ssao &Source);
  virtual ~RSPass_Ssao();

public:
  virtual RSPass_Ssao *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createBuffers();
  bool createTextures();
  bool createViews();
  bool createSamplers();
  bool createPipelines();
};

class RSPass_KBBlur : public RSPass_Base {
private:
  // compute shaders are held as their pipeline states
  RSPipelineState *HoriBlurShader;
  RSPipelineState *VertBlurShader;
  RSUnorderedAccessView *SsaoTexUav;
  RSShaderResourceView *GeoBufferSrv;
  RSShaderResourceView *DepthMapSrv;

public:
  RSPass_KBBlur(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_KBBlur(const RSPass_KBBlur &Source);
  virtual ~RSPass_KBBlur();

public:
  virtual RSPass_KBBlur *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createViews();
};

class RSPass_Shadow : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *AniVertexShader;
  RSPipelineState *StaticPipeline;
  RSPipelineState *AniPipeline;
  std::array<RSDepthStencilView *, MAX_SHADOW_SIZE> DepthStencilView;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  RSBuffer *ViewProjStructedBuffer;
  RSShaderResourceView *ViewProjStructedBufferSrv;
  RSBuffer *InstanceStructedBuffer;
  RSShaderResourceView *InstanceStructedBufferSrv;
  RSBuffer *BonesStructedBuffer;
  RSShaderResourceView *BonesStructedBufferSrv;

public:
  RSPass_Shadow(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Shadow(const RSPass_Shadow &Source);
  virtual ~RSPass_Shadow();

public:
  virtual RSPass_Shadow *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createStates();
  bool createBuffers();
  bool createViews();
  bool createSamplers();
};

class RSPass_Defered : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *PixelShader;
  RSPipelineState *Pipeline;
  RSRenderTargetView *RenderTargetView;
  RSSampler *LinearWrapSampler;
  RSSampler *PointClampSampler;
  RSSampler *ShadowTexSampler;
  RSBuffer *LightInfoStructedBuffer;
  RSShaderResourceView *LightInfoStructedBufferSrv;
  RSBuffer *LightStructedBuffer;
  RSShaderResourceView *LightStructedBufferSrv;
  RSBuffer *AmbientStructedBuffer;
  RSShaderResourceView *AmbientStructedBufferSrv;
  RSBuffer *ShadowStructedBuffer;
  RSShaderResourceView *ShadowStructedBufferSrv;
  RSBuffer *CameraStructedBuffer;
  RSShaderResourceView *CameraStructedBufferSrv;
  RSShaderResourceView *GeoBufferSrv;
  RSShaderResourceView *AnisotropicSrv;
  RSShaderResourceView *SsaoSrv;
  RSShaderResourceView *ShadowDepthSrv;
  RSBuffer *VertexBuffer;
  RSBuffer *IndexBuffer;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_Defered(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Defered(const RSPass_Defered &Source);
  virtual ~RSPass_Defered();

public:
  virtual RSPass_Defered *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createBuffers();
  bool createViews();
  bool createSamplers();
  bool createPipelines();
};

class RSPass_SkyShpere : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *PixelShader;
  RSPipelineState *Pipeline;
  RSSampler *LinearWrapSampler;
  RSRenderTargetView *RenderTargerView;
  RSDepthStencilView *DepthStencilView;
  RSBuffer *SkyShpereInfoStructedBuffer;
  RSShaderResourceView *SkyShpereInfoStructedBufferSrv;
  RS_SUBMESH_DATA SkySphereMesh;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_SkyShpere(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_SkyShpere(const RSPass_SkyShpere &Source);
  virtual ~RSPass_SkyShpere();

public:
  virtual RSPass_SkyShpere *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createStates();
  bool createBuffers();
  bool createViews();
  bool createSamplers();
};

class RSPass_Bloom : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *PixelShader;
  RSPipelineState *Pipeline;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  RSBuffer *ViewProjStructedBuffer;
  RSShaderResourceView *ViewProjStructedBufferSrv;
  RSBuffer *InstanceStructedBuffer;
  RSShaderResourceView *InstanceStructedBufferSrv;
  RSRenderTargetView *Rtv;
  RSDepthStencilView *DepthDsv;
  RS_CAM_INFO *RSCameraInfo;
  RSBuffer *VertexBuffer;
  RSBuffer *IndexBuffer;
  RSSampler *Sampler;

public:
  RSPass_Bloom(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Bloom(const RSPass_Bloom &Source);
  virtual ~RSPass_Bloom();

public:
  virtual RSPass_Bloom *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createBuffers();
  bool createViews();
  bool createSamplers();
  bool createPipelines();
};

class RSPass_PriticleSetUp : public RSPass_Base {
private:
  RS_TILING_CONSTANT TilingConstant;

  RSBuffer *ParticleRenderBuffer;
  RSShaderResourceView *ParticleRender_Srv;
  RSUnorderedAccessView *ParticleRender_Uav;

  RSBuffer *ParticlePartA;
  RSShaderResourceView *PartA_Srv;
  RSUnorderedAccessView *PartA_Uav;

  RSBuffer *ParticlePartB;
  RSUnorderedAccessView *PartB_Uav;

  RSBuffer *ViewspacePosBuffer;
  RSShaderResourceView *ViewSpacePos_Srv;
  RSUnorderedAccessView *ViewSpacePos_Uav;

  RSBuffer *MaxRadiusBuffer;
  RSShaderResourceView *MaxRadius_Srv;
  RSUnorderedAccessView *MaxRadius_Uav;

  RSBuffer *StridedCoarseCullBuffer;
  RSShaderResourceView *StridedCoarseCull_Srv;
  RSUnorderedAccessView *StridedCoarseCull_Uav;

  RSBuffer *StridedCoarseCullCounterBuffer;
  RSShaderResourceView *StridedCoarseCullCounter_Srv;
  RSUnorderedAccessView *StridedCoarseCullCounter_Uav;

  RSBuffer *TiledIndexBuffer;
  RSShaderResourceView *TiledIndex_Srv;
  RSUnorderedAccessView *TiledIndex_Uav;

  RSBuffer *DeadListBuffer;
  RSUnorderedAccessView *DeadList_Uav;

  RSBuffer *AliveIndexBuffer;
  RSShaderResourceView *AliveIndex_Srv;
  RSUnorderedAccessView *AliveIndex_Uav;

  RSBuffer *DeadListConstantBuffer;
  RSBuffer *ActiveListConstantBuffer;

  RSBuffer *EmitterConstantBuffer;
  RSBuffer *CameraConstantBuffer;
  RSBuffer *TilingConstantBuffer;
  RSBuffer *TimeConstantBuffer;

  RSBuffer *DebugCounterBuffer;

  RSTexture *ParticleRandomTexture;
  RSShaderResourceView *ParticleRandom_Srv;

  RSBuffer *SimulEmitterStructedBuffer;
  RSShaderResourceView *SimulEmitterStructedBuffer_Srv;

public:
  RSPass_PriticleSetUp(std::string &Name,
                       PASS_TYPE Type,
                       class RSRoot_DX11 *Root);
  RSPass_PriticleSetUp(const RSPass_PriticleSetUp &Source);
  virtual ~RSPass_PriticleSetUp();

  const RS_TILING_CONSTANT &getTilingConstantInfo() const;

public:
  virtual RSPass_PriticleSetUp *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createBuffers();
  bool createViews();
};

class RSPass_PriticleEmitSimulate : public RSPass_Base {
private:
  class RSParticlesContainer *RSParticleContainerPtr;

  // compute shaders are held as their pipeline states
  RSPipelineState *InitDeadListShader;
  RSPipelineState *ResetParticlesShader;
  RSPipelineState *EmitParticleShader;
  RSPipelineState *SimulateShader;

  RSShaderResourceView *DepthTex_Srv;
  RSShaderResourceView *RandomTex_Srv;
  RSShaderResourceView *SimulEmitterStructedBuffer_Srv;
  RSUnorderedAccessView *DeadList_Uav;
  RSUnorderedAccessView *PartA_Uav;
  RSUnorderedAccessView *PartB_Uav;
  RSUnorderedAccessView *AliveIndex_Uav;
  RSUnorderedAccessView *ViewSpacePos_Uav;
  RSUnorderedAccessView *MaxRadius_Uav;
  RSBuffer *EmitterConstantBuffer;
  RSBuffer *CameraConstantBuffer;
  RSBuffer *DeadListConstantBuffer;
  RSBuffer *SimulEmitterStructedBuffer;
  RSBuffer *TimeConstantBuffer;

  RSSampler *LinearWrapSampler;

  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_PriticleEmitSimulate(std::string &Name,
                              PASS_TYPE Type,
                              class RSRoot_DX11 *Root);
  RSPass_PriticleEmitSimulate(const RSPass_PriticleEmitSimulate &Source);
  virtual ~RSPass_PriticleEmitSimulate();

public:
  virtual RSPass_PriticleEmitSimulate *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createSampler();
  bool checkResources();
};

class RSPass_PriticleTileRender : public RSPass_Base {
private:
  // compute shaders are held as their pipeline states
  RSPipelineState *CoarseCullingShader;
  RSPipelineState *TileCullingShader;
  RSPipelineState *TileRenderShader;
  RSShader *BlendVertexShader;
  RSShader *BlendPixelShader;
  RSPipelineState *BlendPipeline;

  RSBuffer *CameraConstantBuffer;
  RSBuffer *TilingConstantBuffer;
  RSBuffer *ActiveListConstantBuffer;
  RSShaderResourceView *DepthTex_Srv;
  RSShaderResourceView *ViewSpacePos_Srv;
  RSShaderResourceView *MaxRadius_Srv;
  RSShaderResourceView *PartA_Srv;
  RSShaderResourceView *AliveIndex_Srv;
  RSUnorderedAccessView *AliveIndex_Uav;
  RSShaderResourceView *CoarseTileIndex_Srv;
  RSUnorderedAccessView *CoarseTileIndex_Uav;
  RSShaderResourceView *CoarseTileIndexCounter_Srv;
  RSUnorderedAccessView *CoarseTileIndexCounter_Uav;
  RSShaderResourceView *TiledIndex_Srv;
  RSUnorderedAccessView *TiledIndex_Uav;
  RSShaderResourceView *ParticleRender_Srv;
  RSUnorderedAccessView *ParticleRender_Uav;

  RSSampler *LinearClampSampler;

  RSShaderResourceView *ParticleTex_Srv;

  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_PriticleTileRender(std::string &Name,
                            PASS_TYPE Type,
                            class RSRoot_DX11 *Root);
  RSPass_PriticleTileRender(const RSPass_PriticleTileRender &Source);
  virtual ~RSPass_PriticleTileRender();

public:
  virtual RSPass_PriticleTileRender *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createViews();
  bool createSampler();
  bool createBlend();
  bool checkResources();
};

class RSPass_Sprite : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *PixelShader;
  RSPipelineState *Pipeline;
  RSRenderTargetView *RenderTargetView;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  RSBuffer *ProjStructedBuffer;
  RSShaderResourceView *ProjStructedBufferSrv;
  RSBuffer *InstanceStructedBuffer;
  RSShaderResourceView *InstanceStructedBufferSrv;
  RSSampler *LinearSampler;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_Sprite(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Sprite(const RSPass_Sprite &Source);
  virtual ~RSPass_Sprite();

public:
  virtual RSPass_Sprite *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createStates();
  bool createBuffers();
  bool createViews();
  bool createSamplers();
};

class RSPass_SimpleLight : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *PixelShader;
  RSPipelineState *Pipeline;
  RSRenderTargetView *RenderTargetView;
  RSSampler *LinearWrapSampler;
  RSShaderResourceView *GeoBufferSrv;
  RSShaderResourceView *SsaoSrv;
  RSBuffer *VertexBuffer;
  RSBuffer *IndexBuffer;

public:
  RSPass_SimpleLight(std::string &Name,
                     PASS_TYPE Type,
                     class RSRoot_DX11 *Root);
  RSPass_SimpleLight(const RSPass_SimpleLight &Source);
  virtual ~RSPass_SimpleLight();

public:
  virtual RSPass_SimpleLight *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createBuffers();
  bool createViews();
  bool createSamplers();
  bool createPipelines();
};

class RSPass_Billboard : public RSPass_Base {
private:
  RSShader *VertexShader;
  RSShader *GeometryShader;
  RSShader *PixelShader;
  RSPipelineState *Pipeline;
  RSRenderTargetView *RenderTargetView;
  RSDepthStencilView *DepthStencilView;
  RSSampler *LinearWrapSampler;
  RSBuffer *ViewProjStructedBuffer;
  RSShaderResourceView *ViewProjStructedBufferSrv;
  RSBuffer *InstanceStructedBuffer;
  RSShaderResourceView *InstanceStructedBufferSrv;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  RS_CAM_INFO *RSCameraInfo;
  class RSCamera *RSCamera;

public:
  RSPass_Billboard(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Billboard(const RSPass_Billboard &Source);
  virtual ~RSPass_Billboard();

public:
  virtual RSPass_Billboard *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createStates();
  bool createShaders();
  bool createBuffers();
  bool createViews();
  bool createSamplers();
};

class RSPass_Tonemapping : public RSPass_Base {
private:
  // compute shaders are held as their pipeline states
  RSPipelineState *AverLuminShader;
  RSPipelineState *DynamicExposureShader;
  RSPipelineState *ToneMapShader;
  RSUnorderedAccessView *HdrUav;
  RSShaderResourceView *HdrSrv;
  std::array<RSBuffer *, 2> AverageLuminBufferArray;
  std::array<RSShaderResourceView *, 2> AverageLuminSrvArray;
  std::array<RSUnorderedAccessView *, 2> AverageLuminUavArray;

public:
  RSPass_Tonemapping(std::string &Name,
                     PASS_TYPE Type,
                     class RSRoot_DX11 *Root);
  RSPass_Tonemapping(const RSPass_Tonemapping &Source);
  virtual ~RSPass_Tonemapping();

public:
  virtual RSPass_Tonemapping *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createViews();
};

class RSPass_BloomHdr : public RSPass_Base {
private:
  // compute shaders are held as their pipeline states
  RSPipelineState *FilterPixelShader;
  RSPipelineState *KABlurHoriShader;
  RSPipelineState *KABlurVertShader;
  RSPipelineState *BlurHoriShader;
  RSPipelineState *BlurVertShader;
  RSPipelineState *UpSampleShader;
  RSPipelineState *BlendShader;
  RSSampler *LinearBorderSampler;
  RSBuffer *BlurConstBuffer;
  RSBuffer *IntensityConstBuffer;
  RSShaderResourceView *HdrSrv;
  RSUnorderedAccessView *HdrUav;
  RSTexture *NeedBloomTexture;
  RSShaderResourceView *NeedBloomSrv;
  std::array<RSUnorderedAccessView *, 10> NeedBloomUavArray;
  RSTexture *UpSampleTexture;
  RSShaderResourceView *UpSampleSrv;
  std::array<RSUnorderedAccessView *, 8> UpSampleUavArray;

public:
  RSPass_BloomHdr(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_BloomHdr(const RSPass_BloomHdr &Source);
  virtual ~RSPass_BloomHdr();

public:
  virtual RSPass_BloomHdr *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createViews();
  bool createBuffers();
  bool createSampler();
};

class RSPass_FXAA : public RSPass_Base {
private:
  ID3D11ComputeShader *FXAAShader;
  ID3D11UnorderedAccessView *HdrUav;
  ID3D11Texture2D *CopyTex;
  ID3D11ShaderResourceView *CopySrv;
  ID3D11SamplerState *LinearBorderSampler;

public:
  RSPass_FXAA(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_FXAA(const RSPass_FXAA &Source);
  virtual ~RSPass_FXAA();

public:
  virtual RSPass_FXAA *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createShaders();
  bool createViews();
  bool createSamplers();
};

class RSPass_ToSwapChain : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11PixelShader *PixelShader;
  ID3D11Buffer *VertexBuffer;
  ID3D11Buffer *IndexBuffer;
  ID3D11RenderTargetView *SwapChainRtv;
  ID3D11ShaderResourceView *HdrSrv;
  ID3D11SamplerState *LinearWrapSampler;

public:
  RSPass_ToSwapChain(std::string &Name,
                     PASS_TYPE Type,
                     class RSRoot_DX11 *Root);
  RSPass_ToSwapChain(const RSPass_ToSwapChain &Source);
  virtual ~RSPass_ToSwapChain();

public:
  virtual RSPass_ToSwapChain *clonePass() override;

  virtual bool initPass() override;

  virtual void releasePass() override;

  virtual void execuatePass() override;

private:
  bool createBuffers();
  bool createShaders();
  bool createViews();
  bool createSamplers();
};

void setPipelineDeltaTime(float DeltaMilliSecond);

void setPipelineIBLTextures(RSShaderResourceView *EnvSrv,
                            RSShaderResourceView *DiffSrv,
                            RSShaderResourceView *SpecSrv);
