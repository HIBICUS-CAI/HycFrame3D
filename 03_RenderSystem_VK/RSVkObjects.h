//---------------------------------------------------------------
// File: RSVkObjects.h
// Proj: RenderSystem_VK
// Info: 封装Vulkan资源、视图、采样器、着色器与管线状态对象
// Date: 2026.10.11
// Comt: 对象使用与COM相同的引用计数(AddRef/Release)，
//       引用计数归零后等待GPU用完再销毁
//---------------------------------------------------------------

#pragma once

#include "RSSpirvReflect.h"

#include <vulkan/vulkan.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

typedef struct VmaAllocation_T *VmaAllocation;

class RSDevices;

//---------------------------------------------------------------
// base object
//---------------------------------------------------------------

class RSVkObject {
private:
  std::atomic<uint32_t> RefCount;

protected:
  RSDevices *Devices;

public:
  RSVkObject(RSDevices *Owner);
  RSVkObject(const RSVkObject &) = delete;
  RSVkObject &operator=(const RSVkObject &) = delete;
  virtual ~RSVkObject();

  // keep the com style names so the ownership code of passes stays as is
  uint32_t AddRef();
  uint32_t Release();

  RSDevices *getDevices() const { return Devices; }

  // prints objects which are still alive like ReportLiveDeviceObjects,
  // only tracked in debug builds, returns the count of them
  static size_t reportLiveObjects();
};

//---------------------------------------------------------------
// resources with tracked access state
//---------------------------------------------------------------

struct RS_ACCESS {
  VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkPipelineStageFlags2 Stages = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 Access = VK_ACCESS_2_NONE;
  bool Write = false;
};

struct RS_TRACKED_STATE {
  VkImageLayout Layout = VK_IMAGE_LAYOUT_UNDEFINED;
  // last write which may not be visible to later accesses yet
  VkPipelineStageFlags2 WriteStages = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 WriteAccess = VK_ACCESS_2_NONE;
  // reads after the last write which are already synchronized
  VkPipelineStageFlags2 ReadStages = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 ReadAccess = VK_ACCESS_2_NONE;
};

class RSTrackedResource : public RSVkObject {
public:
  // state at the end of the last submitted command buffer, only touched
  // by the submitting thread or before the resource gets published
  RS_TRACKED_STATE GlobalState;

  RSTrackedResource(RSDevices *Owner) : RSVkObject(Owner), GlobalState({}) {}

  virtual bool isImage() const = 0;
};

//---------------------------------------------------------------
// buffer
//---------------------------------------------------------------

enum class RS_MEMORY_USAGE {
  // device local, initial data is uploaded through a staging buffer
  GPU_ONLY,
  // written by cpu through map/unmap with discard semantic like
  // D3D11_USAGE_DYNAMIC, contents are copied to a per frame ring buffer
  DYNAMIC
};

struct RS_BUFFER_DESC {
  VkDeviceSize Size = 0;
  VkBufferUsageFlags Usage = 0;
  RS_MEMORY_USAGE Memory = RS_MEMORY_USAGE::GPU_ONLY;
  uint32_t StructureStride = 0;
};

struct RS_BUFFER_RANGE {
  VkBuffer Buffer = VK_NULL_HANDLE;
  VkDeviceSize Offset = 0;
  VkDeviceSize Size = 0;
};

class RSBuffer : public RSTrackedResource {
private:
  RS_BUFFER_DESC Desc;
  VkBuffer Buffer;
  VmaAllocation Allocation;

  // dynamic buffer data
  std::mutex DynamicLock;
  std::vector<uint8_t> Shadow;
  VkDeviceSize ValidSize;
  bool Dirty;
  RS_BUFFER_RANGE Current;
  uint64_t CurrentFrameSerial;

public:
  RSBuffer(RSDevices *Owner, const RS_BUFFER_DESC &BufferDesc);
  virtual ~RSBuffer();

  bool create();

  virtual bool isImage() const override { return false; }

  const RS_BUFFER_DESC &getDesc() const { return Desc; }
  VkBuffer getHandle() const { return Buffer; }
  bool isDynamic() const { return Desc.Memory == RS_MEMORY_USAGE::DYNAMIC; }

  // dynamic buffer only, every map discards the previous contents
  void *map();
  void unmap(VkDeviceSize WrittenSize = VK_WHOLE_SIZE);

  // gpu location of the current contents, a dynamic buffer gets copied
  // into the ring buffer of the frame being recorded when needed
  RS_BUFFER_RANGE getCurrentRange(VkDeviceSize Alignment);
};

//---------------------------------------------------------------
// texture
//---------------------------------------------------------------

struct RS_TEXTURE_DESC {
  VkImageType Type = VK_IMAGE_TYPE_2D;
  VkFormat Format = VK_FORMAT_UNDEFINED;
  uint32_t Width = 1;
  uint32_t Height = 1;
  uint32_t Depth = 1;
  uint32_t MipLevels = 1;
  uint32_t ArrayLayers = 1;
  VkImageUsageFlags Usage = 0;
  bool CubeCompatible = false;
};

// subresources are ordered as [layer][mip] like D3D11_SUBRESOURCE_DATA
struct RS_SUBRESOURCE_DATA {
  const void *Data = nullptr;
  uint32_t RowPitch = 0;
  uint32_t SlicePitch = 0;
};

class RSTexture : public RSTrackedResource {
private:
  RS_TEXTURE_DESC Desc;
  VkImage Image;
  VmaAllocation Allocation;
  VkImageAspectFlags Aspect;

public:
  RSTexture(RSDevices *Owner, const RS_TEXTURE_DESC &TextureDesc);
  virtual ~RSTexture();

  bool create();

  virtual bool isImage() const override { return true; }

  const RS_TEXTURE_DESC &getDesc() const { return Desc; }
  VkImage getHandle() const { return Image; }
  VkImageAspectFlags getAspect() const { return Aspect; }
  VkImageSubresourceRange getFullRange() const;
  // storage capable images stay in general layout for every shader access
  bool isStorage() const { return Desc.Usage & VK_IMAGE_USAGE_STORAGE_BIT; }
};

//---------------------------------------------------------------
// views
//---------------------------------------------------------------

enum class RS_VIEW_KIND { TEXTURE, STRUCTURED_BUFFER, TYPED_BUFFER };

struct RS_TEXTURE_VIEW_DESC {
  VkImageViewType ViewType = VK_IMAGE_VIEW_TYPE_2D;
  // VK_FORMAT_UNDEFINED means the texture format
  VkFormat Format = VK_FORMAT_UNDEFINED;
  uint32_t BaseMip = 0;
  uint32_t MipCount = VK_REMAINING_MIP_LEVELS;
  uint32_t BaseLayer = 0;
  uint32_t LayerCount = VK_REMAINING_ARRAY_LAYERS;
};

struct RS_BUFFER_VIEW_DESC {
  // VK_FORMAT_UNDEFINED means a structured buffer view, otherwise a typed
  // (texel) buffer view like Buffer<T> / RWBuffer<T>
  VkFormat Format = VK_FORMAT_UNDEFINED;
  uint32_t FirstElement = 0;
  uint32_t NumElements = 0;
  uint32_t ElementSize = 0;
  // uav only, creates a hidden counter like D3D11_BUFFER_UAV_FLAG_APPEND
  bool WithCounter = false;
};

class RSResourceView : public RSVkObject {
protected:
  RS_VIEW_KIND Kind;
  RSTexture *Texture;
  RSBuffer *Buffer;
  VkImageView ImageView;
  VkBufferView TexelView;
  RS_TEXTURE_VIEW_DESC TextureViewDesc;
  RS_BUFFER_VIEW_DESC BufferViewDesc;

public:
  RSResourceView(RSDevices *Owner);
  virtual ~RSResourceView();

  bool createTextureView(RSTexture *Source,
                         const RS_TEXTURE_VIEW_DESC &ViewDesc,
                         VkImageUsageFlags ViewUsage);
  bool createBufferView(RSBuffer *Source, const RS_BUFFER_VIEW_DESC &ViewDesc);

  RS_VIEW_KIND getKind() const { return Kind; }
  RSTexture *getTexture() const { return Texture; }
  RSBuffer *getBuffer() const { return Buffer; }
  RSTrackedResource *getResource() const;
  VkImageView getImageView() const { return ImageView; }
  VkBufferView getTexelView() const { return TexelView; }
  const RS_TEXTURE_VIEW_DESC &getTextureViewDesc() const {
    return TextureViewDesc;
  }
  const RS_BUFFER_VIEW_DESC &getBufferViewDesc() const {
    return BufferViewDesc;
  }
  VkImageSubresourceRange getSubresourceRange() const;
  RS_BUFFER_RANGE getBufferRange(VkDeviceSize Alignment) const;
};

class RSShaderResourceView : public RSResourceView {
public:
  RSShaderResourceView(RSDevices *Owner) : RSResourceView(Owner) {}
};

class RSUnorderedAccessView : public RSResourceView {
private:
  RSBuffer *Counter;

public:
  RSUnorderedAccessView(RSDevices *Owner)
      : RSResourceView(Owner), Counter(nullptr) {}
  virtual ~RSUnorderedAccessView();

  bool createCounter();
  RSBuffer *getCounter() const { return Counter; }
};

class RSRenderTargetView : public RSResourceView {
public:
  RSRenderTargetView(RSDevices *Owner) : RSResourceView(Owner) {}
};

class RSDepthStencilView : public RSResourceView {
public:
  RSDepthStencilView(RSDevices *Owner) : RSResourceView(Owner) {}
};

//---------------------------------------------------------------
// sampler
//---------------------------------------------------------------

class RSSampler : public RSVkObject {
private:
  VkSampler Sampler;

public:
  RSSampler(RSDevices *Owner) : RSVkObject(Owner), Sampler(VK_NULL_HANDLE) {}
  virtual ~RSSampler();

  bool create(const VkSamplerCreateInfo &Info);
  VkSampler getHandle() const { return Sampler; }
};

//---------------------------------------------------------------
// shader and input layout
//---------------------------------------------------------------

class RSShader : public RSVkObject {
private:
  // spir-v still has semantic decorations, they are used for linking
  // stages and stripped when a pipeline gets created
  std::vector<uint32_t> Spirv;
  rs_tool::SPIRV_REFLECTION Reflection;

public:
  RSShader(RSDevices *Owner) : RSVkObject(Owner) {}
  virtual ~RSShader() {}

  bool create(std::vector<uint32_t> &&Code);

  const std::vector<uint32_t> &getSpirv() const { return Spirv; }
  const rs_tool::SPIRV_REFLECTION &getReflection() const { return Reflection; }
  VkShaderStageFlagBits getStage() const { return Reflection.Stage; }
};

struct RS_INPUT_ELEMENT {
  std::string SemanticName = "";
  uint32_t SemanticIndex = 0;
  VkFormat Format = VK_FORMAT_UNDEFINED;
  uint32_t Offset = 0;
};

// vertex buffer layout matched with vertex shader inputs by semantic
class RSInputLayout {
private:
  std::string Name;
  uint32_t Stride;
  std::vector<RS_INPUT_ELEMENT> Elements;

public:
  RSInputLayout(const std::string &LayoutName,
                uint32_t VertexStride,
                const std::vector<RS_INPUT_ELEMENT> &InputElements)
      : Name(LayoutName), Stride(VertexStride), Elements(InputElements) {}

  const std::string &getName() const { return Name; }
  uint32_t getStride() const { return Stride; }
  const std::vector<RS_INPUT_ELEMENT> &getElements() const { return Elements; }
};

//---------------------------------------------------------------
// pipeline state
//---------------------------------------------------------------

constexpr uint32_t RS_MAX_RENDER_TARGETS = 8;

struct RS_BLEND_STATE {
  bool Enable = false;
  VkBlendFactor SrcColor = VK_BLEND_FACTOR_ONE;
  VkBlendFactor DstColor = VK_BLEND_FACTOR_ZERO;
  VkBlendOp ColorOp = VK_BLEND_OP_ADD;
  VkBlendFactor SrcAlpha = VK_BLEND_FACTOR_ONE;
  VkBlendFactor DstAlpha = VK_BLEND_FACTOR_ZERO;
  VkBlendOp AlphaOp = VK_BLEND_OP_ADD;
  VkColorComponentFlags WriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
};

// defaults are the direct3d 11 default states
struct RS_GRAPHICS_PIPELINE_DESC {
  RSShader *VertexShader = nullptr;
  RSShader *GeometryShader = nullptr;
  RSShader *PixelShader = nullptr;
  const RSInputLayout *InputLayout = nullptr;
  VkPrimitiveTopology Topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPolygonMode FillMode = VK_POLYGON_MODE_FILL;
  VkCullModeFlags CullMode = VK_CULL_MODE_BACK_BIT;
  // viewports are flipped to keep direct3d conventions, so the winding
  // keeps the direct3d meaning (clockwise is front by default)
  VkFrontFace FrontFace = VK_FRONT_FACE_CLOCKWISE;
  bool DepthClipEnable = true;
  bool DepthBiasEnable = false;
  float DepthBiasConstant = 0.f;
  float DepthBiasClamp = 0.f;
  float DepthBiasSlope = 0.f;

  bool DepthTestEnable = true;
  bool DepthWriteEnable = true;
  VkCompareOp DepthCompareOp = VK_COMPARE_OP_LESS;

  uint32_t ColorTargetCount = 0;
  std::array<VkFormat, RS_MAX_RENDER_TARGETS> ColorFormats = {};
  std::array<RS_BLEND_STATE, RS_MAX_RENDER_TARGETS> Blend = {};
  VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
};

class RSPipelineState : public RSVkObject {
private:
  VkPipelineBindPoint BindPoint;
  VkPipeline Pipeline;
  VkPipelineLayout Layout;
  VkDescriptorSetLayout SetLayout;
  std::vector<rs_tool::SPIRV_BINDING_INFO> Bindings;
  VkPrimitiveTopology Topology;
  uint32_t VertexStride;

public:
  RSPipelineState(RSDevices *Owner);
  virtual ~RSPipelineState();

  bool createGraphics(const RS_GRAPHICS_PIPELINE_DESC &Desc);
  bool createCompute(RSShader *ComputeShader);

  VkPipelineBindPoint getBindPoint() const { return BindPoint; }
  VkPipeline getHandle() const { return Pipeline; }
  VkPipelineLayout getLayout() const { return Layout; }
  VkDescriptorSetLayout getSetLayout() const { return SetLayout; }
  const std::vector<rs_tool::SPIRV_BINDING_INFO> &getBindings() const {
    return Bindings;
  }
  VkPrimitiveTopology getTopology() const { return Topology; }
  uint32_t getVertexStride() const { return VertexStride; }

private:
  bool createLayout(const std::vector<const RSShader *> &Shaders);
};

//---------------------------------------------------------------
// format helpers
//---------------------------------------------------------------

struct RS_FORMAT_INFO {
  uint32_t BlockSize = 0;
  uint32_t BlockWidth = 1;
  uint32_t BlockHeight = 1;
};

RS_FORMAT_INFO getFormatInfo(VkFormat Format);
bool isDepthFormat(VkFormat Format);
