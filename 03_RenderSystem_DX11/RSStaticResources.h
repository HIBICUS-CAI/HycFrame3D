//---------------------------------------------------------------
// File: RSStaticResources.h
// Proj: RenderSystem_VK
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

  // rasterizer/depth/blend states are parts of pipeline state objects in
  // vulkan, so only shaders and samplers are shared here
  std::unordered_map<std::string, class RSShader *> ShaderMap;
  std::unordered_map<std::string, class RSSampler *> SamplerMap;

  std::unordered_map<std::string, class RSInputLayout *> InputLayoutMap;

  std::unordered_map<std::string, class RSPipeline *> StaticPipelineMap;
  std::unordered_map<std::string, class RSTopic *> StaticTopicMap;

  std::vector<RS_MATERIAL_DATA> MaterialVector;
  std::unordered_map<std::string, UINT> MaterialIndexMap;
  RSBuffer *MaterialBuffer;
  RSShaderResourceView *MaterialBufferSrv;

public:
  RSStaticResources();
  ~RSStaticResources();

  bool startUp(class RSRoot_DX11 *RootPtr);
  void cleanAndStop();

  class RSShader *getStaticShader(const std::string &ShaderName);
  class RSSampler *getStaticSampler(const std::string &SamplerName);
  const class RSInputLayout *getStaticInputLayout(const std::string &LayoutName);
  const class RSPipeline *const
  getStaticPipeline(const std::string &PipelineName);
  const class RSTopic *const getStaticTopic(const std::string &TopicName);
  UINT getStaticMaterialIndex(const std::string &MaterialName);
  RS_MATERIAL_DATA *getMaterialDataPtrForTest() { return &MaterialVector[0]; }
  RSShaderResourceView *getMaterialSrv() { return MaterialBufferSrv; }

  void remapMaterialData();

private:
  bool compileStaticShaders();
  bool buildStaticSamplers();
  bool buildStaticInputLayouts();
  bool buildStaticTopics();
  bool buildStaticPipelines();
  bool buildStaticMaterials();
};
