//---------------------------------------------------------------
// File: RSDevices.h
// Proj: RenderSystem_DX11
// Info: 保存并提供与DirectX直接相关的内容和引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

struct RENDER_DEVICE_CONFIG {
  UINT ForceAdapterIndex = static_cast<UINT>(-1);
  bool ForceSingleThreadFlag = false;
};

class RSDevices {
private:
  class RSRoot_DX11 *RenderSystemRoot;

  RENDER_DEVICE_CONFIG RenderDeivceConfig;

  D3D_DRIVER_TYPE DriveType;
  D3D_FEATURE_LEVEL FeatureLevel;

  ID3D11Device *DX11Device;
  ID3D11DeviceContext *DX11ImmediateContext;
  ID3D11Device1 *DX11Device1;
  ID3D11DeviceContext1 *DX11ImmediateContext1;

  IDXGISwapChain *DXGISwapChain;
  IDXGISwapChain1 *DXGISwapChain1;
  ID3D11RenderTargetView *SwapChainRtv;
  D3D11_VIEWPORT FullWindowViewPort;

  ID3D11Texture2D *HighDynamicTexture;
  ID3D11RenderTargetView *HighDynamicRtv;
  ID3D11ShaderResourceView *HighDynamicSrv;
  ID3D11UnorderedAccessView *HighDynamicUav;

  bool ConcurrentCreateSupportFlag;
  bool CommandListSupportFlag;

  UINT WndWidth;
  UINT WndHeight;

public:
  RSDevices();
  ~RSDevices();

  bool startUp(class RSRoot_DX11 *RootPtr, HWND WndHandle);
  void cleanAndStop();

  ID3D11Device *getDevice() const { return DX11Device; }
  ID3D11DeviceContext *getSTContext() const { return DX11ImmediateContext; }

  ID3D11RenderTargetView *getHighDynamicRtv() const { return HighDynamicRtv; }
  ID3D11RenderTargetView *getSwapChainRtv() const { return SwapChainRtv; }
  ID3D11ShaderResourceView *getHighDynamicSrv() const { return HighDynamicSrv; }
  ID3D11UnorderedAccessView *getHighDynamicUav() const {
    return HighDynamicUav;
  }

  void copyHighDynamicTexture(ID3D11DeviceContext *Context,
                              ID3D11Resource *DstResource) {
    Context->CopyResource(DstResource, HighDynamicTexture);
  }

  bool getConcurrentCreateSupport() const;
  bool getCommandListSupport() const;
  UINT getCurrWndWidth() const;
  UINT getCurrWndHeight() const;

  void presentSwapChain();

private:
  bool createDevices(HWND WndHandle, UINT Width, UINT Height);
  bool createHighDynamicTexture(UINT Width, UINT Height);
  void applyViewPort();
};
