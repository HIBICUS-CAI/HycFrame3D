//---------------------------------------------------------------
// File: RSStaticResources.cpp
// Proj: RenderSystem_VK
// Info: 保存并提供所有常用的资源及其引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: NULL
//---------------------------------------------------------------

#include "RSStaticResources.h"

#include "RSDevices.h"
#include "RSPipeline.h"
#include "RSRoot_VK.h"
#include "RSTopic.h"
#include "RSUtilityFunctions.h"
#include "RSVkObjects.h"

#include <TextUtility.h>

#include <cassert>
#include <cstring>

RSStaticResources::RSStaticResources()
    : RenderSystemRoot(nullptr), ShaderMap({}), SamplerMap({}),
      InputLayoutMap({}), StaticPipelineMap({}), StaticTopicMap({}),
      MaterialVector({}), MaterialIndexMap({}), MaterialBuffer(nullptr),
      MaterialBufferSrv(nullptr) {}

RSStaticResources::~RSStaticResources() {}

bool RSStaticResources::startUp(RSRoot_VK *RootPtr) {
  if (!RootPtr) {
    return false;
  }

  RenderSystemRoot = RootPtr;

  if (!compileStaticShaders()) {
    return false;
  }
  if (!buildStaticSamplers()) {
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
  for (auto &Shader : ShaderMap) {
    SAFE_RELEASE(Shader.second);
  }
  ShaderMap.clear();

  for (auto &Sampler : SamplerMap) {
    SAFE_RELEASE(Sampler.second);
  }
  SamplerMap.clear();

  for (auto &Layout : InputLayoutMap) {
    delete Layout.second;
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
  SAFE_RELEASE(MaterialBuffer);
}

bool RSStaticResources::compileStaticShaders() {
  // TEMP----------------------
  return true;
  // TEMP----------------------
}

bool RSStaticResources::buildStaticSamplers() {
  // TEMP----------------------
  return true;
  // TEMP----------------------
}

bool RSStaticResources::buildStaticInputLayouts() {
  // vertex shader inputs are matched to these elements by semantic when a
  // pipeline state object is created
  using namespace vertex_type;

  InputLayoutMap.insert(
      {"BasicVertex",
       new RSInputLayout(
           "BasicVertex", sizeof(BasicVertex),
           {{"POSITION", 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
            {"NORMAL", 0, VK_FORMAT_R32G32B32_SFLOAT, 12},
            {"TEXCOORD", 0, VK_FORMAT_R32G32_SFLOAT, 24}})});

  InputLayoutMap.insert(
      {"ColorVertex",
       new RSInputLayout(
           "ColorVertex", sizeof(ColorVertex),
           {{"POSITION", 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
            {"NORMAL", 0, VK_FORMAT_R32G32B32_SFLOAT, 12},
            {"COLOR", 0, VK_FORMAT_R32G32B32A32_SFLOAT, 24}})});

  InputLayoutMap.insert(
      {"TangentVertex",
       new RSInputLayout(
           "TangentVertex", sizeof(TangentVertex),
           {{"POSITION", 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
            {"NORMAL", 0, VK_FORMAT_R32G32B32_SFLOAT, 12},
            {"TANGENT", 0, VK_FORMAT_R32G32B32_SFLOAT, 24},
            {"TEXCOORD", 0, VK_FORMAT_R32G32_SFLOAT, 36}})});

  InputLayoutMap.insert(
      {"AnimationVertex",
       new RSInputLayout(
           "AnimationVertex", sizeof(AnimationVertex),
           {{"POSITION", 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
            {"NORMAL", 0, VK_FORMAT_R32G32B32_SFLOAT, 12},
            {"TANGENT", 0, VK_FORMAT_R32G32B32_SFLOAT, 24},
            {"TEXCOORD", 0, VK_FORMAT_R32G32_SFLOAT, 36},
            {"BLENDWEIGHT", 0, VK_FORMAT_R32G32B32A32_SFLOAT, 44},
            {"BLENDINDICES", 0, VK_FORMAT_R32G32B32A32_UINT, 60}})});

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
          "RenderSystem_StaticResources/Materials/static-materials.json")) {
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

  auto Devices = RenderSystemRoot->getDevices();

  RS_BUFFER_DESC BufDesc = {};
  BufDesc.Size = MAX_STRUCTURED_BUFFER_SIZE * sizeof(RS_MATERIAL_DATA);
  BufDesc.Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  BufDesc.Memory = RS_MEMORY_USAGE::DYNAMIC;
  BufDesc.StructureStride = sizeof(RS_MATERIAL_DATA);
  MaterialBuffer = Devices->createBuffer(BufDesc);
  FAIL_NULL_RETURN(MaterialBuffer);

  RS_BUFFER_VIEW_DESC SrvDesc = {};
  SrvDesc.NumElements = MAX_STRUCTURED_BUFFER_SIZE;
  SrvDesc.ElementSize = sizeof(RS_MATERIAL_DATA);
  MaterialBufferSrv = Devices->createShaderResourceView(MaterialBuffer, SrvDesc);
  FAIL_NULL_RETURN(MaterialBufferSrv);

  remapMaterialData();

  return true;
}

RSShader *RSStaticResources::getStaticShader(const std::string &ShaderName) {
  auto Found = ShaderMap.find(ShaderName);
  if (Found != ShaderMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

RSSampler *RSStaticResources::getStaticSampler(const std::string &SamplerName) {
  auto Found = SamplerMap.find(SamplerName);
  if (Found != SamplerMap.end()) {
    return Found->second;
  } else {
    return nullptr;
  }
}

const RSInputLayout *
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
  RS_MATERIAL_DATA *MaterialData =
      static_cast<RS_MATERIAL_DATA *>(MaterialBuffer->map());
  std::memcpy(MaterialData, MaterialVector.data(),
              MaterialVector.size() * sizeof(RS_MATERIAL_DATA));
  MaterialBuffer->unmap();
}
