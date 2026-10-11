//---------------------------------------------------------------
// File: RSVkObjects.cpp
// Proj: RenderSystem_VK
// Info: 封装Vulkan资源、视图、采样器、着色器与管线状态对象
// Date: 2026.10.11
// Comt: 对象使用与COM相同的引用计数(AddRef/Release)，
//       引用计数归零后等待GPU用完再销毁
//---------------------------------------------------------------

#include "RSVkObjects.h"

#include "RSDevices.h"
#include "RSShaderCompile.h"

#include <vma/vk_mem_alloc.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <unordered_map>

//---------------------------------------------------------------
// base object
//---------------------------------------------------------------

RSVkObject::RSVkObject(RSDevices *Owner) : RefCount(1), Devices(Owner) {}

RSVkObject::~RSVkObject() {}

uint32_t RSVkObject::AddRef() { return ++RefCount; }

uint32_t RSVkObject::Release() {
  uint32_t Count = --RefCount;
  if (!Count) {
    // the gpu may still use this object in frames in flight
    Devices->deferDestroy(this);
  }
  return Count;
}

//---------------------------------------------------------------
// buffer
//---------------------------------------------------------------

RSBuffer::RSBuffer(RSDevices *Owner, const RS_BUFFER_DESC &BufferDesc)
    : RSTrackedResource(Owner), Desc(BufferDesc), Buffer(VK_NULL_HANDLE),
      Allocation(nullptr), DynamicLock(), Shadow({}), ValidSize(0),
      Dirty(false), Current({}), CurrentFrameSerial(0) {}

RSBuffer::~RSBuffer() {
  if (Buffer) {
    vmaDestroyBuffer(Devices->getAllocator(), Buffer, Allocation);
    Buffer = VK_NULL_HANDLE;
    Allocation = nullptr;
  }
}

bool RSBuffer::create() {
  if (!Desc.Size) {
    return false;
  }

  if (isDynamic()) {
    // contents live in the ring buffer of each frame
    Shadow.resize(static_cast<size_t>(Desc.Size), 0);
    ValidSize = Desc.Size;
    Dirty = true;
    return true;
  }

  VkBufferCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  Info.size = Desc.Size;
  Info.usage = Desc.Usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo AllocInfo = {};
  AllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  return vmaCreateBuffer(Devices->getAllocator(), &Info, &AllocInfo, &Buffer,
                         &Allocation, nullptr) == VK_SUCCESS;
}

void *RSBuffer::map() {
  assert(isDynamic() && "only dynamic buffer can be mapped");
  return Shadow.data();
}

void RSBuffer::unmap(VkDeviceSize WrittenSize) {
  std::lock_guard<std::mutex> Lock(DynamicLock);
  Dirty = true;
  ValidSize = (WrittenSize == VK_WHOLE_SIZE || WrittenSize > Desc.Size)
                  ? Desc.Size
                  : WrittenSize;
  if (!ValidSize) {
    ValidSize = Desc.Size;
  }
}

RS_BUFFER_RANGE RSBuffer::getCurrentRange(VkDeviceSize Alignment) {
  if (!isDynamic()) {
    return {Buffer, 0, Desc.Size};
  }

  std::lock_guard<std::mutex> Lock(DynamicLock);
  uint64_t Serial = Devices->getFrameSerial();
  if (Dirty || CurrentFrameSerial != Serial || !Current.Buffer) {
    VkDeviceSize Size = ValidSize;
    // constant buffers in hlsl may be larger than the c++ structure
    // because of array padding, keep the bound range large enough
    if (Desc.Usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) {
      Size = (Size + 255) & ~static_cast<VkDeviceSize>(255);
    }
    auto Alloc = Devices->allocateDynamic(Size, Alignment);
    std::memcpy(Alloc.CpuAddress, Shadow.data(),
                static_cast<size_t>(std::min(ValidSize, Desc.Size)));
    Current = {Alloc.Buffer, Alloc.Offset, Size};
    CurrentFrameSerial = Serial;
    Dirty = false;
  }

  return Current;
}

//---------------------------------------------------------------
// texture
//---------------------------------------------------------------

RSTexture::RSTexture(RSDevices *Owner, const RS_TEXTURE_DESC &TextureDesc)
    : RSTrackedResource(Owner), Desc(TextureDesc), Image(VK_NULL_HANDLE),
      Allocation(nullptr), Aspect(VK_IMAGE_ASPECT_COLOR_BIT) {}

RSTexture::~RSTexture() {
  if (Image) {
    vmaDestroyImage(Devices->getAllocator(), Image, Allocation);
    Image = VK_NULL_HANDLE;
    Allocation = nullptr;
  }
}

bool RSTexture::create() {
  if (Desc.Format == VK_FORMAT_D24_UNORM_S8_UINT ||
      Desc.Format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
    Aspect = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  } else if (isDepthFormat(Desc.Format)) {
    Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  } else {
    Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
  }

  VkImageCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  Info.flags = Desc.CubeCompatible ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
  Info.imageType = Desc.Type;
  Info.format = Desc.Format;
  Info.extent = {Desc.Width, Desc.Height, Desc.Depth};
  Info.mipLevels = Desc.MipLevels;
  Info.arrayLayers = Desc.ArrayLayers;
  Info.samples = VK_SAMPLE_COUNT_1_BIT;
  Info.tiling = VK_IMAGE_TILING_OPTIMAL;
  Info.usage = Desc.Usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
               VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  Info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VmaAllocationCreateInfo AllocInfo = {};
  AllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

  return vmaCreateImage(Devices->getAllocator(), &Info, &AllocInfo, &Image,
                        &Allocation, nullptr) == VK_SUCCESS;
}

VkImageSubresourceRange RSTexture::getFullRange() const {
  VkImageSubresourceRange Range = {};
  Range.aspectMask = Aspect;
  Range.baseMipLevel = 0;
  Range.levelCount = Desc.MipLevels;
  Range.baseArrayLayer = 0;
  Range.layerCount = Desc.ArrayLayers;
  return Range;
}

//---------------------------------------------------------------
// views
//---------------------------------------------------------------

RSResourceView::RSResourceView(RSDevices *Owner)
    : RSVkObject(Owner), Kind(RS_VIEW_KIND::TEXTURE), Texture(nullptr),
      Buffer(nullptr), ImageView(VK_NULL_HANDLE), TexelView(VK_NULL_HANDLE),
      TextureViewDesc({}), BufferViewDesc({}) {}

RSResourceView::~RSResourceView() {
  if (ImageView) {
    vkDestroyImageView(Devices->getDevice(), ImageView, nullptr);
  }
  if (TexelView) {
    vkDestroyBufferView(Devices->getDevice(), TexelView, nullptr);
  }
  // views keep their resource alive like direct3d 11 views do
  if (Texture) {
    Texture->Release();
  }
  if (Buffer) {
    Buffer->Release();
  }
}

bool RSResourceView::createTextureView(RSTexture *Source,
                                       const RS_TEXTURE_VIEW_DESC &ViewDesc,
                                       VkImageUsageFlags ViewUsage) {
  Kind = RS_VIEW_KIND::TEXTURE;
  Texture = Source;
  Texture->AddRef();
  TextureViewDesc = ViewDesc;

  const auto &TexDesc = Texture->getDesc();
  if (TextureViewDesc.Format == VK_FORMAT_UNDEFINED) {
    TextureViewDesc.Format = TexDesc.Format;
  }
  if (TextureViewDesc.MipCount == VK_REMAINING_MIP_LEVELS) {
    TextureViewDesc.MipCount = TexDesc.MipLevels - TextureViewDesc.BaseMip;
  }
  if (TextureViewDesc.LayerCount == VK_REMAINING_ARRAY_LAYERS) {
    TextureViewDesc.LayerCount = TexDesc.ArrayLayers - TextureViewDesc.BaseLayer;
  }

  VkImageViewUsageCreateInfo UsageInfo = {};
  UsageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
  UsageInfo.usage = ViewUsage & TexDesc.Usage;

  VkImageViewCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  Info.pNext = UsageInfo.usage ? &UsageInfo : nullptr;
  Info.image = Texture->getHandle();
  Info.viewType = TextureViewDesc.ViewType;
  Info.format = TextureViewDesc.Format;
  Info.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                     VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};
  Info.subresourceRange = getSubresourceRange();
  // shader views of depth stencil images can only see one aspect
  if (Info.subresourceRange.aspectMask ==
      (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT) &&
      !(ViewUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
    Info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  }

  return vkCreateImageView(Devices->getDevice(), &Info, nullptr, &ImageView) ==
         VK_SUCCESS;
}

bool RSResourceView::createBufferView(RSBuffer *Source,
                                      const RS_BUFFER_VIEW_DESC &ViewDesc) {
  Buffer = Source;
  Buffer->AddRef();
  BufferViewDesc = ViewDesc;

  const auto &BufDesc = Buffer->getDesc();
  if (!BufferViewDesc.ElementSize) {
    BufferViewDesc.ElementSize =
        (BufferViewDesc.Format != VK_FORMAT_UNDEFINED)
            ? getFormatInfo(BufferViewDesc.Format).BlockSize
            : (BufDesc.StructureStride ? BufDesc.StructureStride : 4);
  }
  if (!BufferViewDesc.NumElements) {
    BufferViewDesc.NumElements =
        static_cast<uint32_t>(BufDesc.Size / BufferViewDesc.ElementSize) -
        BufferViewDesc.FirstElement;
  }

  if (BufferViewDesc.Format == VK_FORMAT_UNDEFINED) {
    Kind = RS_VIEW_KIND::STRUCTURED_BUFFER;
    return true;
  }

  Kind = RS_VIEW_KIND::TYPED_BUFFER;
  if (Buffer->isDynamic()) {
    assert(false && "typed view of dynamic buffer isn't supported");
    return false;
  }

  VkBufferViewCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
  Info.buffer = Buffer->getHandle();
  Info.format = BufferViewDesc.Format;
  Info.offset = static_cast<VkDeviceSize>(BufferViewDesc.FirstElement) *
                BufferViewDesc.ElementSize;
  Info.range = static_cast<VkDeviceSize>(BufferViewDesc.NumElements) *
               BufferViewDesc.ElementSize;

  return vkCreateBufferView(Devices->getDevice(), &Info, nullptr,
                            &TexelView) == VK_SUCCESS;
}

RSTrackedResource *RSResourceView::getResource() const {
  if (Texture) {
    return Texture;
  }
  return Buffer;
}

VkImageSubresourceRange RSResourceView::getSubresourceRange() const {
  VkImageSubresourceRange Range = {};
  if (!Texture) {
    return Range;
  }
  Range.aspectMask = Texture->getAspect();
  Range.baseMipLevel = TextureViewDesc.BaseMip;
  Range.levelCount = TextureViewDesc.MipCount;
  Range.baseArrayLayer = TextureViewDesc.BaseLayer;
  Range.layerCount = TextureViewDesc.LayerCount;
  return Range;
}

RS_BUFFER_RANGE RSResourceView::getBufferRange(VkDeviceSize Alignment) const {
  if (!Buffer) {
    return {};
  }
  RS_BUFFER_RANGE Range = Buffer->getCurrentRange(Alignment);
  VkDeviceSize Offset = static_cast<VkDeviceSize>(BufferViewDesc.FirstElement) *
                        BufferViewDesc.ElementSize;
  VkDeviceSize Size = static_cast<VkDeviceSize>(BufferViewDesc.NumElements) *
                      BufferViewDesc.ElementSize;
  if (Buffer->isDynamic()) {
    // only the written part of a dynamic buffer exists in the ring
    Size = std::min(Size, Range.Size > Offset ? Range.Size - Offset : 0);
  }
  Range.Offset += Offset;
  Range.Size = Size ? Size : 4;
  return Range;
}

RSUnorderedAccessView::~RSUnorderedAccessView() {
  if (Counter) {
    Counter->Release();
    Counter = nullptr;
  }
}

bool RSUnorderedAccessView::createCounter() {
  RS_BUFFER_DESC Desc = {};
  Desc.Size = sizeof(uint32_t);
  Desc.Usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  Desc.Memory = RS_MEMORY_USAGE::GPU_ONLY;
  uint32_t Zero = 0;
  Counter = Devices->createBuffer(Desc, &Zero);
  return Counter != nullptr;
}

//---------------------------------------------------------------
// sampler
//---------------------------------------------------------------

RSSampler::~RSSampler() {
  if (Sampler) {
    vkDestroySampler(Devices->getDevice(), Sampler, nullptr);
  }
}

bool RSSampler::create(const VkSamplerCreateInfo &Info) {
  // d3d11 always supports 16x anisotropy, vulkan needs the feature
  VkSamplerCreateInfo Fixed = Info;
  if (Fixed.anisotropyEnable) {
    VkPhysicalDeviceFeatures Supported = {};
    vkGetPhysicalDeviceFeatures(Devices->getPhysicalDevice(), &Supported);
    float MaxAnisotropy =
        Devices->getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
    if (!Supported.samplerAnisotropy) {
      Fixed.anisotropyEnable = VK_FALSE;
      Fixed.maxAnisotropy = 1.f;
    } else if (Fixed.maxAnisotropy > MaxAnisotropy) {
      Fixed.maxAnisotropy = MaxAnisotropy;
    }
  }
  return vkCreateSampler(Devices->getDevice(), &Fixed, nullptr, &Sampler) ==
         VK_SUCCESS;
}

//---------------------------------------------------------------
// shader
//---------------------------------------------------------------

bool RSShader::create(std::vector<uint32_t> &&Code) {
  Spirv = std::move(Code);
  return rs_tool::reflectSpirv(Spirv, Reflection);
}

//---------------------------------------------------------------
// pipeline state
//---------------------------------------------------------------

namespace {

VkShaderStageFlags getBindingStage(uint32_t Binding, bool Compute) {
  if (Compute) {
    return VK_SHADER_STAGE_COMPUTE_BIT;
  }
  switch (Binding / RS_BINDING_STAGE_STRIDE) {
  case 0:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case 1:
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  default:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  }
}

VkShaderModule createModule(VkDevice Device,
                            const std::vector<uint32_t> &Spirv) {
  VkShaderModuleCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  Info.codeSize = Spirv.size() * sizeof(uint32_t);
  Info.pCode = Spirv.data();
  VkShaderModule Module = VK_NULL_HANDLE;
  if (vkCreateShaderModule(Device, &Info, nullptr, &Module) != VK_SUCCESS) {
    return VK_NULL_HANDLE;
  }
  return Module;
}

} // namespace

RSPipelineState::RSPipelineState(RSDevices *Owner)
    : RSVkObject(Owner), BindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS),
      Pipeline(VK_NULL_HANDLE), Layout(VK_NULL_HANDLE),
      SetLayout(VK_NULL_HANDLE), Bindings({}),
      Topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST), VertexStride(0) {}

RSPipelineState::~RSPipelineState() {
  VkDevice Device = Devices->getDevice();
  if (Pipeline) {
    vkDestroyPipeline(Device, Pipeline, nullptr);
  }
  if (Layout) {
    vkDestroyPipelineLayout(Device, Layout, nullptr);
  }
  if (SetLayout) {
    vkDestroyDescriptorSetLayout(Device, SetLayout, nullptr);
  }
}

bool RSPipelineState::createLayout(const std::vector<const RSShader *> &Shaders) {
  bool Compute = (BindPoint == VK_PIPELINE_BIND_POINT_COMPUTE);
  std::vector<VkDescriptorSetLayoutBinding> LayoutBindings = {};
  for (const auto Shader : Shaders) {
    for (const auto &Binding : Shader->getReflection().Bindings) {
      auto Existed = std::find_if(
          Bindings.begin(), Bindings.end(),
          [&](const rs_tool::SPIRV_BINDING_INFO &B) {
            return B.Binding == Binding.Binding;
          });
      if (Existed != Bindings.end()) {
        continue;
      }
      Bindings.push_back(Binding);
      VkDescriptorSetLayoutBinding LayoutBinding = {};
      LayoutBinding.binding = Binding.Binding;
      LayoutBinding.descriptorType = Binding.Type;
      LayoutBinding.descriptorCount = 1;
      LayoutBinding.stageFlags = getBindingStage(Binding.Binding, Compute);
      LayoutBindings.push_back(LayoutBinding);
    }
  }

  VkDescriptorSetLayoutCreateInfo SetInfo = {};
  SetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  SetInfo.bindingCount = static_cast<uint32_t>(LayoutBindings.size());
  SetInfo.pBindings = LayoutBindings.data();
  if (vkCreateDescriptorSetLayout(Devices->getDevice(), &SetInfo, nullptr,
                                  &SetLayout) != VK_SUCCESS) {
    return false;
  }

  VkPipelineLayoutCreateInfo LayoutInfo = {};
  LayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  LayoutInfo.setLayoutCount = 1;
  LayoutInfo.pSetLayouts = &SetLayout;
  return vkCreatePipelineLayout(Devices->getDevice(), &LayoutInfo, nullptr,
                                &Layout) == VK_SUCCESS;
}

bool RSPipelineState::createGraphics(const RS_GRAPHICS_PIPELINE_DESC &Desc) {
  BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  Topology = Desc.Topology;
  VertexStride = Desc.InputLayout ? Desc.InputLayout->getStride() : 0;

  std::vector<const RSShader *> Shaders = {};
  for (auto Shader : {Desc.VertexShader, Desc.GeometryShader, Desc.PixelShader}) {
    if (Shader) {
      Shaders.push_back(Shader);
    }
  }
  if (!Desc.VertexShader || !createLayout(Shaders)) {
    return false;
  }

  VkDevice Device = Devices->getDevice();
  std::vector<VkShaderModule> Modules = {};
  std::vector<VkPipelineShaderStageCreateInfo> Stages = {};
  std::unordered_map<std::string, uint32_t> PrevOutputs = {};
  bool Result = true;
  for (const auto Shader : Shaders) {
    // link stages by semantic like direct3d does
    auto Spirv = Shader->getSpirv();
    if (!PrevOutputs.empty()) {
      rs_tool::remapLocations(Spirv, true, PrevOutputs);
    }
    PrevOutputs.clear();
    for (const auto &Output : Shader->getReflection().Outputs) {
      PrevOutputs[Output.Semantic] = Output.Location;
    }
    rs_tool::stripHlslFunctionality1(Spirv);

    VkShaderModule Module = createModule(Device, Spirv);
    if (!Module) {
      Result = false;
      break;
    }
    Modules.push_back(Module);

    VkPipelineShaderStageCreateInfo Stage = {};
    Stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    Stage.stage = Shader->getStage();
    Stage.module = Module;
    Stage.pName = Shader->getReflection().EntryPoint.c_str();
    Stages.push_back(Stage);
  }

  std::vector<VkVertexInputAttributeDescription> Attributes = {};
  VkVertexInputBindingDescription VertexBinding = {};
  if (Result && Desc.InputLayout) {
    VertexBinding.binding = 0;
    VertexBinding.stride = Desc.InputLayout->getStride();
    VertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    for (const auto &Input : Desc.VertexShader->getReflection().Inputs) {
      std::string Name = "";
      uint32_t Index = 0;
      rs_tool::splitSemantic(Input.Semantic, Name, Index);
      const auto &Elements = Desc.InputLayout->getElements();
      auto Found = std::find_if(Elements.begin(), Elements.end(),
                                [&](const RS_INPUT_ELEMENT &E) {
                                  return E.SemanticName == Name &&
                                         E.SemanticIndex == Index;
                                });
      if (Found == Elements.end()) {
        std::fprintf(stderr,
                     "[[[ERROR]]] : vertex shader input %s doesn't exist in "
                     "input layout %s\n",
                     Input.Semantic.c_str(),
                     Desc.InputLayout->getName().c_str());
        Result = false;
        break;
      }
      VkVertexInputAttributeDescription Attribute = {};
      Attribute.location = Input.Location;
      Attribute.binding = 0;
      Attribute.format = Found->Format;
      Attribute.offset = Found->Offset;
      Attributes.push_back(Attribute);
    }
  }

  if (Result) {
    VkPipelineVertexInputStateCreateInfo VertexInput = {};
    VertexInput.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (Desc.InputLayout) {
      VertexInput.vertexBindingDescriptionCount = 1;
      VertexInput.pVertexBindingDescriptions = &VertexBinding;
      VertexInput.vertexAttributeDescriptionCount =
          static_cast<uint32_t>(Attributes.size());
      VertexInput.pVertexAttributeDescriptions = Attributes.data();
    }

    VkPipelineInputAssemblyStateCreateInfo InputAssembly = {};
    InputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssembly.topology = Desc.Topology;

    VkPipelineViewportStateCreateInfo Viewport = {};
    Viewport.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    Viewport.viewportCount = 1;
    Viewport.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo Raster = {};
    Raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // disabling depth clip in d3d11 clamps the depth, both depend on
    // optional features in vulkan and fall back to the default states
    VkPhysicalDeviceFeatures Supported = {};
    vkGetPhysicalDeviceFeatures(Devices->getPhysicalDevice(), &Supported);
    Raster.depthClampEnable =
        (!Desc.DepthClipEnable && Supported.depthClamp) ? VK_TRUE : VK_FALSE;
    Raster.polygonMode =
        Supported.fillModeNonSolid ? Desc.FillMode : VK_POLYGON_MODE_FILL;
    Raster.cullMode = Desc.CullMode;
    Raster.frontFace = Desc.FrontFace;
    Raster.depthBiasEnable = Desc.DepthBiasEnable ? VK_TRUE : VK_FALSE;
    Raster.depthBiasConstantFactor = Desc.DepthBiasConstant;
    Raster.depthBiasClamp = Desc.DepthBiasClamp;
    Raster.depthBiasSlopeFactor = Desc.DepthBiasSlope;
    Raster.lineWidth = 1.f;

    VkPipelineMultisampleStateCreateInfo Multisample = {};
    Multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    Multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    bool HasDepth = Desc.DepthFormat != VK_FORMAT_UNDEFINED;
    VkPipelineDepthStencilStateCreateInfo DepthStencil = {};
    DepthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    DepthStencil.depthTestEnable =
        (HasDepth && Desc.DepthTestEnable) ? VK_TRUE : VK_FALSE;
    DepthStencil.depthWriteEnable =
        (HasDepth && Desc.DepthTestEnable && Desc.DepthWriteEnable) ? VK_TRUE
                                                                    : VK_FALSE;
    DepthStencil.depthCompareOp = Desc.DepthCompareOp;

    std::array<VkPipelineColorBlendAttachmentState, RS_MAX_RENDER_TARGETS>
        BlendAttachments = {};
    for (uint32_t I = 0; I < Desc.ColorTargetCount; I++) {
      const auto &Blend = Desc.Blend[I];
      auto &Attachment = BlendAttachments[I];
      Attachment.blendEnable = Blend.Enable ? VK_TRUE : VK_FALSE;
      Attachment.srcColorBlendFactor = Blend.SrcColor;
      Attachment.dstColorBlendFactor = Blend.DstColor;
      Attachment.colorBlendOp = Blend.ColorOp;
      Attachment.srcAlphaBlendFactor = Blend.SrcAlpha;
      Attachment.dstAlphaBlendFactor = Blend.DstAlpha;
      Attachment.alphaBlendOp = Blend.AlphaOp;
      Attachment.colorWriteMask = Blend.WriteMask;
    }
    VkPipelineColorBlendStateCreateInfo ColorBlend = {};
    ColorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlend.attachmentCount = Desc.ColorTargetCount;
    ColorBlend.pAttachments = BlendAttachments.data();

    const VkDynamicState DynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY};
    VkPipelineDynamicStateCreateInfo Dynamic = {};
    Dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    Dynamic.dynamicStateCount = 3;
    Dynamic.pDynamicStates = DynamicStates;

    VkPipelineRenderingCreateInfo Rendering = {};
    Rendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    Rendering.colorAttachmentCount = Desc.ColorTargetCount;
    Rendering.pColorAttachmentFormats = Desc.ColorFormats.data();
    Rendering.depthAttachmentFormat = Desc.DepthFormat;
    if (Desc.DepthFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
        Desc.DepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) {
      Rendering.stencilAttachmentFormat = Desc.DepthFormat;
    }

    VkGraphicsPipelineCreateInfo Info = {};
    Info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    Info.pNext = &Rendering;
    Info.stageCount = static_cast<uint32_t>(Stages.size());
    Info.pStages = Stages.data();
    Info.pVertexInputState = &VertexInput;
    Info.pInputAssemblyState = &InputAssembly;
    Info.pViewportState = &Viewport;
    Info.pRasterizationState = &Raster;
    Info.pMultisampleState = &Multisample;
    Info.pDepthStencilState = &DepthStencil;
    Info.pColorBlendState = &ColorBlend;
    Info.pDynamicState = &Dynamic;
    Info.layout = Layout;

    Result = vkCreateGraphicsPipelines(Device, Devices->getPipelineCache(), 1,
                                       &Info, nullptr,
                                       &Pipeline) == VK_SUCCESS;
  }

  for (auto Module : Modules) {
    vkDestroyShaderModule(Device, Module, nullptr);
  }

  return Result;
}

bool RSPipelineState::createCompute(RSShader *ComputeShader) {
  BindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
  if (!ComputeShader ||
      ComputeShader->getStage() != VK_SHADER_STAGE_COMPUTE_BIT ||
      !createLayout({ComputeShader})) {
    return false;
  }

  VkDevice Device = Devices->getDevice();
  auto Spirv = ComputeShader->getSpirv();
  rs_tool::stripHlslFunctionality1(Spirv);
  VkShaderModule Module = createModule(Device, Spirv);
  if (!Module) {
    return false;
  }

  VkComputePipelineCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  Info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  Info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  Info.stage.module = Module;
  Info.stage.pName = ComputeShader->getReflection().EntryPoint.c_str();
  Info.layout = Layout;

  bool Result = vkCreateComputePipelines(Device, Devices->getPipelineCache(), 1,
                                         &Info, nullptr,
                                         &Pipeline) == VK_SUCCESS;
  vkDestroyShaderModule(Device, Module, nullptr);

  return Result;
}

//---------------------------------------------------------------
// format helpers
//---------------------------------------------------------------

RS_FORMAT_INFO getFormatInfo(VkFormat Format) {
  switch (Format) {
  case VK_FORMAT_R8_UNORM:
  case VK_FORMAT_R8_UINT:
    return {1, 1, 1};
  case VK_FORMAT_R8G8_UNORM:
  case VK_FORMAT_R16_SFLOAT:
  case VK_FORMAT_R16_UNORM:
  case VK_FORMAT_R16_UINT:
  case VK_FORMAT_D16_UNORM:
    return {2, 1, 1};
  case VK_FORMAT_R8G8B8A8_UNORM:
  case VK_FORMAT_R8G8B8A8_SRGB:
  case VK_FORMAT_R8G8B8A8_UINT:
  case VK_FORMAT_B8G8R8A8_UNORM:
  case VK_FORMAT_B8G8R8A8_SRGB:
  case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
  case VK_FORMAT_R16G16_SFLOAT:
  case VK_FORMAT_R16G16_UNORM:
  case VK_FORMAT_R32_SFLOAT:
  case VK_FORMAT_R32_UINT:
  case VK_FORMAT_R32_SINT:
  case VK_FORMAT_D32_SFLOAT:
  case VK_FORMAT_D24_UNORM_S8_UINT:
    return {4, 1, 1};
  case VK_FORMAT_R16G16B16A16_SFLOAT:
  case VK_FORMAT_R16G16B16A16_UNORM:
  case VK_FORMAT_R32G32_SFLOAT:
  case VK_FORMAT_R32G32_UINT:
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
    return {8, 1, 1};
  case VK_FORMAT_R32G32B32_SFLOAT:
    return {12, 1, 1};
  case VK_FORMAT_R32G32B32A32_SFLOAT:
  case VK_FORMAT_R32G32B32A32_UINT:
  case VK_FORMAT_R32G32B32A32_SINT:
    return {16, 1, 1};
  case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
  case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
  case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
  case VK_FORMAT_BC4_UNORM_BLOCK:
  case VK_FORMAT_BC4_SNORM_BLOCK:
    return {8, 4, 4};
  case VK_FORMAT_BC2_UNORM_BLOCK:
  case VK_FORMAT_BC2_SRGB_BLOCK:
  case VK_FORMAT_BC3_UNORM_BLOCK:
  case VK_FORMAT_BC3_SRGB_BLOCK:
  case VK_FORMAT_BC5_UNORM_BLOCK:
  case VK_FORMAT_BC5_SNORM_BLOCK:
  case VK_FORMAT_BC6H_UFLOAT_BLOCK:
  case VK_FORMAT_BC6H_SFLOAT_BLOCK:
  case VK_FORMAT_BC7_UNORM_BLOCK:
  case VK_FORMAT_BC7_SRGB_BLOCK:
    return {16, 4, 4};
  default:
    assert(false && "unknown format size");
    return {4, 1, 1};
  }
}

bool isDepthFormat(VkFormat Format) {
  switch (Format) {
  case VK_FORMAT_D16_UNORM:
  case VK_FORMAT_D32_SFLOAT:
  case VK_FORMAT_D24_UNORM_S8_UINT:
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
    return true;
  default:
    return false;
  }
}
