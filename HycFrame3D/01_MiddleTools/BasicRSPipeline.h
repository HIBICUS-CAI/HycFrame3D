#pragma once

#include <vector>
#include <array>
#include "RSPass_Base.h"

bool CreateBasicPipeline();

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
constexpr UINT PTC_MAX_COARSE_CULL_TILE_SIZE = PTC_MAX_COARSE_CULL_TILE_X * PTC_MAX_COARSE_CULL_TILE_Y;
constexpr UINT PTC_NUM_PER_TILE = 1023;
constexpr UINT PTC_TILE_BUFFER_SIZE = PTC_NUM_PER_TILE + 1;
constexpr UINT PTC_TILE_X_SIZE = 32;
constexpr UINT PTC_TILE_Y_SIZE = 32;
constexpr UINT PTC_COARSE_CULLING_THREADS = 256;

struct BLM_BLUR_INFO
{
    UINT mTexWidth;
    UINT mTexHeight;
    UINT mPads[2];
};

struct RS_PARTICLE_PART_A
{
    DirectX::XMFLOAT4 mColorAndAlpha = {};
    DirectX::XMFLOAT2 mViewSpaceVelocityXY = {};
    float mEmitterNormalDotLight = 0.f;
    UINT mEmitterProperties = 0;
    float mRotation = 0.f;
    UINT mIsSleeping = 0;
    UINT mCollisionCount = 0;
    float mPads[1] = { 0.f };
};

struct RS_PARTICLE_PART_B
{
    DirectX::XMFLOAT3 mWorldPosition = {};
    float mMass = 0.f;
    DirectX::XMFLOAT3 mWorldSpaceVelocity = {};
    float mLifeSpan = 0.f;
    float mDistanceToEye = 0.f;
    float mAge = 0.f;
    float mStartSize = 0.f;
    float mEndSize = 0.f;
    DirectX::XMFLOAT4 mStartColor = {};
    DirectX::XMFLOAT4 mEndColor = {};
    DirectX::XMFLOAT3 mAcceleration = {};
    float mPads[1] = { 0.f };
};

struct RS_ALIVE_INDEX_BUFFER_ELEMENT
{
    float mDistance;
    float mIndex;
};

struct CAMERA_STATUS
{
    DirectX::XMFLOAT4X4 mView = {};
    DirectX::XMFLOAT4X4 mInvView = {};
    DirectX::XMFLOAT4X4 mProj = {};
    DirectX::XMFLOAT4X4 mInvProj = {};
    DirectX::XMFLOAT4X4 mViewProj = {};
    DirectX::XMFLOAT3 mEyePosition = {};
    float mPad[1] = { 0.f };
};

struct SIMULATE_EMITTER_INFO
{
    DirectX::XMFLOAT3 mWorldPosition = {};
    float mPads[1] = { 0.f };
};

struct RS_TILING_CONSTANT
{
    UINT mNumTilesX = 0;
    UINT mNumTilesY = 0;
    UINT mNumCoarseCullingTilesX = 0;
    UINT mNumCoarseCullingTilesY = 0;
    UINT mNumCullingTilesPerCoarseTileX = 0;
    UINT mNumCullingTilesPerCoarseTileY = 0;
    UINT mPads[2] = { 0 };
};

struct PTC_TIME_CONSTANT
{
    float mDeltaTime = 0.016f;
    float mTotalTime = 0.f;
    float mPads[2] = { 0.f };
};

struct ViewProj
{
    DirectX::XMFLOAT4X4 mViewMat = {};
    DirectX::XMFLOAT4X4 mProjMat = {};
};

struct ViewProjCamUpPos
{
    DirectX::XMFLOAT4X4 mViewMat = {};
    DirectX::XMFLOAT4X4 mProjMat = {};
    DirectX::XMFLOAT3 mCamUpVec = {};
    DirectX::XMFLOAT3 mCamPos = {};
};

struct Ambient
{
    DirectX::XMFLOAT4 mAmbient = {};
};

struct LightInfo
{
    DirectX::XMFLOAT3 mCameraPos = {};
    float mPad0 = 0.f;
    UINT mDirectLightNum = 0;
    UINT mSpotLightNum = 0;
    UINT mPointLightNum = 0;
    UINT mShadowLightNum = 0;
    INT mShadowLightIndex[4] = { -1,-1,-1,-1 };
};

struct ShadowInfo
{
    DirectX::XMFLOAT4X4 mShadowViewMat = {};
    DirectX::XMFLOAT4X4 mShadowProjMat = {};
    DirectX::XMFLOAT4X4 mSSAOMat = {};
};

struct SsaoInfo
{
    DirectX::XMFLOAT4X4 mProj;
    DirectX::XMFLOAT4X4 mView;
    DirectX::XMFLOAT4X4 mInvProj;
    DirectX::XMFLOAT4X4 mTexProj;
    DirectX::XMFLOAT4 mOffsetVec[14];
    float mOcclusionRadius;
    float mOcclusionFadeStart;
    float mOcclusionFadeEnd;
    float mSurfaceEpsilon;
};

struct SkyShpereInfo
{
    DirectX::XMFLOAT4X4 mWorldMat = {};
    DirectX::XMFLOAT4X4 mViewMat = {};
    DirectX::XMFLOAT4X4 mProjMat = {};
    DirectX::XMFLOAT3 mEyePosition = {};
    float mPad = 0.f;
};

struct OnlyProj
{
    DirectX::XMFLOAT4X4 mProjMat = {};
};

class RSPass_MRT :public RSPass_Base
{
public:
    RSPass_MRT(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_MRT(const RSPass_MRT& _source);
    virtual ~RSPass_MRT();

public:
    virtual RSPass_MRT* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateBuffers();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11VertexShader* mAniVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11PixelShader* mNDPixelShader;
    DRAWCALL_TYPE mDrawCallType;
    RSDrawCallsPipe* mDrawCallPipe;
    ID3D11Buffer* mViewProjStructedBuffer;
    ID3D11ShaderResourceView* mViewProjStructedBufferSrv;
    ID3D11Buffer* mInstanceStructedBuffer;
    ID3D11ShaderResourceView* mInstanceStructedBufferSrv;
    ID3D11Buffer* mBonesStructedBuffer;
    ID3D11ShaderResourceView* mBonesStructedBufferSrv;
    ID3D11SamplerState* mLinearSampler;
    ID3D11RenderTargetView* mGeoBufferRtv;
    ID3D11RenderTargetView* mAnisotropicRtv;
    ID3D11DepthStencilView* mDepthDsv;
    RS_CAM_INFO* mRSCameraInfo;
};

class RSPass_Ssao :public RSPass_Base
{
public:
    RSPass_Ssao(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_Ssao(const RSPass_Ssao& _source);
    virtual ~RSPass_Ssao();

public:
    virtual RSPass_Ssao* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateBuffers();
    bool CreateTextures();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11VertexShader* mCompressVertexShader;
    ID3D11PixelShader* mCompressPixelShader;
    ID3D11RenderTargetView* mRenderTargetView;
    ID3D11ShaderResourceView* mNotCompressSrv;
    ID3D11RenderTargetView* mCompressRtv;
    ID3D11SamplerState* mSamplePointClamp;
    ID3D11SamplerState* mSampleLinearClamp;
    ID3D11SamplerState* mSampleDepthMap;
    ID3D11SamplerState* mSampleLinearWrap;
    ID3D11Buffer* mSsaoInfoStructedBuffer;
    ID3D11ShaderResourceView* mSsaoInfoStructedBufferSrv;
    ID3D11ShaderResourceView* mGeoBufferSrv;
    ID3D11ShaderResourceView* mDepthMapSrv;
    ID3D11ShaderResourceView* mRandomMapSrv;
    DirectX::XMFLOAT4 mOffsetVec[14];
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
    RS_CAM_INFO* mRSCameraInfo;
};

class RSPass_KBBlur :public RSPass_Base
{
public:
    RSPass_KBBlur(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_KBBlur(const RSPass_KBBlur& _source);
    virtual ~RSPass_KBBlur();

public:
    virtual RSPass_KBBlur* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateViews();

private:
    ID3D11ComputeShader* mHoriBlurShader;
    ID3D11ComputeShader* mVertBlurShader;
    ID3D11UnorderedAccessView* mSsaoTexUav;
    ID3D11ShaderResourceView* mGeoBufferSrv;
    ID3D11ShaderResourceView* mDepthMapSrv;
};

class RSPass_Shadow :public RSPass_Base
{
public:
    RSPass_Shadow(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_Shadow(const RSPass_Shadow& _source);
    virtual ~RSPass_Shadow();

public:
    virtual RSPass_Shadow* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateStates();
    bool CreateBuffers();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11VertexShader* mAniVertexShader;
    ID3D11RasterizerState* mRasterizerState;
    std::array<ID3D11DepthStencilView*, MAX_SHADOW_SIZE> mDepthStencilView;
    DRAWCALL_TYPE mDrawCallType;
    RSDrawCallsPipe* mDrawCallPipe;
    ID3D11Buffer* mViewProjStructedBuffer;
    ID3D11ShaderResourceView* mViewProjStructedBufferSrv;
    ID3D11Buffer* mInstanceStructedBuffer;
    ID3D11ShaderResourceView* mInstanceStructedBufferSrv;
    ID3D11Buffer* mBonesStructedBuffer;
    ID3D11ShaderResourceView* mBonesStructedBufferSrv;
};

class RSPass_Defered :public RSPass_Base
{
public:
    RSPass_Defered(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_Defered(const RSPass_Defered& _source);
    virtual ~RSPass_Defered();

public:
    virtual RSPass_Defered* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateBuffers();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11RenderTargetView* mRenderTargetView;
    ID3D11SamplerState* mLinearWrapSampler;
    ID3D11SamplerState* mPointClampSampler;
    ID3D11SamplerState* mShadowTexSampler;
    ID3D11Buffer* mLightInfoStructedBuffer;
    ID3D11ShaderResourceView* mLightInfoStructedBufferSrv;
    ID3D11Buffer* mLightStructedBuffer;
    ID3D11ShaderResourceView* mLightStructedBufferSrv;
    ID3D11Buffer* mAmbientStructedBuffer;
    ID3D11ShaderResourceView* mAmbientStructedBufferSrv;
    ID3D11Buffer* mShadowStructedBuffer;
    ID3D11ShaderResourceView* mShadowStructedBufferSrv;
    ID3D11Buffer* mCameraStructedBuffer;
    ID3D11ShaderResourceView* mCameraStructedBufferSrv;
    ID3D11ShaderResourceView* mGeoBufferSrv;
    ID3D11ShaderResourceView* mAnisotropicSrv;
    ID3D11ShaderResourceView* mSsaoSrv;
    ID3D11ShaderResourceView* mShadowDepthSrv;
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
    RS_CAM_INFO* mRSCameraInfo;
};

class RSPass_SkyShpere :public RSPass_Base
{
public:
    RSPass_SkyShpere(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_SkyShpere(const RSPass_SkyShpere& _source);
    virtual ~RSPass_SkyShpere();

public:
    virtual RSPass_SkyShpere* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateStates();
    bool CreateBuffers();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11RasterizerState* mRasterizerState;
    ID3D11DepthStencilState* mDepthStencilState;
    ID3D11SamplerState* mLinearWrapSampler;
    ID3D11RenderTargetView* mRenderTargerView;
    ID3D11DepthStencilView* mDepthStencilView;
    ID3D11Buffer* mSkyShpereInfoStructedBuffer;
    ID3D11ShaderResourceView* mSkyShpereInfoStructedBufferSrv;
    RS_SUBMESH_DATA mSkySphereMesh;
    RS_CAM_INFO* mRSCameraInfo;
};

class RSPass_Bloom :public RSPass_Base
{
public:
    RSPass_Bloom(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_Bloom(const RSPass_Bloom& _source);
    virtual ~RSPass_Bloom();

public:
    virtual RSPass_Bloom* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateBuffers();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    DRAWCALL_TYPE mDrawCallType;
    RSDrawCallsPipe* mDrawCallPipe;
    ID3D11Buffer* mViewProjStructedBuffer;
    ID3D11ShaderResourceView* mViewProjStructedBufferSrv;
    ID3D11Buffer* mInstanceStructedBuffer;
    ID3D11ShaderResourceView* mInstanceStructedBufferSrv;
    ID3D11RenderTargetView* mRtv;
    ID3D11DepthStencilView* mDepthDsv;
    RS_CAM_INFO* mRSCameraInfo;
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
    ID3D11SamplerState* mSampler;
};

class RSPass_PriticleSetUp :public RSPass_Base
{
public:
    RSPass_PriticleSetUp(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_PriticleSetUp(const RSPass_PriticleSetUp& _source);
    virtual ~RSPass_PriticleSetUp();

    const RS_TILING_CONSTANT& GetTilingConstantInfo() const;

public:
    virtual RSPass_PriticleSetUp* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateBuffers();
    bool CreateViews();

private:
    RS_TILING_CONSTANT mTilingConstant;

    ID3D11Buffer* mParticleRenderBuffer;
    ID3D11ShaderResourceView* mParticleRender_Srv;
    ID3D11UnorderedAccessView* mParticleRender_Uav;

    ID3D11Buffer* mParticlePartA;
    ID3D11ShaderResourceView* mPartA_Srv;
    ID3D11UnorderedAccessView* mPartA_Uav;

    ID3D11Buffer* mParticlePartB;
    ID3D11UnorderedAccessView* mPartB_Uav;

    ID3D11Buffer* mViewspacePosBuffer;
    ID3D11ShaderResourceView* mViewSpacePos_Srv;
    ID3D11UnorderedAccessView* mViewSpacePos_Uav;

    ID3D11Buffer* mMaxRadiusBuffer;
    ID3D11ShaderResourceView* mMaxRadius_Srv;
    ID3D11UnorderedAccessView* mMaxRadius_Uav;

    ID3D11Buffer* mStridedCoarseCullBuffer;
    ID3D11ShaderResourceView* mStridedCoarseCull_Srv;
    ID3D11UnorderedAccessView* mStridedCoarseCull_Uav;

    ID3D11Buffer* mStridedCoarseCullCounterBuffer;
    ID3D11ShaderResourceView* mStridedCoarseCullCounter_Srv;
    ID3D11UnorderedAccessView* mStridedCoarseCullCounter_Uav;

    ID3D11Buffer* mTiledIndexBuffer;
    ID3D11ShaderResourceView* mTiledIndex_Srv;
    ID3D11UnorderedAccessView* mTiledIndex_Uav;

    ID3D11Buffer* mDeadListBuffer;
    ID3D11UnorderedAccessView* mDeadList_Uav;

    ID3D11Buffer* mAliveIndexBuffer;
    ID3D11ShaderResourceView* mAliveIndex_Srv;
    ID3D11UnorderedAccessView* mAliveIndex_Uav;

    ID3D11Buffer* mDeadListConstantBuffer;
    ID3D11Buffer* mActiveListConstantBuffer;

    ID3D11Buffer* mEmitterConstantBuffer;
    ID3D11Buffer* mCameraConstantBuffer;
    ID3D11Buffer* mTilingConstantBuffer;
    ID3D11Buffer* mTimeConstantBuffer;

    ID3D11Buffer* mDebugCounterBuffer;

    ID3D11Texture2D* mParticleRandomTexture;
    ID3D11ShaderResourceView* mParticleRandom_Srv;

    ID3D11Buffer* mSimulEmitterStructedBuffer;
    ID3D11ShaderResourceView* mSimulEmitterStructedBuffer_Srv;
};

class RSPass_PriticleEmitSimulate :public RSPass_Base
{
public:
    RSPass_PriticleEmitSimulate(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_PriticleEmitSimulate(const RSPass_PriticleEmitSimulate& _source);
    virtual ~RSPass_PriticleEmitSimulate();

public:
    virtual RSPass_PriticleEmitSimulate* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateSampler();
    bool CheckResources();

private:
    class RSParticlesContainer* mRSParticleContainerPtr;

    ID3D11ComputeShader* mInitDeadListShader;
    ID3D11ComputeShader* mResetParticlesShader;
    ID3D11ComputeShader* mEmitParticleShader;
    ID3D11ComputeShader* mSimulateShader;

    ID3D11ShaderResourceView* mDepthTex_Srv;
    ID3D11ShaderResourceView* mRandomTex_Srv;
    ID3D11ShaderResourceView* mSimulEmitterStructedBuffer_Srv;
    ID3D11UnorderedAccessView* mDeadList_Uav;
    ID3D11UnorderedAccessView* mPartA_Uav;
    ID3D11UnorderedAccessView* mPartB_Uav;
    ID3D11UnorderedAccessView* mAliveIndex_Uav;
    ID3D11UnorderedAccessView* mViewSpacePos_Uav;
    ID3D11UnorderedAccessView* mMaxRadius_Uav;
    ID3D11Buffer* mEmitterConstantBuffer;
    ID3D11Buffer* mCameraConstantBuffer;
    ID3D11Buffer* mDeadListConstantBuffer;
    ID3D11Buffer* mSimulEmitterStructedBuffer;
    ID3D11Buffer* mTimeConstantBuffer;

    ID3D11SamplerState* mLinearWrapSampler;

    RS_CAM_INFO* mRSCameraInfo;
};

class RSPass_PriticleTileRender :public RSPass_Base
{
public:
    RSPass_PriticleTileRender(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_PriticleTileRender(const RSPass_PriticleTileRender& _source);
    virtual ~RSPass_PriticleTileRender();

public:
    virtual RSPass_PriticleTileRender* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateViews();
    bool CreateSampler();
    bool CreateBlend();
    bool CheckResources();

private:
    ID3D11ComputeShader* mCoarseCullingShader;
    ID3D11ComputeShader* mTileCullingShader;
    ID3D11ComputeShader* mTileRenderShader;
    ID3D11VertexShader* mBlendVertexShader;
    ID3D11PixelShader* mBlendPixelShader;

    ID3D11Buffer* mCameraConstantBuffer;
    ID3D11Buffer* mTilingConstantBuffer;
    ID3D11Buffer* mActiveListConstantBuffer;
    ID3D11ShaderResourceView* mDepthTex_Srv;
    ID3D11ShaderResourceView* mViewSpacePos_Srv;
    ID3D11ShaderResourceView* mMaxRadius_Srv;
    ID3D11ShaderResourceView* mPartA_Srv;
    ID3D11ShaderResourceView* mAliveIndex_Srv;
    ID3D11UnorderedAccessView* mAliveIndex_Uav;
    ID3D11ShaderResourceView* mCoarseTileIndex_Srv;
    ID3D11UnorderedAccessView* mCoarseTileIndex_Uav;
    ID3D11ShaderResourceView* mCoarseTileIndexCounter_Srv;
    ID3D11UnorderedAccessView* mCoarseTileIndexCounter_Uav;
    ID3D11ShaderResourceView* mTiledIndex_Srv;
    ID3D11UnorderedAccessView* mTiledIndex_Uav;
    ID3D11ShaderResourceView* mParticleRender_Srv;
    ID3D11UnorderedAccessView* mParticleRender_Uav;

    ID3D11SamplerState* mLinearClampSampler;
    ID3D11BlendState* mParticleBlendState;

    ID3D11ShaderResourceView* mParticleTex_Srv;

    RS_CAM_INFO* mRSCameraInfo;
};

class RSPass_Sprite :public RSPass_Base
{
public:
    RSPass_Sprite(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_Sprite(const RSPass_Sprite& _source);
    virtual ~RSPass_Sprite();

public:
    virtual RSPass_Sprite* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateStates();
    bool CreateBuffers();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11DepthStencilState* mDepthStencilState;
    ID3D11BlendState* mBlendState;
    ID3D11RenderTargetView* mRenderTargetView;
    DRAWCALL_TYPE mDrawCallType;
    RSDrawCallsPipe* mDrawCallPipe;
    ID3D11Buffer* mProjStructedBuffer;
    ID3D11ShaderResourceView* mProjStructedBufferSrv;
    ID3D11Buffer* mInstanceStructedBuffer;
    ID3D11ShaderResourceView* mInstanceStructedBufferSrv;
    ID3D11SamplerState* mLinearSampler;
    RS_CAM_INFO* mRSCameraInfo;
};

class RSPass_SimpleLight :public RSPass_Base
{
public:
    RSPass_SimpleLight(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_SimpleLight(const RSPass_SimpleLight& _source);
    virtual ~RSPass_SimpleLight();

public:
    virtual RSPass_SimpleLight* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateBuffers();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11RenderTargetView* mRenderTargetView;
    ID3D11SamplerState* mLinearWrapSampler;
    ID3D11ShaderResourceView* mGeoBufferSrv;
    ID3D11ShaderResourceView* mSsaoSrv;
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
};

class RSPass_Billboard :public RSPass_Base
{
public:
    RSPass_Billboard(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_Billboard(const RSPass_Billboard& _source);
    virtual ~RSPass_Billboard();

public:
    virtual RSPass_Billboard* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateStates();
    bool CreateShaders();
    bool CreateBuffers();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11VertexShader* mVertexShader;
    ID3D11GeometryShader* mGeometryShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11BlendState* mBlendState;
    ID3D11RenderTargetView* mRenderTargetView;
    ID3D11DepthStencilView* mDepthStencilView;
    ID3D11SamplerState* mLinearWrapSampler;
    ID3D11Buffer* mViewProjStructedBuffer;
    ID3D11ShaderResourceView* mViewProjStructedBufferSrv;
    ID3D11Buffer* mInstanceStructedBuffer;
    ID3D11ShaderResourceView* mInstanceStructedBufferSrv;
    DRAWCALL_TYPE mDrawCallType;
    RSDrawCallsPipe* mDrawCallPipe;
    RS_CAM_INFO* mRSCameraInfo;
    class RSCamera* mRSCamera;
};

class RSPass_Tonemapping :public RSPass_Base
{
public:
    RSPass_Tonemapping(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_Tonemapping(const RSPass_Tonemapping& _source);
    virtual ~RSPass_Tonemapping();

public:
    virtual RSPass_Tonemapping* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateViews();

private:
    ID3D11ComputeShader* mComputeShader;
    ID3D11UnorderedAccessView* mHdrUav;
};

class RSPass_BloomHdr :public RSPass_Base
{
public:
    RSPass_BloomHdr(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_BloomHdr(const RSPass_BloomHdr& _source);
    virtual ~RSPass_BloomHdr();

public:
    virtual RSPass_BloomHdr* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateShaders();
    bool CreateViews();
    bool CreateBuffers();
    bool CreateSampler();

private:
    ID3D11ComputeShader* mFilterPixelShader;
    ID3D11ComputeShader* mBlurHoriShader;
    ID3D11ComputeShader* mBlurVertShader;
    ID3D11ComputeShader* mUpSampleShader;
    ID3D11ComputeShader* mBlendShader;
    ID3D11SamplerState* mLinearClampSampler;
    ID3D11Buffer* mBlurConstBuffer;
    ID3D11ShaderResourceView* mHdrSrv;
    ID3D11UnorderedAccessView* mHdrUav;
    ID3D11Texture2D* mNeedBloomTexture;
    ID3D11ShaderResourceView* mNeedBloomSrv;
    std::array<ID3D11UnorderedAccessView*, 10> mNeedBloomUavArray;
    ID3D11Texture2D* mUpSampleTexture;
    ID3D11ShaderResourceView* mUpSampleSrv;
    std::array<ID3D11UnorderedAccessView*, 8> mUpSampleUavArray;
};

class RSPass_ToSwapChain :public RSPass_Base
{
public:
    RSPass_ToSwapChain(std::string& _name, PASS_TYPE _type,
        class RSRoot_DX11* _root);
    RSPass_ToSwapChain(const RSPass_ToSwapChain& _source);
    virtual ~RSPass_ToSwapChain();

public:
    virtual RSPass_ToSwapChain* ClonePass() override;

    virtual bool InitPass();

    virtual void ReleasePass();

    virtual void ExecuatePass();

private:
    bool CreateBuffers();
    bool CreateShaders();
    bool CreateViews();
    bool CreateSamplers();

private:
    ID3D11Buffer* mVertexBuffer;
    ID3D11Buffer* mIndexBuffer;
    ID3D11VertexShader* mVertexShader;
    ID3D11PixelShader* mPixelShader;
    ID3D11RenderTargetView* mSwapChainRtv;
    ID3D11ShaderResourceView* mHdrSrv;
    ID3D11SamplerState* mLinearWrapSampler;
};

void SetPipelineDeltaTime(float _deltaMilliSecond);

void SetPipelineIBLTextures(ID3D11ShaderResourceView* _envSrv,
    ID3D11ShaderResourceView* _diffSrv,
    ID3D11ShaderResourceView* _specSrv);
