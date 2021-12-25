#pragma once

#include <vector>
#include <array>
#include "RSPass_Base.h"

bool CreateBasicPipeline();

struct ViewProj
{
    DirectX::XMFLOAT4X4 mViewMat = {};
    DirectX::XMFLOAT4X4 mProjMat = {};
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
    ID3D11PixelShader* mPixelShader;
    ID3D11PixelShader* mNDPixelShader;
    DRAWCALL_TYPE mDrawCallType;
    RSDrawCallsPipe* mDrawCallPipe;
    ID3D11Buffer* mViewProjStructedBuffer;
    ID3D11ShaderResourceView* mViewProjStructedBufferSrv;
    ID3D11Buffer* mInstanceStructedBuffer;
    ID3D11ShaderResourceView* mInstanceStructedBufferSrv;
    ID3D11SamplerState* mLinearSampler;
    ID3D11RenderTargetView* mDiffuseRtv;
    ID3D11RenderTargetView* mNormalRtv;
    ID3D11RenderTargetView* mWorldPosRtv;
    ID3D11RenderTargetView* mDiffAlbeRtv;
    ID3D11RenderTargetView* mFresShinRtv;
    ID3D11DepthStencilView* mDepthDsv;
    RS_CAM_INFO* mRSCameraInfo;
};
