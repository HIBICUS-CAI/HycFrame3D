#pragma once

#include "RSPass_Base.h"

#include <array>
#include <vector>

bool
createBasicPipeline();

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"

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

#pragma clang diagnostic pop

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
  dx::XMFLOAT3 CamPos = {};
};

struct Ambient {
  dx::XMFLOAT4 Ambient = {};
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
  ID3D11VertexShader *VertexShader;
  ID3D11VertexShader *AniVertexShader;
  ID3D11PixelShader *PixelShader;
  ID3D11PixelShader *NDPixelShader;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  ID3D11Buffer *ViewProjStructedBuffer;
  ID3D11ShaderResourceView *ViewProjStructedBufferSrv;
  ID3D11Buffer *InstanceStructedBuffer;
  ID3D11ShaderResourceView *InstanceStructedBufferSrv;
  ID3D11Buffer *BonesStructedBuffer;
  ID3D11ShaderResourceView *BonesStructedBufferSrv;
  ID3D11SamplerState *LinearSampler;
  ID3D11RenderTargetView *GeoBufferRtv;
  ID3D11RenderTargetView *AnisotropicRtv;
  ID3D11DepthStencilView *DepthDsv;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_MRT(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_MRT(const RSPass_MRT &Source);
  virtual ~RSPass_MRT();

public:
  virtual RSPass_MRT *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createBuffers();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_Ssao : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11PixelShader *PixelShader;
  ID3D11VertexShader *CompressVertexShader;
  ID3D11PixelShader *CompressPixelShader;
  ID3D11RenderTargetView *RenderTargetView;
  ID3D11ShaderResourceView *NotCompressSrv;
  ID3D11RenderTargetView *CompressRtv;
  ID3D11SamplerState *SamplePointClamp;
  ID3D11SamplerState *SampleLinearClamp;
  ID3D11SamplerState *SampleDepthMap;
  ID3D11SamplerState *SampleLinearWrap;
  ID3D11Buffer *SsaoInfoStructedBuffer;
  ID3D11ShaderResourceView *SsaoInfoStructedBufferSrv;
  ID3D11ShaderResourceView *GeoBufferSrv;
  ID3D11ShaderResourceView *DepthMapSrv;
  ID3D11ShaderResourceView *RandomMapSrv;
  std::array<dx::XMFLOAT4, 14> OffsetVec;
  ID3D11Buffer *VertexBuffer;
  ID3D11Buffer *IndexBuffer;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_Ssao(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Ssao(const RSPass_Ssao &Source);
  virtual ~RSPass_Ssao();

public:
  virtual RSPass_Ssao *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createBuffers();
  bool
  createTextures();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_KBBlur : public RSPass_Base {
private:
  ID3D11ComputeShader *HoriBlurShader;
  ID3D11ComputeShader *VertBlurShader;
  ID3D11UnorderedAccessView *SsaoTexUav;
  ID3D11ShaderResourceView *GeoBufferSrv;
  ID3D11ShaderResourceView *DepthMapSrv;

public:
  RSPass_KBBlur(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_KBBlur(const RSPass_KBBlur &Source);
  virtual ~RSPass_KBBlur();

public:
  virtual RSPass_KBBlur *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createViews();
};

class RSPass_Shadow : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11VertexShader *AniVertexShader;
  ID3D11RasterizerState *RasterizerState;
  std::array<ID3D11DepthStencilView *, MAX_SHADOW_SIZE> DepthStencilView;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  ID3D11Buffer *ViewProjStructedBuffer;
  ID3D11ShaderResourceView *ViewProjStructedBufferSrv;
  ID3D11Buffer *InstanceStructedBuffer;
  ID3D11ShaderResourceView *InstanceStructedBufferSrv;
  ID3D11Buffer *BonesStructedBuffer;
  ID3D11ShaderResourceView *BonesStructedBufferSrv;

public:
  RSPass_Shadow(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Shadow(const RSPass_Shadow &Source);
  virtual ~RSPass_Shadow();

public:
  virtual RSPass_Shadow *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createStates();
  bool
  createBuffers();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_Defered : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11PixelShader *PixelShader;
  ID3D11RenderTargetView *RenderTargetView;
  ID3D11SamplerState *LinearWrapSampler;
  ID3D11SamplerState *PointClampSampler;
  ID3D11SamplerState *ShadowTexSampler;
  ID3D11Buffer *LightInfoStructedBuffer;
  ID3D11ShaderResourceView *LightInfoStructedBufferSrv;
  ID3D11Buffer *LightStructedBuffer;
  ID3D11ShaderResourceView *LightStructedBufferSrv;
  ID3D11Buffer *AmbientStructedBuffer;
  ID3D11ShaderResourceView *AmbientStructedBufferSrv;
  ID3D11Buffer *ShadowStructedBuffer;
  ID3D11ShaderResourceView *ShadowStructedBufferSrv;
  ID3D11Buffer *CameraStructedBuffer;
  ID3D11ShaderResourceView *CameraStructedBufferSrv;
  ID3D11ShaderResourceView *GeoBufferSrv;
  ID3D11ShaderResourceView *AnisotropicSrv;
  ID3D11ShaderResourceView *SsaoSrv;
  ID3D11ShaderResourceView *ShadowDepthSrv;
  ID3D11Buffer *VertexBuffer;
  ID3D11Buffer *IndexBuffer;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_Defered(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Defered(const RSPass_Defered &Source);
  virtual ~RSPass_Defered();

public:
  virtual RSPass_Defered *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createBuffers();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_SkyShpere : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11PixelShader *PixelShader;
  ID3D11RasterizerState *RasterizerState;
  ID3D11DepthStencilState *DepthStencilState;
  ID3D11SamplerState *LinearWrapSampler;
  ID3D11RenderTargetView *RenderTargerView;
  ID3D11DepthStencilView *DepthStencilView;
  ID3D11Buffer *SkyShpereInfoStructedBuffer;
  ID3D11ShaderResourceView *SkyShpereInfoStructedBufferSrv;
  RS_SUBMESH_DATA SkySphereMesh;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_SkyShpere(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_SkyShpere(const RSPass_SkyShpere &Source);
  virtual ~RSPass_SkyShpere();

public:
  virtual RSPass_SkyShpere *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createStates();
  bool
  createBuffers();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_Bloom : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11PixelShader *PixelShader;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  ID3D11Buffer *ViewProjStructedBuffer;
  ID3D11ShaderResourceView *ViewProjStructedBufferSrv;
  ID3D11Buffer *InstanceStructedBuffer;
  ID3D11ShaderResourceView *InstanceStructedBufferSrv;
  ID3D11RenderTargetView *Rtv;
  ID3D11DepthStencilView *DepthDsv;
  RS_CAM_INFO *RSCameraInfo;
  ID3D11Buffer *VertexBuffer;
  ID3D11Buffer *IndexBuffer;
  ID3D11SamplerState *Sampler;

public:
  RSPass_Bloom(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Bloom(const RSPass_Bloom &Source);
  virtual ~RSPass_Bloom();

public:
  virtual RSPass_Bloom *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createBuffers();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_PriticleSetUp : public RSPass_Base {
private:
  RS_TILING_CONSTANT TilingConstant;

  ID3D11Buffer *ParticleRenderBuffer;
  ID3D11ShaderResourceView *ParticleRender_Srv;
  ID3D11UnorderedAccessView *ParticleRender_Uav;

  ID3D11Buffer *ParticlePartA;
  ID3D11ShaderResourceView *PartA_Srv;
  ID3D11UnorderedAccessView *PartA_Uav;

  ID3D11Buffer *ParticlePartB;
  ID3D11UnorderedAccessView *PartB_Uav;

  ID3D11Buffer *ViewspacePosBuffer;
  ID3D11ShaderResourceView *ViewSpacePos_Srv;
  ID3D11UnorderedAccessView *ViewSpacePos_Uav;

  ID3D11Buffer *MaxRadiusBuffer;
  ID3D11ShaderResourceView *MaxRadius_Srv;
  ID3D11UnorderedAccessView *MaxRadius_Uav;

  ID3D11Buffer *StridedCoarseCullBuffer;
  ID3D11ShaderResourceView *StridedCoarseCull_Srv;
  ID3D11UnorderedAccessView *StridedCoarseCull_Uav;

  ID3D11Buffer *StridedCoarseCullCounterBuffer;
  ID3D11ShaderResourceView *StridedCoarseCullCounter_Srv;
  ID3D11UnorderedAccessView *StridedCoarseCullCounter_Uav;

  ID3D11Buffer *TiledIndexBuffer;
  ID3D11ShaderResourceView *TiledIndex_Srv;
  ID3D11UnorderedAccessView *TiledIndex_Uav;

  ID3D11Buffer *DeadListBuffer;
  ID3D11UnorderedAccessView *DeadList_Uav;

  ID3D11Buffer *AliveIndexBuffer;
  ID3D11ShaderResourceView *AliveIndex_Srv;
  ID3D11UnorderedAccessView *AliveIndex_Uav;

  ID3D11Buffer *DeadListConstantBuffer;
  ID3D11Buffer *ActiveListConstantBuffer;

  ID3D11Buffer *EmitterConstantBuffer;
  ID3D11Buffer *CameraConstantBuffer;
  ID3D11Buffer *TilingConstantBuffer;
  ID3D11Buffer *TimeConstantBuffer;

  ID3D11Buffer *DebugCounterBuffer;

  ID3D11Texture2D *ParticleRandomTexture;
  ID3D11ShaderResourceView *ParticleRandom_Srv;

  ID3D11Buffer *SimulEmitterStructedBuffer;
  ID3D11ShaderResourceView *SimulEmitterStructedBuffer_Srv;

public:
  RSPass_PriticleSetUp(std::string &Name,
                       PASS_TYPE Type,
                       class RSRoot_DX11 *Root);
  RSPass_PriticleSetUp(const RSPass_PriticleSetUp &Source);
  virtual ~RSPass_PriticleSetUp();

  const RS_TILING_CONSTANT &
  getTilingConstantInfo() const;

public:
  virtual RSPass_PriticleSetUp *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createBuffers();
  bool
  createViews();
};

class RSPass_PriticleEmitSimulate : public RSPass_Base {
private:
  class RSParticlesContainer *RSParticleContainerPtr;

  ID3D11ComputeShader *InitDeadListShader;
  ID3D11ComputeShader *ResetParticlesShader;
  ID3D11ComputeShader *EmitParticleShader;
  ID3D11ComputeShader *SimulateShader;

  ID3D11ShaderResourceView *DepthTex_Srv;
  ID3D11ShaderResourceView *RandomTex_Srv;
  ID3D11ShaderResourceView *SimulEmitterStructedBuffer_Srv;
  ID3D11UnorderedAccessView *DeadList_Uav;
  ID3D11UnorderedAccessView *PartA_Uav;
  ID3D11UnorderedAccessView *PartB_Uav;
  ID3D11UnorderedAccessView *AliveIndex_Uav;
  ID3D11UnorderedAccessView *ViewSpacePos_Uav;
  ID3D11UnorderedAccessView *MaxRadius_Uav;
  ID3D11Buffer *EmitterConstantBuffer;
  ID3D11Buffer *CameraConstantBuffer;
  ID3D11Buffer *DeadListConstantBuffer;
  ID3D11Buffer *SimulEmitterStructedBuffer;
  ID3D11Buffer *TimeConstantBuffer;

  ID3D11SamplerState *LinearWrapSampler;

  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_PriticleEmitSimulate(std::string &Name,
                              PASS_TYPE Type,
                              class RSRoot_DX11 *Root);
  RSPass_PriticleEmitSimulate(const RSPass_PriticleEmitSimulate &Source);
  virtual ~RSPass_PriticleEmitSimulate();

public:
  virtual RSPass_PriticleEmitSimulate *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createSampler();
  bool
  checkResources();
};

class RSPass_PriticleTileRender : public RSPass_Base {
private:
  ID3D11ComputeShader *CoarseCullingShader;
  ID3D11ComputeShader *TileCullingShader;
  ID3D11ComputeShader *TileRenderShader;
  ID3D11VertexShader *BlendVertexShader;
  ID3D11PixelShader *BlendPixelShader;

  ID3D11Buffer *CameraConstantBuffer;
  ID3D11Buffer *TilingConstantBuffer;
  ID3D11Buffer *ActiveListConstantBuffer;
  ID3D11ShaderResourceView *DepthTex_Srv;
  ID3D11ShaderResourceView *ViewSpacePos_Srv;
  ID3D11ShaderResourceView *MaxRadius_Srv;
  ID3D11ShaderResourceView *PartA_Srv;
  ID3D11ShaderResourceView *AliveIndex_Srv;
  ID3D11UnorderedAccessView *AliveIndex_Uav;
  ID3D11ShaderResourceView *CoarseTileIndex_Srv;
  ID3D11UnorderedAccessView *CoarseTileIndex_Uav;
  ID3D11ShaderResourceView *CoarseTileIndexCounter_Srv;
  ID3D11UnorderedAccessView *CoarseTileIndexCounter_Uav;
  ID3D11ShaderResourceView *TiledIndex_Srv;
  ID3D11UnorderedAccessView *TiledIndex_Uav;
  ID3D11ShaderResourceView *ParticleRender_Srv;
  ID3D11UnorderedAccessView *ParticleRender_Uav;

  ID3D11SamplerState *LinearClampSampler;
  ID3D11BlendState *ParticleBlendState;

  ID3D11ShaderResourceView *ParticleTex_Srv;

  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_PriticleTileRender(std::string &Name,
                            PASS_TYPE Type,
                            class RSRoot_DX11 *Root);
  RSPass_PriticleTileRender(const RSPass_PriticleTileRender &Source);
  virtual ~RSPass_PriticleTileRender();

public:
  virtual RSPass_PriticleTileRender *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createViews();
  bool
  createSampler();
  bool
  createBlend();
  bool
  checkResources();
};

class RSPass_Sprite : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11PixelShader *PixelShader;
  ID3D11DepthStencilState *DepthStencilState;
  ID3D11BlendState *BlendState;
  ID3D11RenderTargetView *RenderTargetView;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  ID3D11Buffer *ProjStructedBuffer;
  ID3D11ShaderResourceView *ProjStructedBufferSrv;
  ID3D11Buffer *InstanceStructedBuffer;
  ID3D11ShaderResourceView *InstanceStructedBufferSrv;
  ID3D11SamplerState *LinearSampler;
  RS_CAM_INFO *RSCameraInfo;

public:
  RSPass_Sprite(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Sprite(const RSPass_Sprite &Source);
  virtual ~RSPass_Sprite();

public:
  virtual RSPass_Sprite *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createStates();
  bool
  createBuffers();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_SimpleLight : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11PixelShader *PixelShader;
  ID3D11RenderTargetView *RenderTargetView;
  ID3D11SamplerState *LinearWrapSampler;
  ID3D11ShaderResourceView *GeoBufferSrv;
  ID3D11ShaderResourceView *SsaoSrv;
  ID3D11Buffer *VertexBuffer;
  ID3D11Buffer *IndexBuffer;

public:
  RSPass_SimpleLight(std::string &Name,
                     PASS_TYPE Type,
                     class RSRoot_DX11 *Root);
  RSPass_SimpleLight(const RSPass_SimpleLight &Source);
  virtual ~RSPass_SimpleLight();

public:
  virtual RSPass_SimpleLight *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createBuffers();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_Billboard : public RSPass_Base {
private:
  ID3D11VertexShader *VertexShader;
  ID3D11GeometryShader *GeometryShader;
  ID3D11PixelShader *PixelShader;
  ID3D11BlendState *BlendState;
  ID3D11RenderTargetView *RenderTargetView;
  ID3D11DepthStencilView *DepthStencilView;
  ID3D11SamplerState *LinearWrapSampler;
  ID3D11Buffer *ViewProjStructedBuffer;
  ID3D11ShaderResourceView *ViewProjStructedBufferSrv;
  ID3D11Buffer *InstanceStructedBuffer;
  ID3D11ShaderResourceView *InstanceStructedBufferSrv;
  DRAWCALL_TYPE DrawCallType;
  RSDrawCallsPipe *DrawCallPipe;
  RS_CAM_INFO *RSCameraInfo;
  class RSCamera *RSCamera;

public:
  RSPass_Billboard(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_Billboard(const RSPass_Billboard &Source);
  virtual ~RSPass_Billboard();

public:
  virtual RSPass_Billboard *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createStates();
  bool
  createShaders();
  bool
  createBuffers();
  bool
  createViews();
  bool
  createSamplers();
};

class RSPass_Tonemapping : public RSPass_Base {
private:
  ID3D11ComputeShader *AverLuminShader;
  ID3D11ComputeShader *DynamicExposureShader;
  ID3D11ComputeShader *ToneMapShader;
  ID3D11UnorderedAccessView *HdrUav;
  ID3D11ShaderResourceView *HdrSrv;
  std::array<ID3D11Buffer *, 2> AverageLuminBufferArray;
  std::array<ID3D11ShaderResourceView *, 2> AverageLuminSrvArray;
  std::array<ID3D11UnorderedAccessView *, 2> AverageLuminUavArray;

public:
  RSPass_Tonemapping(std::string &Name,
                     PASS_TYPE Type,
                     class RSRoot_DX11 *Root);
  RSPass_Tonemapping(const RSPass_Tonemapping &Source);
  virtual ~RSPass_Tonemapping();

public:
  virtual RSPass_Tonemapping *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createViews();
};

class RSPass_BloomHdr : public RSPass_Base {
private:
  ID3D11ComputeShader *FilterPixelShader;
  ID3D11ComputeShader *KABlurHoriShader;
  ID3D11ComputeShader *KABlurVertShader;
  ID3D11ComputeShader *BlurHoriShader;
  ID3D11ComputeShader *BlurVertShader;
  ID3D11ComputeShader *UpSampleShader;
  ID3D11ComputeShader *BlendShader;
  ID3D11SamplerState *LinearBorderSampler;
  ID3D11Buffer *BlurConstBuffer;
  ID3D11Buffer *IntensityConstBuffer;
  ID3D11ShaderResourceView *HdrSrv;
  ID3D11UnorderedAccessView *HdrUav;
  ID3D11Texture2D *NeedBloomTexture;
  ID3D11ShaderResourceView *NeedBloomSrv;
  std::array<ID3D11UnorderedAccessView *, 10> NeedBloomUavArray;
  ID3D11Texture2D *UpSampleTexture;
  ID3D11ShaderResourceView *UpSampleSrv;
  std::array<ID3D11UnorderedAccessView *, 8> UpSampleUavArray;

public:
  RSPass_BloomHdr(std::string &Name, PASS_TYPE Type, class RSRoot_DX11 *Root);
  RSPass_BloomHdr(const RSPass_BloomHdr &Source);
  virtual ~RSPass_BloomHdr();

public:
  virtual RSPass_BloomHdr *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createViews();
  bool
  createBuffers();
  bool
  createSampler();
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
  virtual RSPass_FXAA *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createShaders();
  bool
  createViews();
  bool
  createSamplers();
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
  virtual RSPass_ToSwapChain *
  clonePass() override;

  virtual bool
  initPass() override;

  virtual void
  releasePass() override;

  virtual void
  execuatePass() override;

private:
  bool
  createBuffers();
  bool
  createShaders();
  bool
  createViews();
  bool
  createSamplers();
};

void
setPipelineDeltaTime(float DeltaMilliSecond);

void
setPipelineIBLTextures(ID3D11ShaderResourceView *EnvSrv,
                       ID3D11ShaderResourceView *DiffSrv,
                       ID3D11ShaderResourceView *SpecSrv);
