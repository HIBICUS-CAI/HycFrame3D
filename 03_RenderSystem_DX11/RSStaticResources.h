//---------------------------------------------------------------
// File: RSStaticResources.h
// Proj: RenderSystem_DX11
// Info: 保存并提供所有常用的资源及其引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include <unordered_map>

class RSStaticResources {
private:
  class RSRoot_DX11 *RenderSystemRoot;

  std::unordered_map<std::string, ID3D11VertexShader *> VertexShaderMap;
  std::unordered_map<std::string, ID3D11GeometryShader *> GeometryShaderMap;
  std::unordered_map<std::string, ID3D11PixelShader *> PixelShaderMap;
  std::unordered_map<std::string, ID3D11ComputeShader *> ComputeShaderMap;

  std::unordered_map<std::string, ID3D11RasterizerState *> RasterizerStateMap;
  std::unordered_map<std::string, ID3D11DepthStencilState *>
      DepthStencilStateMap;
  std::unordered_map<std::string, ID3D11BlendState *> BlendStateMap;

  std::unordered_map<std::string, ID3D11SamplerState *> SamplerMap;

  std::unordered_map<std::string, ID3D11InputLayout *> InputLayoutMap;

  std::unordered_map<std::string, class RSPipeline *> StaticPipelineMap;
  std::unordered_map<std::string, class RSTopic *> StaticTopicMap;

  std::vector<RS_MATERIAL_DATA> MaterialVector;
  std::unordered_map<std::string, UINT> MaterialIndexMap;
  ID3D11Buffer *MaterialBuffer;
  ID3D11ShaderResourceView *MaterialBufferSrv;

public:
  RSStaticResources();
  ~RSStaticResources();

  bool
  startUp(class RSRoot_DX11 *RootPtr);

  void
  cleanAndStop();

  ID3D11VertexShader *
  getStaticVertexShader(const std::string &ShaderName);

  ID3D11GeometryShader *
  getStaticGeometryShader(const std::string &ShaderName);

  ID3D11PixelShader *
  getStaticPixelShader(const std::string &ShaderName);

  ID3D11ComputeShader *
  getStaticComputeShader(const std::string &ShaderName);

  ID3D11RasterizerState *
  getStaticRasterizerState(const std::string &StateName);

  ID3D11DepthStencilState *
  getStaticDepthStencilState(const std::string &StateName);

  ID3D11BlendState *
  getStaticBlendState(const std::string &StateName);

  ID3D11SamplerState *
  getStaticSampler(const std::string &SamplerName);

  ID3D11InputLayout *
  getStaticInputLayout(const std::string &LayoutName);

  const class RSPipeline *const
  getStaticPipeline(const std::string &PipelineName);

  const class RSTopic *const
  getStaticTopic(const std::string &TopicName);

  UINT
  getStaticMaterialIndex(const std::string &MaterialName);

  RS_MATERIAL_DATA *
  getMaterialDataPtrForTest() {
    return &MaterialVector[0];
  }

  void
  remapMaterialData();

  ID3D11ShaderResourceView *
  getMaterialSrv() {
    return MaterialBufferSrv;
  }

private:
  bool
  compileStaticShaders();

  bool
  buildStaticStates();

  bool
  buildStaticInputLayouts();

  bool
  buildStaticTopics();

  bool
  buildStaticPipelines();

  bool
  buildStaticMaterials();
};
