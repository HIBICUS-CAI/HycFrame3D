﻿//---------------------------------------------------------------
// File: RSDevices.h
// Proj: RenderSystem_DX11
// Info: 保存并提供与DirectX直接相关的内容和引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

struct RENDER_DEVICE_CONFIG
{
    UINT mForceAdapterIndex = (UINT)-1;
    bool mForceSingleThread = false;
};

class RSDevices
{
public:
    RSDevices();
    ~RSDevices();

    bool StartUp(class RSRoot_DX11* _root, HWND _wnd);
    void CleanAndStop();

    ID3D11Device* GetDevice() const { return mDevice; }
    ID3D11DeviceContext* GetSTContext() const { return mImmediateContext; }
    ID3D11RenderTargetView* GetHighDynamicRtv() const { return mHighDynamicRtv; }
    ID3D11RenderTargetView* GetSwapChainRtv() const { return mSwapChainRtv; }
    ID3D11ShaderResourceView* GetHighDynamicSrv() const { return mHighDynamicSrv; }
    ID3D11UnorderedAccessView* GetHighDynamicUav() const { return mHighDynamicUav; }
    void CopyHighDynamicTexture(ID3D11DeviceContext* _context,
        ID3D11Resource* _dstResource)
    {
        _context->CopyResource(_dstResource, mHighDynamicTexture);
    }

    bool GetConcurrentCreateSupport() const;
    bool GetCommandListSupport() const;

    UINT GetCurrWndWidth() const;
    UINT GetCurrWndHeight() const;

    void PresentSwapChain();

private:
    bool CreateDevices(HWND _wnd, UINT _width, UINT _height);
    bool CreateHighDynamicTexture(UINT _width, UINT _height);
    void ApplyViewPort();

private:
    class RSRoot_DX11* mRootPtr;

    RENDER_DEVICE_CONFIG mRenderDeivceConfig;

    D3D_DRIVER_TYPE mDriveType;
    D3D_FEATURE_LEVEL mFeatureLevel;

    ID3D11Device* mDevice;
    ID3D11DeviceContext* mImmediateContext;
    ID3D11Device1* mDevice1;
    ID3D11DeviceContext1* mImmediateContext1;

    IDXGISwapChain* mDXGISwapChain;
    IDXGISwapChain1* mDXGISwapChain1;
    ID3D11RenderTargetView* mSwapChainRtv;
    D3D11_VIEWPORT mFullWindowViewPort;

    ID3D11Texture2D* mHighDynamicTexture;
    ID3D11RenderTargetView* mHighDynamicRtv;
    ID3D11ShaderResourceView* mHighDynamicSrv;
    ID3D11UnorderedAccessView* mHighDynamicUav;

    bool mConcurrentCreateSupport;
    bool mCommandListSupport;

    UINT mWndWidth;
    UINT mWndHeight;
};
