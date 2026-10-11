//---------------------------------------------------------------
// File: RSSpirvReflect.h
// Proj: RenderSystem_VK
// Info: 解析并修改编译后的SPIR-V，提供管线创建所需的反射信息
// Date: 2026.10.11
// Comt: 只处理本项目HLSL着色器实际用到的指令
//---------------------------------------------------------------

#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace rs_tool {

enum class SPIRV_SCALAR_KIND { FLOAT, SINT, UINT };

enum class SPIRV_IMAGE_DIM { NONE, TEX1D, TEX2D, TEX3D, CUBE, BUFFER };

struct SPIRV_BINDING_INFO {
  std::string Name = "";
  uint32_t Binding = 0;
  VkDescriptorType Type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
  SPIRV_IMAGE_DIM Dim = SPIRV_IMAGE_DIM::NONE;
  bool Arrayed = false;
  SPIRV_SCALAR_KIND Scalar = SPIRV_SCALAR_KIND::FLOAT;
  // hidden counter buffer of an append/consume/counter structured buffer
  bool IsCounter = false;
  uint32_t CounterOwnerBinding = 0;
  // array stride of a structured buffer, used to validate c++ structures
  uint32_t ElementStride = 0;
};

struct SPIRV_IO_INFO {
  uint32_t Location = 0;
  // HLSL semantic in upper case, e.g. "TEXCOORD" or "COLOR1"
  std::string Semantic = "";
};

struct SPIRV_REFLECTION {
  VkShaderStageFlagBits Stage = VK_SHADER_STAGE_ALL;
  std::string EntryPoint = "";
  std::vector<SPIRV_BINDING_INFO> Bindings = {};
  std::vector<SPIRV_IO_INFO> Inputs = {};
  std::vector<SPIRV_IO_INFO> Outputs = {};
};

// semantics come from SPV_GOOGLE_hlsl_functionality1 decorations, so this
// must run before stripHlslFunctionality1.
bool reflectSpirv(const std::vector<uint32_t> &Code,
                  SPIRV_REFLECTION &OutReflection);

// glslang leaves counter buffers of append/consume buffers at binding 0,
// move each of them to (binding of the owner buffer + Offset).
void patchCounterBindings(std::vector<uint32_t> &Code, uint32_t Offset);

// images are compiled with unknown storage format, but atomic operations
// require an explicit r32 format, so give it to images used by atomics.
void fixAtomicImageFormats(std::vector<uint32_t> &Code);

// remove SPV_GOOGLE_hlsl_functionality1 / user_type extensions and their
// decorations, so modules do not need VK_GOOGLE_hlsl_functionality1.
void stripHlslFunctionality1(std::vector<uint32_t> &Code);

// change locations of input or output variables by semantic name, used to
// link shader stages by semantic like direct3d does.
void remapLocations(std::vector<uint32_t> &Code,
                    bool ForInputs,
                    const std::unordered_map<std::string, uint32_t> &Locations);

// split "TEXCOORD1" to "TEXCOORD" and 1
void splitSemantic(const std::string &Semantic,
                   std::string &OutName,
                   uint32_t &OutIndex);

} // namespace rs_tool
