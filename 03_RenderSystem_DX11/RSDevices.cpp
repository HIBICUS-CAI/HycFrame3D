//---------------------------------------------------------------
// File: RSDevices.cpp
// Proj: RenderSystem_DX11
// Info: 保存并提供与DirectX直接相关的内容和引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma comment(lib, "dxgi")

#include "RSDevices.h"

#include "RSRoot_DX11.h"

#include <TextUtility.h>
#include <fstream>

RSDevices::RSDevices()
    : RenderSystemRoot(nullptr), RenderDeivceConfig({}),
      DriveType(D3D_DRIVER_TYPE_HARDWARE), FeatureLevel(D3D_FEATURE_LEVEL_11_1),
      DX11Device(nullptr), DX11ImmediateContext(nullptr), DX11Device1(nullptr),
      DX11ImmediateContext1(nullptr), DXGISwapChain(nullptr),
      DXGISwapChain1(nullptr), SwapChainRtv(nullptr), FullWindowViewPort({}),
      HighDynamicTexture(nullptr), HighDynamicRtv(nullptr),
      HighDynamicSrv(nullptr), HighDynamicUav(nullptr),
      ConcurrentCreateSupportFlag(false), CommandListSupportFlag(false),
      WndWidth(0), WndHeight(0) {}

RSDevices::~RSDevices() {}

bool RSDevices::startUp(RSRoot_DX11 *RootPtr, HWND WndHandle) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;

  {
    using namespace hyc;
    using namespace hyc::text;
    TomlNode ConfigRoot = {};
    TomlNode Node = {};
    std::string ErrorMess = "";
    if (!loadTomlAndParse(ConfigRoot,
                          ".\\Assets\\Configs\\render-device-config.toml",
                          ErrorMess)) {
      return false;
    }

    if (!getTomlNode(ConfigRoot, "device.adapter.manual-setting", Node)) {
      return false;
    } else {
      if (getAs<bool>(Node) &&
          getTomlNode(ConfigRoot, "device.adapter.index-designation", Node)) {
        RenderDeivceConfig.ForceAdapterIndex = getAs<uint>(Node);
      }
    }

    if (!getTomlNode(ConfigRoot, "device.single-thread", Node)) {
      return false;
    } else {
      RenderDeivceConfig.ForceSingleThreadFlag = getAs<bool>(Node);
    }
  }

  WndWidth = 1280;
  WndHeight = 720;

  FullWindowViewPort.Width = static_cast<FLOAT>(WndWidth);
  FullWindowViewPort.Height = static_cast<FLOAT>(WndHeight);
  FullWindowViewPort.MinDepth = 0.f;
  FullWindowViewPort.MaxDepth = 1.f;
  FullWindowViewPort.TopLeftX = 0.f;
  FullWindowViewPort.TopLeftY = 0.f;

  if (!createDevices(WndHandle, WndWidth, WndHeight)) {
    return false;
  }

  if (!createHighDynamicTexture(WndWidth, WndHeight)) {
    return false;
  }

  applyViewPort();

  D3D11_FEATURE_DATA_THREADING THreadSupport = {};
  HRESULT Hr = DX11Device->CheckFeatureSupport(
      D3D11_FEATURE_THREADING, &THreadSupport, sizeof(THreadSupport));
  FAIL_HR_RETURN(Hr);

  if (THreadSupport.DriverConcurrentCreates == TRUE) {
    ConcurrentCreateSupportFlag = true;
  }
  if (THreadSupport.DriverCommandLists == TRUE) {
    CommandListSupportFlag = true;
  }

  return true;
}

void RSDevices::cleanAndStop() {
  if (DX11ImmediateContext) {
    DX11ImmediateContext->ClearState();
  }
  SAFE_RELEASE(HighDynamicUav);
  SAFE_RELEASE(HighDynamicSrv);
  SAFE_RELEASE(HighDynamicRtv);
  SAFE_RELEASE(HighDynamicTexture);
  SAFE_RELEASE(SwapChainRtv);
  SAFE_RELEASE(DX11ImmediateContext1);
  SAFE_RELEASE(DX11ImmediateContext);
  SAFE_RELEASE(DXGISwapChain1);
  SAFE_RELEASE(DXGISwapChain);

#ifdef _DEBUG
  ID3D11Debug *DebugPtr = nullptr;
  HRESULT Hr = DX11Device->QueryInterface(IID_PPV_ARGS(&DebugPtr));
  if (SUCCEEDED(Hr)) {
    Hr = DebugPtr->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    DebugPtr->Release();
    (void)Hr;
  }
#endif // _DEBUG

  SAFE_RELEASE(DX11Device1);
  SAFE_RELEASE(DX11Device);
}

void RSDevices::presentSwapChain() { DXGISwapChain->Present(0, 0); }

bool RSDevices::createDevices(HWND WndHandle, UINT Width, UINT Height) {
  HRESULT Hr = S_OK;

  UINT DeviceCreateFlag = 0;
#ifdef _DEBUG
  DeviceCreateFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG

  D3D_DRIVER_TYPE DriverTypes[] = {D3D_DRIVER_TYPE_HARDWARE,
                                   D3D_DRIVER_TYPE_REFERENCE,
                                   D3D_DRIVER_TYPE_WARP};
  UINT NumDriverTypes = ARRAYSIZE(DriverTypes);
  D3D_FEATURE_LEVEL FeatureLevels[] = {D3D_FEATURE_LEVEL_11_1,
                                       D3D_FEATURE_LEVEL_11_0};
  UINT NumFeatLevels = ARRAYSIZE(FeatureLevels);
  FeatureLevel = FeatureLevels[0];

  std::vector<std::pair<IDXGIAdapter *, std::wstring>> AllAdapters = {};
  AllAdapters.clear();
  {
    IDXGIFactory1 *DxgiFactory = nullptr;
    UINT Index = 0;
    IDXGIAdapter *Adapter = nullptr;

    Hr = CreateDXGIFactory1(IID_PPV_ARGS(&DxgiFactory));
    FAIL_HR_RETURN(Hr);

    while (DxgiFactory->EnumAdapters(Index, &Adapter) != DXGI_ERROR_NOT_FOUND) {
      DXGI_ADAPTER_DESC AdapDesc = {};
      ZeroMemory(&AdapDesc, sizeof(AdapDesc));
      std::wstring WStr = L"";

      Adapter->GetDesc(&AdapDesc);
      WStr = L"--------------------\nAdapter Info in Index : " +
             std::to_wstring(Index++) + L"\n";
      WStr += AdapDesc.Description;
      WStr += L"\n--------------------\n";
      OutputDebugStringW(WStr.c_str());
      AllAdapters.push_back({Adapter, AdapDesc.Description});
    }

    std::wofstream File(
        L".\\Assets\\Configs\\AutoCreated_AdapterInfoInSystem.txt");
    Index = 0;
    for (const auto &AdapInfo : AllAdapters) {
      std::wstring Info = L"--------------------\nAdapter Info in Index : " +
                          std::to_wstring(Index++) + L"\n" + AdapInfo.second +
                          L"\n--------------------\n";
      File << Info;
    }
    File.close();
  }

  UINT AdapterIndex = static_cast<UINT>(AllAdapters.size() - 1);
  if (AllAdapters[AdapterIndex].second == L"Microsoft Basic Render Driver") {
    --AdapterIndex;
  }
  if (RenderDeivceConfig.ForceAdapterIndex != static_cast<UINT>(-1)) {
    AdapterIndex = RenderDeivceConfig.ForceAdapterIndex;
  }

  if (AdapterIndex >= 0 &&
      AdapterIndex < static_cast<UINT>(AllAdapters.size())) {
    Hr = D3D11CreateDevice(AllAdapters[AdapterIndex].first,
                           D3D_DRIVER_TYPE_UNKNOWN, nullptr, DeviceCreateFlag,
                           FeatureLevels, NumFeatLevels, D3D11_SDK_VERSION,
                           &DX11Device, &FeatureLevel, &DX11ImmediateContext);
    if (Hr == E_INVALIDARG) {
      Hr = D3D11CreateDevice(
          AllAdapters[AdapterIndex].first, D3D_DRIVER_TYPE_UNKNOWN, nullptr,
          DeviceCreateFlag, FeatureLevels + 1, NumFeatLevels - 1,
          D3D11_SDK_VERSION, &DX11Device, &FeatureLevel, &DX11ImmediateContext);
    }
  }

  if (!DX11Device && !DX11ImmediateContext) {
    for (UINT I = 0; I < NumDriverTypes; I++) {
      DriveType = DriverTypes[I];
      Hr = D3D11CreateDevice(nullptr, DriveType, nullptr, DeviceCreateFlag,
                             FeatureLevels, NumFeatLevels, D3D11_SDK_VERSION,
                             &DX11Device, &FeatureLevel, &DX11ImmediateContext);

      if (Hr == E_INVALIDARG) {
        Hr = D3D11CreateDevice(nullptr, DriveType, nullptr, DeviceCreateFlag,
                               FeatureLevels + 1, NumFeatLevels - 1,
                               D3D11_SDK_VERSION, &DX11Device, &FeatureLevel,
                               &DX11ImmediateContext);
      }

      if (SUCCEEDED(Hr)) {
        break;
      }
    }
    FAIL_HR_RETURN(Hr);
  }

  if (!DX11Device || !DX11ImmediateContext) {
    return false;
  }

  IDXGIFactory1 *DxgiFactory1 = nullptr;
  {
    IDXGIDevice *DxgiDevice = nullptr;
    Hr = DX11Device->QueryInterface(IID_PPV_ARGS(&DxgiDevice));
    if (SUCCEEDED(Hr)) {
      IDXGIAdapter *Adapter = nullptr;
      Hr = DxgiDevice->GetAdapter(&Adapter);
      if (SUCCEEDED(Hr)) {
        Hr = Adapter->GetParent(IID_PPV_ARGS(&DxgiFactory1));
        Adapter->Release();
      }
      DxgiDevice->Release();
    }
  }
  FAIL_HR_RETURN(Hr);

  IDXGIFactory2 *DxgiFactory2 = nullptr;
  Hr = DxgiFactory1->QueryInterface(IID_PPV_ARGS(&DxgiFactory2));
  (void)Hr;
  if (DxgiFactory2) {
    // 11.1+
    Hr = DX11Device->QueryInterface(IID_PPV_ARGS(&DX11Device1));
    if (SUCCEEDED(Hr)) {
      DX11ImmediateContext->QueryInterface(
          IID_PPV_ARGS(&DX11ImmediateContext1));
    }
    DXGI_SWAP_CHAIN_DESC1 DC = {};
    DC.Width = Width;
    DC.Height = Height;
    DC.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DC.SampleDesc.Count = 1;
    DC.SampleDesc.Quality = 0;
    DC.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    DC.BufferCount = 2;
    DC.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    Hr = DxgiFactory2->CreateSwapChainForHwnd(
        DX11Device, WndHandle, &DC, nullptr, nullptr, &DXGISwapChain1);
    if (SUCCEEDED(Hr)) {
      Hr = DXGISwapChain1->QueryInterface(IID_PPV_ARGS(&DXGISwapChain));
    }

    DxgiFactory2->Release();
  } else {
    // 11.0
    DXGI_SWAP_CHAIN_DESC DC = {};
    DC.BufferCount = 2;
    DC.BufferDesc.Width = Width;
    DC.BufferDesc.Height = Height;
    DC.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DC.BufferDesc.RefreshRate.Numerator = 60;
    DC.BufferDesc.RefreshRate.Denominator = 1;
    DC.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    DC.OutputWindow = WndHandle;
    DC.SampleDesc.Count = 1;
    DC.SampleDesc.Quality = 0;
    DC.Windowed = TRUE;
    DC.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    Hr = DxgiFactory1->CreateSwapChain(DX11Device, &DC, &DXGISwapChain);
  }

  DxgiFactory1->Release();
  FAIL_HR_RETURN(Hr);

  ID3D11Texture2D *BackBuffer = nullptr;
  Hr = DXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&BackBuffer));
  FAIL_HR_RETURN(Hr);

  Hr = DX11Device->CreateRenderTargetView(BackBuffer, nullptr, &SwapChainRtv);
  BackBuffer->Release();
  FAIL_HR_RETURN(Hr);

  return true;
}

bool RSDevices::createHighDynamicTexture(UINT Width, UINT Height) {
  HRESULT Hr = S_OK;
  D3D11_TEXTURE2D_DESC TexDesc = {};
  D3D11_RENDER_TARGET_VIEW_DESC RtvDesc = {};
  D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
  D3D11_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};
  ZeroMemory(&TexDesc, sizeof(TexDesc));
  ZeroMemory(&RtvDesc, sizeof(RtvDesc));
  ZeroMemory(&SrvDesc, sizeof(SrvDesc));
  ZeroMemory(&UavDesc, sizeof(UavDesc));

  TexDesc.Width = Width;
  TexDesc.Height = Height;
  TexDesc.MipLevels = 1;
  TexDesc.ArraySize = 1;
  TexDesc.SampleDesc.Count = 1;
  TexDesc.Usage = D3D11_USAGE_DEFAULT;
  TexDesc.CPUAccessFlags = 0;
  TexDesc.MiscFlags = 0;
  TexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  TexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE |
                      D3D11_BIND_UNORDERED_ACCESS;
  Hr = DX11Device->CreateTexture2D(&TexDesc, nullptr, &HighDynamicTexture);
  FAIL_HR_RETURN(Hr);

  RtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  RtvDesc.Texture2D.MipSlice = 0;
  RtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  Hr = DX11Device->CreateRenderTargetView(HighDynamicTexture, &RtvDesc,
                                          &HighDynamicRtv);
  FAIL_HR_RETURN(Hr);

  SrvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  SrvDesc.Texture2D.MostDetailedMip = 0;
  SrvDesc.Texture2D.MipLevels = 1;
  Hr = DX11Device->CreateShaderResourceView(HighDynamicTexture, &SrvDesc,
                                            &HighDynamicSrv);
  FAIL_HR_RETURN(Hr);

  UavDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
  UavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
  UavDesc.Texture2D.MipSlice = 0;
  Hr = DX11Device->CreateUnorderedAccessView(HighDynamicTexture, &UavDesc,
                                             &HighDynamicUav);
  FAIL_HR_RETURN(Hr);

  return true;
}

void RSDevices::applyViewPort() {
  DX11ImmediateContext->RSSetViewports(1, &FullWindowViewPort);
}

bool RSDevices::getConcurrentCreateSupport() const {
  return ConcurrentCreateSupportFlag;
}

bool RSDevices::getCommandListSupport() const {
  return CommandListSupportFlag && (!RenderDeivceConfig.ForceSingleThreadFlag);
}

UINT RSDevices::getCurrWndWidth() const { return WndWidth; }

UINT RSDevices::getCurrWndHeight() const { return WndHeight; }
