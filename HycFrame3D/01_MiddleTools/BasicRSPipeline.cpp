#include "BasicRSPipeline.h"
#include <string>
#include <vector>
#include <DirectXColors.h>
#include "RSRoot_DX11.h"
#include "RSDevices.h"
#include "RSTopic.h"
#include "RSPipeline.h"
#include "RSPipelinesManager.h"
#include "RSDrawCallsPool.h"
#include "RSCamerasContainer.h"
#include "RSCamera.h"
#include "RSResourceManager.h"
#include "RSShaderCompile.h"

#define RS_RELEASE(p) { if (p) { (p)->Release(); (p)=nullptr; } }
static RSRoot_DX11* g_Root = nullptr;
static RSPipeline* g_BasicPipeline = nullptr;
static D3D11_VIEWPORT g_ViewPort = {};

bool CreateBasicPipeline()
{
    g_Root = GetRSRoot_DX11_Singleton();
    std::string name = "";

    name = "mrt-pass";
    RSPass_MRT* mrt = new RSPass_MRT(name, PASS_TYPE::RENDER, g_Root);
    mrt->SetExecuateOrder(1);

    name = "mrt-topic";
    RSTopic* mrt_topic = new RSTopic(name);
    mrt_topic->StartTopicAssembly();
    mrt_topic->InsertPass(mrt);
    mrt_topic->SetExecuateOrder(1);
    mrt_topic->FinishTopicAssembly();

    name = "light-pipeline";
    g_BasicPipeline = new RSPipeline(name);
    g_BasicPipeline->StartPipelineAssembly();
    g_BasicPipeline->InsertTopic(mrt_topic);
    g_BasicPipeline->FinishPipelineAssembly();

    if (!g_BasicPipeline->InitAllTopics(g_Root->Devices()))
    {
        return false;
    }

    name = g_BasicPipeline->GetPipelineName();
    g_Root->PipelinesManager()->AddPipeline(name, g_BasicPipeline);
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

RSPass_MRT::RSPass_MRT(std::string& _name, PASS_TYPE _type,
    RSRoot_DX11* _root) :
    RSPass_Base(_name, _type, _root),
    mDrawCallType(DRAWCALL_TYPE::OPACITY), mDrawCallPipe(nullptr),
    mVertexShader(nullptr), mPixelShader(nullptr),
    mNDPixelShader(nullptr),
    mViewProjStructedBuffer(nullptr),
    mViewProjStructedBufferSrv(nullptr),
    mInstanceStructedBuffer(nullptr),
    mInstanceStructedBufferSrv(nullptr),
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
    mPixelShader(_source.mPixelShader),
    mNDPixelShader(_source.mNDPixelShader),
    mViewProjStructedBuffer(_source.mViewProjStructedBuffer),
    mInstanceStructedBuffer(_source.mInstanceStructedBuffer),
    mViewProjStructedBufferSrv(_source.mViewProjStructedBufferSrv),
    mInstanceStructedBufferSrv(_source.mInstanceStructedBufferSrv),
    mLinearSampler(_source.mLinearSampler),
    mDiffuseRtv(_source.mDiffuseRtv),
    mNormalRtv(_source.mNormalRtv),
    mDepthDsv(_source.mDepthDsv),
    mRSCameraInfo(_source.mRSCameraInfo),
    mWorldPosRtv(_source.mWorldPosRtv),
    mDiffAlbeRtv(_source.mDiffAlbeRtv),
    mFresShinRtv(_source.mFresShinRtv)
{

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
    if (!CreateShaders()) { return false; }
    if (!CreateBuffers()) { return false; }
    if (!CreateViews()) { return false; }
    if (!CreateSamplers()) { return false; }

    mDrawCallType = DRAWCALL_TYPE::OPACITY;
    mDrawCallPipe = g_Root->DrawCallsPool()->GetDrawCallsPipe(mDrawCallType);

    std::string name = "temp-cam";
    mRSCameraInfo = g_Root->CamerasContainer()->GetRSCameraInfo(name);

    return true;
}

void RSPass_MRT::ReleasePass()
{
    RS_RELEASE(mVertexShader);
    RS_RELEASE(mPixelShader);
    RS_RELEASE(mViewProjStructedBuffer);
    RS_RELEASE(mViewProjStructedBufferSrv);
    RS_RELEASE(mInstanceStructedBuffer);
    RS_RELEASE(mInstanceStructedBufferSrv);
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
    STContext()->VSSetShader(mVertexShader, nullptr, 0);
    STContext()->PSSetShader(mPixelShader, nullptr, 0);

    STContext()->PSSetSamplers(0, 1, &mLinearSampler);

    DirectX::XMMATRIX mat = {};
    DirectX::XMFLOAT4X4 flt44 = {};
    UINT stride = sizeof(VertexType::TangentVertex);
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

    for (auto& call : mDrawCallPipe->mDatas)
    {
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
        STContext()->IASetVertexBuffers(
            0, 1, &call.mMeshData.mVertexBuffer, &stride, &offset);
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
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    hr = Device()->CreateTexture2D(&texDesc, nullptr, &texture);
    if (FAILED(hr)) { return false; }

    rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    hr = Device()->CreateRenderTargetView(texture, &rtvDesc, &mNormalRtv);
    if (FAILED(hr)) { return false; }

    srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
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
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
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
