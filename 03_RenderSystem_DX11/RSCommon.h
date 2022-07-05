//---------------------------------------------------------------
// File: RSCommon.h
// Proj: RenderSystem_DX11
// Info: 声明RenderSystem_DX11相关的全局类型
// Date: 2021.9.14
// Mail: cai_genkan@outlook.com
// Comt: 在此lib项目内中提供ExtraMacro
//---------------------------------------------------------------

#pragma once

#include <DirectXMath.h>
#include <Windows.h>
#include <array>
#include <d3d11_1.h>
#include <string>
#include <vector>

namespace dx = DirectX;

enum class PASS_TYPE { RENDER, COMPUTE };

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"

constexpr UINT RS_INVALID_ORDER = 0;

constexpr UINT MAX_STRUCTURED_BUFFER_SIZE = 256;
constexpr UINT MAX_INSTANCE_SIZE = MAX_STRUCTURED_BUFFER_SIZE;
constexpr UINT MAX_LIGHT_SIZE = MAX_STRUCTURED_BUFFER_SIZE;
constexpr UINT MAX_PARTICLE_EMITTER_SIZE = MAX_STRUCTURED_BUFFER_SIZE;
constexpr UINT MAX_SHADOW_SIZE = 4;

#pragma clang diagnostic pop

enum class LAYOUT_TYPE {
  NORMAL_COLOR,
  NORMAL_TEX,
  NORMAL_TANGENT_TEX,
  NORMAL_TANGENT_TEX_WEIGHT_BONE
};

enum class DRAWCALL_TYPE {
  OPACITY,
  TRANSPARENCY,
  LIGHT,
  COLOR_GEO,
  MIRROR_SELF,
  MIRRO_INSIDE,
  UI_SPRITE,

  MAX
};

enum class LENS_TYPE { PERSPECTIVE, ORTHOGRAPHIC };

struct CAM_INFO {
  LENS_TYPE Type = LENS_TYPE::PERSPECTIVE;
  dx::XMFLOAT3 Position = {};
  dx::XMFLOAT3 LookAtVector = {};
  dx::XMFLOAT3 UpVector = {};
  dx::XMFLOAT2 PerspFovYRatio = {};
  dx::XMFLOAT2 OrthoWidthHeight = {};
  dx::XMFLOAT2 NearFarZ = {};
};

struct RS_CAM_INFO {
  dx::XMFLOAT4X4 ViewMatrix = {};
  dx::XMFLOAT4X4 InvViewMatrix = {};
  dx::XMFLOAT4X4 ProjMatrix = {};
  dx::XMFLOAT4X4 InvProjMatrix = {};
  dx::XMFLOAT4X4 ViewProjMatrix = {};
  dx::XMFLOAT3 EyePosition = {};
};

enum class LIGHT_TYPE { DIRECT, POINT, SPOT };

struct LIGHT_INFO {
  LIGHT_TYPE Type = LIGHT_TYPE::DIRECT;
  bool ShadowFlag = false;
  // TODO change to physically based radiometry with lumen in future
  // now it's just a factor to scale light strength
  float Intensity = 1.f;
  dx::XMFLOAT3 Albedo = {};
  float FalloffStart = 0.f;
  dx::XMFLOAT3 Direction = {};
  float FalloffEnd = 0.f;
  dx::XMFLOAT3 Position = {};
  float SpotPower = 0.f;
};

struct RS_LIGHT_INFO {
  float Intensity = 1.f;
  dx::XMFLOAT3 Albedo = {};
  float FalloffStart = 0.f;
  dx::XMFLOAT3 Direction = {};
  float FalloffEnd = 0.f;
  dx::XMFLOAT3 Position = {};
  float SpotPower = 0.f;
};

enum class PARTICLE_TEXTURE {
  WHITE_SMOKE,
  WHITE_CIRCLE,

  SIZE
};

struct PARTICLE_EMITTER_INFO {
  float EmitNumPerSecond = 0.f;
  dx::XMFLOAT3 Position = {};
  dx::XMFLOAT3 Velocity = {};
  dx::XMFLOAT3 PosVariance = {};
  float VelVariance = 0.f;
  dx::XMFLOAT3 Acceleration = {};
  float ParticleMass = 0.f;
  float LifeSpan = 0.f;
  float StartSize = 0.f;
  float EndSize = 0.f;
  dx::XMFLOAT4 StartColor = {};
  dx::XMFLOAT4 EndColor = {};
  bool StreakFlag = false;
  PARTICLE_TEXTURE TextureID = PARTICLE_TEXTURE::WHITE_CIRCLE;
};

struct RS_PARTICLE_EMITTER_INFO {
  UINT EmitterIndex = 0;
  float EmitNumPerSecond = 0.f;
  UINT EmitSize = 0;
  float Accumulation = 0.f;
  dx::XMFLOAT3 Position = {};
  float VelVariance = 0.f;
  dx::XMFLOAT3 Velocity = {};
  float ParticleMass = 0.f;
  dx::XMFLOAT3 PosVariance = {};
  float LifeSpan = 0.f;
  dx::XMFLOAT3 Acceleration = {};
  float StartSize = 0.f;
  float EndSize = 0.f;
  UINT TextureID = static_cast<UINT>(PARTICLE_TEXTURE::WHITE_CIRCLE);
  UINT StreakFlag = 0;
  UINT MiscFlag = 0;
  dx::XMFLOAT4 StartColor = {};
  dx::XMFLOAT4 EndColor = {};
};

struct MATERIAL_INFO {
  UINT MajorMaterialID = 0;
  UINT MinorMaterialID = 0;
  float InterpolateFactor = 0.f;
};

struct RS_MATERIAL_INFO {
  UINT MajorMaterialID = 0;
  UINT MinorMaterialID = 0;
  float InterpolateFactor = 0.f;
};

struct RS_MATERIAL_DATA {
  dx::XMFLOAT3 FresnelR0 = {};
  float SubSurface = 0.f;
  float Metallic = 0.f;
  float Specular = 0.f;
  float SpecularTint = 0.f;
  float Roughness = 0.f;
  float Anisotropic = 0.f;
  float Sheen = 0.f;
  float SheenTint = 0.f;
  float Clearcoat = 0.f;
  float ClearcoatGloss = 0.f;
};

enum class TOPOLOGY_TYPE {
  NONE,
  POINTLIST,
  LINELIST,
  LINESTRIP,
  TRIANGLELIST,
  TRIANGLESTRIP
};

struct SUBMESH_INFO {
  TOPOLOGY_TYPE TopologyType = TOPOLOGY_TYPE::NONE;
  std::vector<UINT> *IndeicesPtr = nullptr;
  void *VerteicesPtr = nullptr;
  std::vector<std::string> *TexturesPtr = nullptr;
  MATERIAL_INFO *MaterialPtr = nullptr;
  bool AnimationFlag = false;
};

enum class MESH_TEXTURE_TYPE {
  ALBEDO,
  NORMAL,
  METALLIC,
  ROUGHNESS,
  EMISSIVE,

  SIZE
};

struct RS_SUBMESH_DATA {
  D3D_PRIMITIVE_TOPOLOGY TopologyType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
  ID3D11InputLayout *InputLayout = nullptr;
  ID3D11Buffer *IndexBuffer = nullptr;
  ID3D11Buffer *VertexBuffer = nullptr;
  UINT IndexSize = 0;
  std::array<std::string, static_cast<size_t>(MESH_TEXTURE_TYPE::SIZE)>
      Textures = {""};
  RS_MATERIAL_INFO Material = {};
  bool AnimationFlag = false;
};

struct RS_SUBMESH_DRAWCALL_DATA {
  D3D_PRIMITIVE_TOPOLOGY TopologyType = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
  ID3D11InputLayout *InputLayout = nullptr;
  ID3D11Buffer *IndexBuffer = nullptr;
  ID3D11Buffer *VertexBuffer = nullptr;
  UINT IndexSize = 0;
};

struct RS_INSTANCE_DATA {
  dx::XMFLOAT4X4 WorldMatrix = {};
  RS_MATERIAL_INFO MaterialData = {};
  dx::XMFLOAT4 CustomizedData1 = {};
  dx::XMFLOAT4 CustomizedData2 = {};
};

struct RS_SUBMESH_BONE_DATA {
  std::string BoneName = "";
  dx::XMFLOAT4X4 LocalToBone = {};
  dx::XMFLOAT4X4 BoneTransform = {};
};

struct RS_INSTANCE_DRAWCALL_DATA {
  std::vector<RS_INSTANCE_DATA> *DataArrayPtr = nullptr;
  // std::vector<std::vector<RS_SUBMESH_BONE_DATA>>*
  void *BonesArrayPtr = nullptr;
};

struct RS_MESH_TEXTURE_INFO {
  bool EnabledFlag = false;
  ID3D11ShaderResourceView *Srv = nullptr;
};

struct RS_MISC_INFO {
  dx::XMFLOAT2 RtvSize;
  dx::XMFLOAT2 InvRtvSize;
  float DeltaTime;
};

struct RS_DRAWCALL_DATA {
  RS_SUBMESH_DRAWCALL_DATA MeshData = {};
  RS_INSTANCE_DRAWCALL_DATA InstanceData = {};
  RS_MATERIAL_INFO MaterialData = {};
  std::array<RS_MESH_TEXTURE_INFO, static_cast<size_t>(MESH_TEXTURE_TYPE::SIZE)>
      TextureData = {{}};
  RS_MISC_INFO MiscData = {};
};

struct RSDrawCallsPipe {
  std::vector<RS_DRAWCALL_DATA> Data = {};
};

enum class RS_RESOURCE_TYPE {
  BUFFER,
  TEXTURE1D,
  TEXTURE2D,
  TEXTURE3D,

  SIZE
};

struct RS_RESOURCE_INFO {
  RS_RESOURCE_TYPE Type = RS_RESOURCE_TYPE::BUFFER;

  union RS_RESOURCE_DATA {
    ID3D11Buffer *Buffer = nullptr;
    ID3D11Texture1D *Texture1D;
    ID3D11Texture2D *Texture2D;
    ID3D11Texture3D *Texture3D;
  } Resource;

  ID3D11RenderTargetView *Rtv = nullptr;
  ID3D11DepthStencilView *Dsv = nullptr;
  ID3D11ShaderResourceView *Srv = nullptr;
  ID3D11UnorderedAccessView *Uav = nullptr;
};

#ifdef _RS_DX11
#include "RSExtraMacro.h"
#endif // _RS_DX11
#include "RSVertexType.h"
