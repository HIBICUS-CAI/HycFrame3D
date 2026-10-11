//---------------------------------------------------------------
// File: RSShaderCompile.h
// Proj: RenderSystem_VK
// Info: 在运行时将HLSL编译为SPIR-V
// Date: 2026.10.11
// Comt: 寄存器与描述符绑定号的对应关系在此定义
//---------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>

// every stage gets its own binding range inside one descriptor set, so
// slots of different stages never collide, same as direct3d 11 where
// VS t0 and PS t0 are different slots.
// binding = stage base + register class base + register index
constexpr uint32_t RS_BINDING_STAGE_STRIDE = 64;
constexpr uint32_t RS_BINDING_VERTEX_BASE = 0;
constexpr uint32_t RS_BINDING_GEOMETRY_BASE = 1 * RS_BINDING_STAGE_STRIDE;
constexpr uint32_t RS_BINDING_PIXEL_BASE = 2 * RS_BINDING_STAGE_STRIDE;
constexpr uint32_t RS_BINDING_COMPUTE_BASE = 0;
constexpr uint32_t RS_BINDING_CBV_OFFSET = 0;      // b0 - b7
constexpr uint32_t RS_BINDING_SRV_OFFSET = 8;      // t0 - t31
constexpr uint32_t RS_BINDING_UAV_OFFSET = 40;     // u0 - u7
constexpr uint32_t RS_BINDING_SAMPLER_OFFSET = 48; // s0 - s7
// hidden counters of append/consume/counter buffers at u# + 16
constexpr uint32_t RS_BINDING_COUNTER_OFFSET = 16;
constexpr uint32_t RS_MAX_CBV_SLOT = 8;
constexpr uint32_t RS_MAX_SRV_SLOT = 32;
constexpr uint32_t RS_MAX_UAV_SLOT = 8;
constexpr uint32_t RS_MAX_SAMPLER_SLOT = 8;

namespace rs_tool {

struct SHADER_MACRO {
  const char *Name;
  const char *Definition;
};

// ShaderModel accepts the direct3d style profile names such as "vs_5_0",
// Macros is terminated by {nullptr, nullptr} like D3D_SHADER_MACRO.
bool compileShaderFromFile(const std::string &FileName,
                           const std::string &EntryPoint,
                           const std::string &ShaderModel,
                           std::vector<uint32_t> &OutSpirv,
                           const SHADER_MACRO *Macros = nullptr);

void finalizeShaderCompiler();

} // namespace rs_tool
