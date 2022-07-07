//---------------------------------------------------------------
// File: RSStaticResources.cpp
// Proj: RenderSystem_DX11
// Info: 保存并提供所有常用的资源及其引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSStaticResources.h"

#include "RSDevices.h"
#include "RSPipeline.h"
#include "RSRoot_DX11.h"
#include "RSShaderCompile.h"
#include "RSTopic.h"
#include "RSUtilityFunctions.h"

#include <TextUtility.h>

RSStaticResources::RSStaticResources()
    : RenderSystemRoot(nullptr), VertexShaderMap({}), GeometryShaderMap({}),
      PixelShaderMap({}), ComputeShaderMap({}), RasterizerStateMap({}),
      DepthStencilStateMap({}), BlendStateMap({}), SamplerMap({}),
      InputLayoutMap({}), StaticPipelineMap({}), StaticTopicMap({}),
      MaterialVector({}), MaterialIndexMap({}), MaterialBuffer(nullptr),
      MaterialBufferSrv(nullptr) {}

RSStaticResources::~RSStaticResources() {}

bool RSStaticResources::startUp(RSRoot_DX11 *RootPtr) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;

  if (!compileStaticShaders()) {
    return false;
  }
  if (!buildStaticStates()) {
    return false;
  }
  if (!buildStaticInputLayouts()) {
    return false;
  }
  if (!buildStaticTopics()) {
    return false;
  }
  if (!buildStaticPipelines()) {
    return false;
  }
  if (!buildStaticMaterials()) {
    return false;
  }

  return true;
}

void RSStaticResources::cleanAndStop() {
  for (auto &VShader : VertexShaderMap) {
    SAFE_RELEASE(VShader.second);
  }
  VertexShaderMap.clear();

  for (auto &GShader : GeometryShaderMap) {
    SAFE_RELEASE(GShader.second);
  }
  GeometryShaderMap.clear();

  for (auto &PShader : PixelShaderMap) {
    SAFE_RELEASE(PShader.second);
  }
  PixelShaderMap.clear();

  for (auto &CShader : ComputeShaderMap) {
    SAFE_RELEASE(CShader.second);
  }
  ComputeShaderMap.clear();

  for (auto &RState : RasterizerStateMap) {
    SAFE_RELEASE(RState.second);
  }
  RasterizerStateMap.clear();

  for (auto &DState : DepthStencilStateMap) {
    SAFE_RELEASE(DState.second);
  }
  DepthStencilStateMap.clear();

  for (auto &BState : BlendStateMap) {
    SAFE_RELEASE(BState.second);
  }
  BlendStateMap.clear();

  for (auto &SState : SamplerMap) {
    SAFE_RELEASE(SState.second);
  }
  SamplerMap.clear();

  for (auto &Layout : InputLayoutMap) {
    SAFE_RELEASE(Layout.second);
  }
  InputLayoutMap.clear();

  for (auto &Pipeline : StaticPipelineMap) {
    Pipeline.second->releasePipeline();
    delete Pipeline.second;
    Pipeline.second = nullptr;
  }
  StaticPipelineMap.clear();

  for (auto &Topic : StaticTopicMap) {
    Topic.second->releaseTopic();
    delete Topic.second;
    Topic.second = nullptr;
  }
  StaticTopicMap.clear();

  MaterialVector.clear();
  MaterialIndexMap.clear();
  SAFE_RELEASE(MaterialBufferSrv);
  SAFE_RELEASE(MaterialBuffer)
}

bool RSStaticResources::compileStaticShaders() {
  // TEMP----------------------
  return true;
  // TEMP----------------------
}

bool RSStaticResources::buildStaticStates() {
  // TEMP----------------------
  return true;
  // TEMP----------------------
}

bool RSStaticResources::buildStaticInputLayouts() {
  ID3DBlob *ShaderBlob = nullptr;
  ID3D11InputLayout *InputLayout = nullptr;
  HRESULT Hr = S_OK;

  {
    Hr = rs_tool::compileShaderFromFile(
        L"RenderSystem_StaticResources\\InputLayouts\\input_layout_basic.hlsl",
        "main", "vs_5_0", &ShaderBlob);
    FAIL_HR_RETURN(Hr);

    D3D11_INPUT_ELEMENT_DESC Layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};
    UINT NumInputLayouts = ARRAYSIZE(Layout);

    Hr = RenderSystemRoot->getDevices()->getDevice()->CreateInputLayout(
        Layout, NumInputLayouts, ShaderBlob->GetBufferPointer(),
        ShaderBlob->GetBufferSize(), &InputLayout);
    ShaderBlob->Release();
    FAIL_HR_RETURN(Hr);
    InputLayoutMap.insert({"BasicVertex", InputLayout});
    ShaderBlob = nullptr;
    InputLayout = nullptr;
  }

  {
    Hr = rs_tool::compileShaderFromFile(
        L"RenderSystem_StaticResources\\InputLayouts\\input_layout_color.hlsl",
        "main", "vs_5_0", &ShaderBlob);
    FAIL_HR_RETURN(Hr);

    D3D11_INPUT_ELEMENT_DESC Layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};
    UINT NumInputLayouts = ARRAYSIZE(Layout);

    Hr = RenderSystemRoot->getDevices()->getDevice()->CreateInputLayout(
        Layout, NumInputLayouts, ShaderBlob->GetBufferPointer(),
        ShaderBlob->GetBufferSize(), &InputLayout);
    ShaderBlob->Release();
    FAIL_HR_RETURN(Hr);
    InputLayoutMap.insert({"ColorVertex", InputLayout});
    ShaderBlob = nullptr;
    InputLayout = nullptr;
  }

  {
    Hr = rs_tool::compileShaderFromFile(
        L"RenderSystem_StaticResources\\InputLayouts\\input_layout_tangent."
        L"hlsl",
        "main", "vs_5_0", &ShaderBlob);
    FAIL_HR_RETURN(Hr);

    D3D11_INPUT_ELEMENT_DESC Layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};
    UINT NumInputLayouts = ARRAYSIZE(Layout);

    Hr = RenderSystemRoot->getDevices()->getDevice()->CreateInputLayout(
        Layout, NumInputLayouts, ShaderBlob->GetBufferPointer(),
        ShaderBlob->GetBufferSize(), &InputLayout);
    ShaderBlob->Release();
    FAIL_HR_RETURN(Hr);
    InputLayoutMap.insert({"TangentVertex", InputLayout});
    ShaderBlob = nullptr;
    InputLayout = nullptr;
  }

  {
    Hr = rs_tool::compileShaderFromFile(
        L"RenderSystem_StaticResources\\InputLayouts\\input_layout_animation."
        L"hlsl",
        "main", "vs_5_0", &ShaderBlob);
    FAIL_HR_RETURN(Hr);

    D3D11_INPUT_ELEMENT_DESC Layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 60,
         D3D11_INPUT_PER_VERTEX_DATA, 0}};
    UINT NumInputLayouts = ARRAYSIZE(Layout);

    Hr = RenderSystemRoot->getDevices()->getDevice()->CreateInputLayout(
        Layout, NumInputLayouts, ShaderBlob->GetBufferPointer(),
        ShaderBlob->GetBufferSize(), &InputLayout);
    ShaderBlob->Release();
    FAIL_HR_RETURN(Hr);
    InputLayoutMap.insert({"AnimationVertex", InputLayout});
    ShaderBlob = nullptr;
    InputLayout = nullptr;
  }

  return true;
}

bool RSStaticResources::buildStaticTopics() {
  // TEMP----------------------
  return true;
  // TEMP----------------------
}

bool RSStaticResources::buildStaticPipelines() {
  // TEMP----------------------
  return true;
  // TEMP----------------------
}

bool RSStaticResources::buildStaticMaterials() {
  using namespace hyc::text;
  JsonFile MaterialFile = {};
  if (!loadJsonAndParse(
          MaterialFile,
          "RenderSystem_StaticResources\\Materials\\static-materials.json")) {
    return false;
  }

  UINT MaterialSize = MaterialFile["static-material"].Size();
  MaterialVector.resize(MaterialSize);
  std::string MaterialName = "";

  for (UINT I = 0; I < MaterialSize; I++) {
    MaterialVector[I].FresnelR0.x =
        MaterialFile["static-material"][I]["fresnel-r0"][0].GetFloat();
    MaterialVector[I].FresnelR0.y =
        MaterialFile["static-material"][I]["fresnel-r0"][1].GetFloat();
    MaterialVector[I].FresnelR0.z =
        MaterialFile["static-material"][I]["fresnel-r0"][2].GetFloat();
    MaterialVector[I].SubSurface =
        MaterialFile["static-material"][I]["subsurface"].GetFloat();
    MaterialVector[I].Metallic =
        MaterialFile["static-material"][I]["metallic"].GetFloat();
    MaterialVector[I].Specular =
        MaterialFile["static-material"][I]["specular"].GetFloat();
    MaterialVector[I].SpecularTint =
        MaterialFile["static-material"][I]["specular-tint"].GetFloat();
    MaterialVector[I].Roughness =
        MaterialFile["static-material"][I]["roughness"].GetFloat();
    MaterialVector[I].Anisotropic =
        MaterialFile["static-material"][I]["anisotropic"].GetFloat();
    MaterialVector[I].Sheen =
        MaterialFile["static-material"][I]["sheen"].GetFloat();
    MaterialVector[I].SheenTint =
        MaterialFile["static-material"][I]["sheen-tint"].GetFloat();
    MaterialVector[I].Clearcoat =
        MaterialFile["static-material"][I]["clearcoat"].GetFloat();
    MaterialVector[I].ClearcoatGloss =
        MaterialFile["static-material"][I]["clearcoat-gloss"].GetFloat();

    MaterialName = MaterialFile["static-material"][I]["name"].GetString();
    MaterialIndexMap.insert({MaterialName, I});
  }

  HRESULT Hr = S_OK;
  D3D11_BUFFER_DESC BufDesc = {};
  D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
  auto DevicePtr = RenderSystemRoot->getDevices()->getDevice();
  ZeroMemory(&BufDesc, sizeof(BufDesc));
  ZeroMemory(&SrvDesc, sizeof(SrvDesc));

  BufDesc.Usage = D3D11_USAGE_DYNAMIC;
  BufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  BufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  BufDesc.ByteWidth = MAX_STRUCTURED_BUFFER_SIZE * sizeof(RS_MATERIAL_DATA);
  BufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  BufDesc.StructureByteStride = sizeof(RS_MATERIAL_DATA);
  Hr = DevicePtr->CreateBuffer(&BufDesc, nullptr, &MaterialBuffer);
  FAIL_HR_RETURN(Hr);

  SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
  SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
  SrvDesc.Buffer.ElementWidth = MAX_STRUCTURED_BUFFER_SIZE;
  Hr = DevicePtr->CreateShaderResourceView(MaterialBuffer, &SrvDesc,
                                           &MaterialBufferSrv);
  FAIL_HR_RETURN(Hr);

  remapMaterialData();

  return true;
}

ID3D11VertexShader *
RSStaticResources::getStaticVertexShader(const std::string &ShaderName) {
  auto Found = VertexShaderMap.find(ShaderName);
  if (Found != VertexShaderMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

ID3D11GeometryShader *
RSStaticResources::getStaticGeometryShader(const std::string &ShaderName) {
  auto Found = GeometryShaderMap.find(ShaderName);
  if (Found != GeometryShaderMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

ID3D11PixelShader *
RSStaticResources::getStaticPixelShader(const std::string &ShaderName) {
  auto Found = PixelShaderMap.find(ShaderName);
  if (Found != PixelShaderMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

ID3D11ComputeShader *
RSStaticResources::getStaticComputeShader(const std::string &ShaderName) {
  auto Found = ComputeShaderMap.find(ShaderName);
  if (Found != ComputeShaderMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

ID3D11RasterizerState *
RSStaticResources::getStaticRasterizerState(const std::string &StateName) {
  auto Found = RasterizerStateMap.find(StateName);
  if (Found != RasterizerStateMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

ID3D11DepthStencilState *
RSStaticResources::getStaticDepthStencilState(const std::string &StateName) {
  auto Found = DepthStencilStateMap.find(StateName);
  if (Found != DepthStencilStateMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

ID3D11BlendState *
RSStaticResources::getStaticBlendState(const std::string &StateName) {
  auto Found = BlendStateMap.find(StateName);
  if (Found != BlendStateMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

ID3D11SamplerState *
RSStaticResources::getStaticSampler(const std::string &SamplerName) {
  auto Found = SamplerMap.find(SamplerName);
  if (Found != SamplerMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

ID3D11InputLayout *
RSStaticResources::getStaticInputLayout(const std::string &LayoutName) {
  auto Found = InputLayoutMap.find(LayoutName);
  if (Found != InputLayoutMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

const RSPipeline *const
RSStaticResources::getStaticPipeline(const std::string &PipelineName) {
  auto Found = StaticPipelineMap.find(PipelineName);
  if (Found != StaticPipelineMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

const RSTopic *const
RSStaticResources::getStaticTopic(const std::string &TopicName) {
  auto Found = StaticTopicMap.find(TopicName);
  if (Found != StaticTopicMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

UINT RSStaticResources::getStaticMaterialIndex(
    const std::string &MaterialName) {
  auto Found = MaterialIndexMap.find(MaterialName);
  assert(Found != MaterialIndexMap.end());
  return Found->second;
}

void RSStaticResources::remapMaterialData() {
  auto ContextPtr = RenderSystemRoot->getDevices()->getSTContext();
  D3D11_MAPPED_SUBRESOURCE msr = {};
  ContextPtr->Map(MaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
  RS_MATERIAL_DATA *MaterialData = (RS_MATERIAL_DATA *)msr.pData;
  memcpy_s(MaterialData, MaterialVector.size() * sizeof(RS_MATERIAL_DATA),
           MaterialVector.data(),
           MaterialVector.size() * sizeof(RS_MATERIAL_DATA));
  ContextPtr->Unmap(MaterialBuffer, 0);
}
