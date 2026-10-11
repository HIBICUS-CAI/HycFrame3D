//---------------------------------------------------------------
// File: RSCommandContext.cpp
// Proj: RenderSystem_VK
// Info: 录制Vulkan指令并自动处理资源屏障与描述符集
// Date: 2026.10.11
// Comt: 资源绑定方式与D3D11的DeviceContext保持一致，
//       每个topic拥有一个context，可在不同线程并行录制
//---------------------------------------------------------------

#include "RSCommandContext.h"

#include "RSDevices.h"

#include <algorithm>
#include <cassert>
#include <cstdio>

static_assert(RS_COMMAND_CONTEXT_FRAMES == RS_FRAME_IN_FLIGHT,
              "context must have resources for every frame in flight");

namespace {

constexpr VkPipelineStageFlags2 ALL_SHADER_STAGES =
    VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
    VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT |
    VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
    VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;

VkImageLayout getShaderReadLayout(const RSTexture *Texture) {
  return Texture->isStorage() ? VK_IMAGE_LAYOUT_GENERAL
                              : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

VkPipelineStageFlags2 getStageFlag(RS_SHADER_STAGE Stage) {
  switch (Stage) {
  case RS_SHADER_STAGE::VERTEX:
    return VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
  case RS_SHADER_STAGE::GEOMETRY:
    return VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
  case RS_SHADER_STAGE::PIXEL:
    return VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
  default:
    return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
  }
}

enum class SLOT_CLASS { CBV, SRV, UAV, SAMPLER, COUNTER };

struct SLOT_LOCATION {
  RS_SHADER_STAGE Stage = RS_SHADER_STAGE::VERTEX;
  SLOT_CLASS Class = SLOT_CLASS::CBV;
  uint32_t Slot = 0;
};

SLOT_LOCATION decodeBinding(uint32_t Binding, bool Compute) {
  SLOT_LOCATION Location = {};
  if (Compute) {
    Location.Stage = RS_SHADER_STAGE::COMPUTE;
  } else {
    switch (Binding / RS_BINDING_STAGE_STRIDE) {
    case 0:
      Location.Stage = RS_SHADER_STAGE::VERTEX;
      break;
    case 1:
      Location.Stage = RS_SHADER_STAGE::GEOMETRY;
      break;
    default:
      Location.Stage = RS_SHADER_STAGE::PIXEL;
      break;
    }
  }
  uint32_t Local = Binding % RS_BINDING_STAGE_STRIDE;
  if (Local < RS_BINDING_SRV_OFFSET) {
    Location.Class = SLOT_CLASS::CBV;
    Location.Slot = Local - RS_BINDING_CBV_OFFSET;
  } else if (Local < RS_BINDING_UAV_OFFSET) {
    Location.Class = SLOT_CLASS::SRV;
    Location.Slot = Local - RS_BINDING_SRV_OFFSET;
  } else if (Local < RS_BINDING_SAMPLER_OFFSET) {
    Location.Class = SLOT_CLASS::UAV;
    Location.Slot = Local - RS_BINDING_UAV_OFFSET;
  } else if (Local < RS_BINDING_UAV_OFFSET + RS_BINDING_COUNTER_OFFSET) {
    Location.Class = SLOT_CLASS::SAMPLER;
    Location.Slot = Local - RS_BINDING_SAMPLER_OFFSET;
  } else {
    Location.Class = SLOT_CLASS::COUNTER;
    Location.Slot = Local - RS_BINDING_UAV_OFFSET - RS_BINDING_COUNTER_OFFSET;
  }
  return Location;
}

VkExtent2D getMipExtent(const RSTexture *Texture, uint32_t Mip) {
  const auto &Desc = Texture->getDesc();
  return {std::max(1u, Desc.Width >> Mip), std::max(1u, Desc.Height >> Mip)};
}

} // namespace

//---------------------------------------------------------------
// common accesses
//---------------------------------------------------------------

namespace rs_access {

RS_ACCESS colorAttachment(const RSTexture *Texture) {
  RS_ACCESS Access = {};
  Access.Layout = Texture->isStorage() ? VK_IMAGE_LAYOUT_GENERAL
                                       : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  Access.Stages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
  Access.Access = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT |
                  VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
  Access.Write = true;
  return Access;
}

RS_ACCESS depthAttachment() {
  RS_ACCESS Access = {};
  Access.Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  Access.Stages = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT |
                  VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
  Access.Access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                  VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  Access.Write = true;
  return Access;
}

RS_ACCESS transferSrc(const RSTrackedResource *Resource) {
  RS_ACCESS Access = {};
  if (Resource->isImage()) {
    Access.Layout = static_cast<const RSTexture *>(Resource)->isStorage()
                        ? VK_IMAGE_LAYOUT_GENERAL
                        : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
  }
  Access.Stages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  Access.Access = VK_ACCESS_2_TRANSFER_READ_BIT;
  Access.Write = false;
  return Access;
}

RS_ACCESS transferDst(const RSTrackedResource *Resource) {
  RS_ACCESS Access = {};
  if (Resource->isImage()) {
    Access.Layout = static_cast<const RSTexture *>(Resource)->isStorage()
                        ? VK_IMAGE_LAYOUT_GENERAL
                        : VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  }
  Access.Stages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  Access.Access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  Access.Write = true;
  return Access;
}

RS_ACCESS present() {
  RS_ACCESS Access = {};
  Access.Layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  Access.Stages = VK_PIPELINE_STAGE_2_NONE;
  Access.Access = VK_ACCESS_2_NONE;
  Access.Write = false;
  return Access;
}

} // namespace rs_access

//---------------------------------------------------------------
// life cycle
//---------------------------------------------------------------

RSCommandContext::RSCommandContext()
    : Devices(nullptr), Frames({}), FrameIndex(0), Command(VK_NULL_HANDLE),
      Recording(false), Slots({}), CounterResets({}), GraphicsPipeline(nullptr),
      ComputePipeline(nullptr), BoundPipeline(VK_NULL_HANDLE),
      Topology(VK_PRIMITIVE_TOPOLOGY_MAX_ENUM), VertexBuffer(nullptr),
      VertexOffset(0), IndexBuffer(nullptr), IndexOffset(0), Viewport({}),
      ViewportSet(false), Rendering({}), LocalStates({}),
      EntryRequirements({}), ImageBarriers({}), BufferBarriers({}),
      DynamicAlignment(256) {}

RSCommandContext::~RSCommandContext() {}

bool RSCommandContext::startUp(RSDevices *DevicesPtr) {
  if (!DevicesPtr) {
    return false;
  }
  Devices = DevicesPtr;
  VkDevice Device = Devices->getDevice();

  const auto &Limits = Devices->getPhysicalDeviceProperties().limits;
  DynamicAlignment = std::max<VkDeviceSize>(
      {Limits.minUniformBufferOffsetAlignment,
       Limits.minStorageBufferOffsetAlignment, 16});

  for (auto &Frame : Frames) {
    VkCommandPoolCreateInfo PoolInfo = {};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    PoolInfo.queueFamilyIndex = Devices->getQueueFamily();
    if (vkCreateCommandPool(Device, &PoolInfo, nullptr, &Frame.Pool) !=
        VK_SUCCESS) {
      return false;
    }

    VkCommandBuffer Buffers[2] = {};
    VkCommandBufferAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = Frame.Pool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 2;
    if (vkAllocateCommandBuffers(Device, &AllocInfo, Buffers) != VK_SUCCESS) {
      return false;
    }
    Frame.Command = Buffers[0];
    Frame.Glue = Buffers[1];
  }

  return true;
}

void RSCommandContext::cleanAndStop() {
  if (!Devices) {
    return;
  }
  VkDevice Device = Devices->getDevice();
  for (auto &Frame : Frames) {
    for (auto Pool : Frame.DescriptorPools) {
      vkDestroyDescriptorPool(Device, Pool, nullptr);
    }
    Frame.DescriptorPools.clear();
    if (Frame.Pool) {
      vkDestroyCommandPool(Device, Frame.Pool, nullptr);
      Frame.Pool = VK_NULL_HANDLE;
    }
  }
  Devices = nullptr;
}

void RSCommandContext::beginRecording() {
  assert(!Recording);
  FrameIndex = Devices->getFrameIndex();
  auto &Frame = Frames[FrameIndex];
  VkDevice Device = Devices->getDevice();

  vkResetCommandPool(Device, Frame.Pool, 0);
  for (auto Pool : Frame.DescriptorPools) {
    vkResetDescriptorPool(Device, Pool, 0);
  }
  Frame.DescriptorPoolIndex = 0;

  Command = Frame.Command;
  VkCommandBufferBeginInfo BeginInfo = {};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(Command, &BeginInfo);

  // every topic starts from the default state like a deferred context
  // after FinishCommandList(FALSE)
  Slots = {};
  CounterResets.clear();
  GraphicsPipeline = nullptr;
  ComputePipeline = nullptr;
  BoundPipeline = VK_NULL_HANDLE;
  Topology = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
  VertexBuffer = nullptr;
  VertexOffset = 0;
  IndexBuffer = nullptr;
  IndexOffset = 0;
  ViewportSet = false;
  Rendering = {};
  LocalStates.clear();
  EntryRequirements.clear();
  ImageBarriers.clear();
  BufferBarriers.clear();

  Recording = true;
}

void RSCommandContext::endRecording() {
  assert(Recording);
  flushPendingClears();
  endRendering();
  flushBarriers();
  vkEndCommandBuffer(Command);
  Recording = false;
}

VkCommandBuffer RSCommandContext::recordGlue() {
  auto &Frame = Frames[FrameIndex];
  bool Begun = false;

  for (const auto &Requirement : EntryRequirements) {
    auto Resource = Requirement.Resource;
    const auto &Global = Resource->GlobalState;
    const auto &Access = Requirement.Access;
    bool LayoutChange = Resource->isImage() && Global.Layout != Access.Layout;
    bool HasWrite = Global.WriteStages || Global.WriteAccess;
    bool NeedWar = Access.Write && Global.ReadStages;
    if (!LayoutChange && !HasWrite && !NeedWar) {
      continue;
    }
    if (!Begun) {
      VkCommandBufferBeginInfo BeginInfo = {};
      BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(Frame.Glue, &BeginInfo);
      Begun = true;
    }
    RSDevices::recordTransition(Frame.Glue, Resource, Global, Access);
  }

  for (const auto &Local : LocalStates) {
    Local.first->GlobalState = Local.second;
  }

  if (!Begun) {
    return VK_NULL_HANDLE;
  }
  vkEndCommandBuffer(Frame.Glue);
  return Frame.Glue;
}

//---------------------------------------------------------------
// barriers
//---------------------------------------------------------------

void RSCommandContext::requireAccess(RSTrackedResource *Resource,
                                     const RS_ACCESS &Access) {
  if (!Resource) {
    return;
  }

  auto Found = LocalStates.find(Resource);
  if (Found == LocalStates.end()) {
    // first access in this context, the glue command buffer recorded at
    // submission makes it visible to every later access of this context
    EntryRequirements.push_back({Resource, Access});
    RS_TRACKED_STATE State = {};
    State.Layout = Access.Layout;
    if (Access.Write) {
      State.WriteStages = Access.Stages;
      State.WriteAccess = Access.Access;
    } else {
      State.ReadStages = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
      State.ReadAccess = VK_ACCESS_2_MEMORY_READ_BIT;
    }
    LocalStates.insert({Resource, State});
    return;
  }

  auto &Current = Found->second;
  bool LayoutChange = Resource->isImage() && Current.Layout != Access.Layout;
  VkPipelineStageFlags2 SrcStages = VK_PIPELINE_STAGE_2_NONE;
  VkAccessFlags2 SrcAccess = VK_ACCESS_2_NONE;
  bool NeedBarrier = false;

  if (Access.Write || LayoutChange) {
    SrcStages = Current.WriteStages | Current.ReadStages;
    SrcAccess = Current.WriteAccess;
    NeedBarrier = LayoutChange || SrcStages;
  } else {
    bool HasWrite = Current.WriteStages || Current.WriteAccess;
    bool Visible = (Current.ReadStages & Access.Stages) == Access.Stages &&
                   (Current.ReadAccess & Access.Access) == Access.Access;
    if (HasWrite && !Visible) {
      SrcStages = Current.WriteStages;
      SrcAccess = Current.WriteAccess;
      NeedBarrier = true;
    }
  }

  if (NeedBarrier) {
    if (Resource->isImage()) {
      auto Texture = static_cast<RSTexture *>(Resource);
      VkImageMemoryBarrier2 Barrier = {};
      Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
      Barrier.srcStageMask = SrcStages;
      Barrier.srcAccessMask = SrcAccess;
      Barrier.dstStageMask = Access.Stages;
      Barrier.dstAccessMask = Access.Access;
      Barrier.oldLayout = Current.Layout;
      Barrier.newLayout = Access.Layout;
      Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      Barrier.image = Texture->getHandle();
      Barrier.subresourceRange = Texture->getFullRange();
      ImageBarriers.push_back(Barrier);
    } else {
      auto Buffer = static_cast<RSBuffer *>(Resource);
      VkBufferMemoryBarrier2 Barrier = {};
      Barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
      Barrier.srcStageMask = SrcStages;
      Barrier.srcAccessMask = SrcAccess;
      Barrier.dstStageMask = Access.Stages;
      Barrier.dstAccessMask = Access.Access;
      Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      Barrier.buffer = Buffer->getHandle();
      Barrier.offset = 0;
      Barrier.size = VK_WHOLE_SIZE;
      BufferBarriers.push_back(Barrier);
    }
  }

  if (Access.Write) {
    Current.Layout = Access.Layout;
    Current.WriteStages = Access.Stages;
    Current.WriteAccess = Access.Access;
    Current.ReadStages = VK_PIPELINE_STAGE_2_NONE;
    Current.ReadAccess = VK_ACCESS_2_NONE;
  } else if (LayoutChange) {
    // the layout transition counts as a write done in Access.Stages
    Current.Layout = Access.Layout;
    Current.WriteStages = Access.Stages;
    Current.WriteAccess = VK_ACCESS_2_NONE;
    Current.ReadStages = Access.Stages;
    Current.ReadAccess = Access.Access;
  } else {
    Current.ReadStages |= Access.Stages;
    Current.ReadAccess |= Access.Access;
  }
}

void RSCommandContext::flushBarriers() {
  if (ImageBarriers.empty() && BufferBarriers.empty()) {
    return;
  }
  assert(!Rendering.Begun && "barriers can't be recorded while rendering");

  VkDependencyInfo Dependency = {};
  Dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  Dependency.imageMemoryBarrierCount = static_cast<uint32_t>(ImageBarriers.size());
  Dependency.pImageMemoryBarriers = ImageBarriers.data();
  Dependency.bufferMemoryBarrierCount =
      static_cast<uint32_t>(BufferBarriers.size());
  Dependency.pBufferMemoryBarriers = BufferBarriers.data();
  vkCmdPipelineBarrier2(Command, &Dependency);

  ImageBarriers.clear();
  BufferBarriers.clear();
}

RS_ACCESS RSCommandContext::getShaderAccess(RSTrackedResource *Resource,
                                            VkDescriptorType Type,
                                            bool Write,
                                            VkPipelineStageFlags2 Stage) const {
  RS_ACCESS Access = {};
  Access.Stages = Stage;
  Access.Write = Write;
  if (Resource->isImage()) {
    auto Texture = static_cast<RSTexture *>(Resource);
    Access.Layout = Write ? VK_IMAGE_LAYOUT_GENERAL : getShaderReadLayout(Texture);
  }
  switch (Type) {
  case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    Access.Access = VK_ACCESS_2_UNIFORM_READ_BIT;
    break;
  case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
  case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    Access.Access = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    break;
  default:
    Access.Access = Write ? (VK_ACCESS_2_SHADER_STORAGE_READ_BIT |
                             VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT)
                          : VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    break;
  }
  return Access;
}

bool RSCommandContext::isRenderTarget(RSTrackedResource *Resource) const {
  if (!Rendering.Active || !Resource) {
    return false;
  }
  for (uint32_t I = 0; I < Rendering.RtvCount; I++) {
    if (Rendering.Rtvs[I] && Rendering.Rtvs[I]->getTexture() == Resource) {
      return true;
    }
  }
  return Rendering.Dsv && Rendering.Dsv->getTexture() == Resource;
}

//---------------------------------------------------------------
// output merger
//---------------------------------------------------------------

void RSCommandContext::setRenderTargets(UINT NumViews,
                                        RSRenderTargetView *const *Rtvs,
                                        RSDepthStencilView *Dsv) {
  flushPendingClears();
  endRendering();

  Rendering = {};
  for (UINT I = 0; I < NumViews && I < RS_MAX_RENDER_TARGETS; I++) {
    Rendering.Rtvs[I] = Rtvs ? Rtvs[I] : nullptr;
    Rendering.LoadOps[I] = VK_ATTACHMENT_LOAD_OP_LOAD;
    if (Rendering.Rtvs[I]) {
      Rendering.RtvCount = I + 1;
    }
  }
  Rendering.Dsv = Dsv;
  Rendering.DepthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  Rendering.Active = Rendering.RtvCount || Rendering.Dsv;
}

void RSCommandContext::clearRenderTargetView(RSRenderTargetView *Rtv,
                                             const float Color[4]) {
  if (!Rtv) {
    return;
  }
  VkClearColorValue ClearColor = {};
  for (int I = 0; I < 4; I++) {
    ClearColor.float32[I] = Color[I];
  }

  if (Rendering.Active && !Rendering.Begun) {
    for (uint32_t I = 0; I < Rendering.RtvCount; I++) {
      if (Rendering.Rtvs[I] == Rtv) {
        Rendering.LoadOps[I] = VK_ATTACHMENT_LOAD_OP_CLEAR;
        Rendering.ClearColors[I] = ClearColor;
        return;
      }
    }
  }

  // the view isn't a pending render target, clear by transfer
  endRendering();
  auto Texture = Rtv->getTexture();
  requireAccess(Texture, rs_access::transferDst(Texture));
  flushBarriers();
  auto Range = Rtv->getSubresourceRange();
  vkCmdClearColorImage(Command, Texture->getHandle(),
                       LocalStates[Texture].Layout, &ClearColor, 1, &Range);
}

void RSCommandContext::clearDepthStencilView(RSDepthStencilView *Dsv,
                                             float Depth) {
  if (!Dsv) {
    return;
  }

  if (Rendering.Active && !Rendering.Begun && Rendering.Dsv == Dsv) {
    Rendering.DepthLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    Rendering.ClearDepth = Depth;
    return;
  }

  endRendering();
  auto Texture = Dsv->getTexture();
  requireAccess(Texture, rs_access::transferDst(Texture));
  flushBarriers();
  VkClearDepthStencilValue Value = {Depth, 0};
  auto Range = Dsv->getSubresourceRange();
  vkCmdClearDepthStencilImage(Command, Texture->getHandle(),
                              LocalStates[Texture].Layout, &Value, 1, &Range);
}

void RSCommandContext::setViewport(const VkViewport &ViewPort) {
  Viewport = ViewPort;
  ViewportSet = true;
}

//---------------------------------------------------------------
// pipeline and input assembler
//---------------------------------------------------------------

void RSCommandContext::setPipelineState(RSPipelineState *Pipeline) {
  if (!Pipeline) {
    return;
  }
  if (Pipeline->getBindPoint() == VK_PIPELINE_BIND_POINT_COMPUTE) {
    ComputePipeline = Pipeline;
  } else {
    GraphicsPipeline = Pipeline;
  }
}

void RSCommandContext::setPrimitiveTopology(VkPrimitiveTopology TopologyType) {
  Topology = TopologyType;
}

void RSCommandContext::setVertexBuffer(RSBuffer *Buffer, VkDeviceSize Offset) {
  VertexBuffer = Buffer;
  VertexOffset = Offset;
}

void RSCommandContext::setIndexBuffer(RSBuffer *Buffer, VkDeviceSize Offset) {
  IndexBuffer = Buffer;
  IndexOffset = Offset;
}

//---------------------------------------------------------------
// resource binding
//---------------------------------------------------------------

void RSCommandContext::setConstantBuffers(RS_SHADER_STAGE Stage,
                                          UINT StartSlot,
                                          UINT NumBuffers,
                                          RSBuffer *const *Buffers) {
  auto &Table = Slots[static_cast<size_t>(Stage)];
  for (UINT I = 0; I < NumBuffers && StartSlot + I < RS_MAX_CBV_SLOT; I++) {
    Table.Cbv[StartSlot + I] = Buffers ? Buffers[I] : nullptr;
  }
}

void RSCommandContext::setShaderResources(RS_SHADER_STAGE Stage,
                                          UINT StartSlot,
                                          UINT NumViews,
                                          RSShaderResourceView *const *Views) {
  auto &Table = Slots[static_cast<size_t>(Stage)];
  for (UINT I = 0; I < NumViews && StartSlot + I < RS_MAX_SRV_SLOT; I++) {
    Table.Srv[StartSlot + I] = Views ? Views[I] : nullptr;
  }
}

void RSCommandContext::setUnorderedAccessViews(
    RS_SHADER_STAGE Stage,
    UINT StartSlot,
    UINT NumViews,
    RSUnorderedAccessView *const *Views,
    const UINT *InitialCounts) {
  auto &Table = Slots[static_cast<size_t>(Stage)];
  for (UINT I = 0; I < NumViews && StartSlot + I < RS_MAX_UAV_SLOT; I++) {
    auto View = Views ? Views[I] : nullptr;
    Table.Uav[StartSlot + I] = View;
    if (View && View->getCounter() && InitialCounts &&
        InitialCounts[I] != static_cast<UINT>(-1)) {
      CounterResets.push_back({View, InitialCounts[I]});
    }
  }
}

void RSCommandContext::setSamplers(RS_SHADER_STAGE Stage,
                                   UINT StartSlot,
                                   UINT NumSamplers,
                                   RSSampler *const *Samplers) {
  auto &Table = Slots[static_cast<size_t>(Stage)];
  for (UINT I = 0; I < NumSamplers && StartSlot + I < RS_MAX_SAMPLER_SLOT;
       I++) {
    Table.Sampler[StartSlot + I] = Samplers ? Samplers[I] : nullptr;
  }
}

//---------------------------------------------------------------
// descriptors
//---------------------------------------------------------------

void RSCommandContext::prepareResources(RSPipelineState *Pipeline,
                                        bool Compute) {
  for (const auto &Binding : Pipeline->getBindings()) {
    auto Location = decodeBinding(Binding.Binding, Compute);
    auto &Table = Slots[static_cast<size_t>(Location.Stage)];
    auto Stage = getStageFlag(Location.Stage);
    RSTrackedResource *Resource = nullptr;
    bool Write = false;

    switch (Location.Class) {
    case SLOT_CLASS::CBV: {
      auto Buffer = Table.Cbv[Location.Slot];
      if (Buffer && !Buffer->isDynamic()) {
        Resource = Buffer;
      }
      break;
    }
    case SLOT_CLASS::SRV: {
      auto View = Table.Srv[Location.Slot];
      if (View && !(View->getBuffer() && View->getBuffer()->isDynamic())) {
        Resource = View->getResource();
      }
      break;
    }
    case SLOT_CLASS::UAV: {
      auto View = Table.Uav[Location.Slot];
      Resource = View ? View->getResource() : nullptr;
      Write = true;
      break;
    }
    case SLOT_CLASS::COUNTER: {
      auto View = Table.Uav[Location.Slot];
      Resource = View ? View->getCounter() : nullptr;
      Write = true;
      break;
    }
    default:
      break;
    }

    // direct3d 11 unbinds a shader resource which is also bound as an
    // output, the shader then reads zero. keep the same result.
    if (!Resource || (!Compute && isRenderTarget(Resource))) {
      continue;
    }
    requireAccess(Resource, getShaderAccess(Resource, Binding.Type, Write,
                                            Stage));
  }

  if (!Compute) {
    if (VertexBuffer && !VertexBuffer->isDynamic()) {
      RS_ACCESS Access = {};
      Access.Stages = VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT;
      Access.Access = VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
      requireAccess(VertexBuffer, Access);
    }
    if (IndexBuffer && !IndexBuffer->isDynamic()) {
      RS_ACCESS Access = {};
      Access.Stages = VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
      Access.Access = VK_ACCESS_2_INDEX_READ_BIT;
      requireAccess(IndexBuffer, Access);
    }
  }
}

VkDescriptorSet
RSCommandContext::allocateDescriptorSet(VkDescriptorSetLayout Layout) {
  auto &Frame = Frames[FrameIndex];
  VkDevice Device = Devices->getDevice();

  while (true) {
    if (Frame.DescriptorPoolIndex >= Frame.DescriptorPools.size()) {
      const VkDescriptorPoolSize Sizes[] = {
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4096},
          {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4096},
          {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024},
          {VK_DESCRIPTOR_TYPE_SAMPLER, 2048},
          {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 512},
          {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 512}};
      VkDescriptorPoolCreateInfo PoolInfo = {};
      PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      PoolInfo.maxSets = 1024;
      PoolInfo.poolSizeCount = static_cast<uint32_t>(std::size(Sizes));
      PoolInfo.pPoolSizes = Sizes;
      VkDescriptorPool Pool = VK_NULL_HANDLE;
      if (vkCreateDescriptorPool(Device, &PoolInfo, nullptr, &Pool) !=
          VK_SUCCESS) {
        return VK_NULL_HANDLE;
      }
      Frame.DescriptorPools.push_back(Pool);
    }

    VkDescriptorSetAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = Frame.DescriptorPools[Frame.DescriptorPoolIndex];
    AllocInfo.descriptorSetCount = 1;
    AllocInfo.pSetLayouts = &Layout;
    VkDescriptorSet Set = VK_NULL_HANDLE;
    VkResult Result = vkAllocateDescriptorSets(Device, &AllocInfo, &Set);
    if (Result == VK_SUCCESS) {
      return Set;
    }
    if (Result != VK_ERROR_OUT_OF_POOL_MEMORY &&
        Result != VK_ERROR_FRAGMENTED_POOL) {
      return VK_NULL_HANDLE;
    }
    ++Frame.DescriptorPoolIndex;
  }
}

void RSCommandContext::bindDescriptors(RSPipelineState *Pipeline,
                                       bool Compute) {
  const auto &Bindings = Pipeline->getBindings();
  if (Bindings.empty()) {
    return;
  }

  VkDescriptorSet Set = allocateDescriptorSet(Pipeline->getSetLayout());
  if (!Set) {
    assert(false && "failed to allocate descriptor set");
    return;
  }

  std::vector<VkWriteDescriptorSet> Writes(Bindings.size());
  std::vector<VkDescriptorBufferInfo> BufferInfos(Bindings.size());
  std::vector<VkDescriptorImageInfo> ImageInfos(Bindings.size());
  std::vector<VkBufferView> TexelViews(Bindings.size());

  for (size_t I = 0; I < Bindings.size(); I++) {
    const auto &Binding = Bindings[I];
    auto Location = decodeBinding(Binding.Binding, Compute);
    auto &Table = Slots[static_cast<size_t>(Location.Stage)];

    auto &Write = Writes[I];
    Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Write.dstSet = Set;
    Write.dstBinding = Binding.Binding;
    Write.descriptorCount = 1;
    Write.descriptorType = Binding.Type;

    RSResourceView *View = nullptr;
    switch (Location.Class) {
    case SLOT_CLASS::SRV:
      View = Table.Srv[Location.Slot];
      break;
    case SLOT_CLASS::UAV:
      View = Table.Uav[Location.Slot];
      break;
    default:
      break;
    }
    if (View && !Compute && isRenderTarget(View->getResource())) {
      View = nullptr;
    }

    switch (Binding.Type) {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
      RSBuffer *Buffer = (Location.Class == SLOT_CLASS::CBV)
                             ? Table.Cbv[Location.Slot]
                             : nullptr;
      RS_BUFFER_RANGE Range = Buffer ? Buffer->getCurrentRange(DynamicAlignment)
                                     : Devices->getDummyBuffer()->getCurrentRange(
                                           DynamicAlignment);
      BufferInfos[I] = {Range.Buffer, Range.Offset, Range.Size};
      Write.pBufferInfo = &BufferInfos[I];
      break;
    }
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
      RS_BUFFER_RANGE Range = {};
      if (Location.Class == SLOT_CLASS::COUNTER) {
        auto Uav = Table.Uav[Location.Slot];
        if (Uav && Uav->getCounter()) {
          Range = Uav->getCounter()->getCurrentRange(DynamicAlignment);
        }
      } else if (View && View->getKind() == RS_VIEW_KIND::STRUCTURED_BUFFER) {
        Range = View->getBufferRange(DynamicAlignment);
#ifdef _DEBUG
        // c++ structures and shader structures must agree on the stride
        uint32_t ElementSize = View->getBufferViewDesc().ElementSize;
        if (Binding.ElementStride && ElementSize != Binding.ElementStride) {
          std::fprintf(stderr,
                       "[[[WARNING]]] : stride of %s is %u in shader but %u "
                       "in buffer view\n",
                       Binding.Name.c_str(), Binding.ElementStride,
                       ElementSize);
        }
#endif // _DEBUG
      }
      if (!Range.Buffer) {
        Range = Devices->getDummyBuffer()->getCurrentRange(DynamicAlignment);
      }
      BufferInfos[I] = {Range.Buffer, Range.Offset, Range.Size};
      Write.pBufferInfo = &BufferInfos[I];
      break;
    }
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
      bool Storage = Binding.Type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
      if (!View || View->getKind() != RS_VIEW_KIND::TEXTURE) {
        View = Devices->getDummyView(Binding.Type, Binding.Dim,
                                     Binding.Arrayed, Binding.Scalar);
      }
      ImageInfos[I].imageView = View->getImageView();
      ImageInfos[I].imageLayout =
          Storage ? VK_IMAGE_LAYOUT_GENERAL
                  : getShaderReadLayout(View->getTexture());
      Write.pImageInfo = &ImageInfos[I];
      break;
    }
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
      if (!View || View->getKind() != RS_VIEW_KIND::TYPED_BUFFER) {
        View = Devices->getDummyView(Binding.Type, Binding.Dim,
                                     Binding.Arrayed, Binding.Scalar);
      }
      TexelViews[I] = View->getTexelView();
      Write.pTexelBufferView = &TexelViews[I];
      break;
    }
    case VK_DESCRIPTOR_TYPE_SAMPLER: {
      RSSampler *Sampler = (Location.Class == SLOT_CLASS::SAMPLER)
                               ? Table.Sampler[Location.Slot]
                               : nullptr;
      if (!Sampler) {
        Sampler = Devices->getDefaultSampler();
      }
      ImageInfos[I].sampler = Sampler->getHandle();
      Write.pImageInfo = &ImageInfos[I];
      break;
    }
    default:
      assert(false && "unsupported descriptor type");
      break;
    }
  }

  vkUpdateDescriptorSets(Devices->getDevice(),
                         static_cast<uint32_t>(Writes.size()), Writes.data(), 0,
                         nullptr);
  vkCmdBindDescriptorSets(Command, Pipeline->getBindPoint(),
                          Pipeline->getLayout(), 0, 1, &Set, 0, nullptr);
}

void RSCommandContext::applyCounterResets() {
  if (CounterResets.empty()) {
    return;
  }
  endRendering();
  for (const auto &Reset : CounterResets) {
    requireAccess(Reset.first->getCounter(),
                  rs_access::transferDst(Reset.first->getCounter()));
  }
  flushBarriers();
  for (const auto &Reset : CounterResets) {
    vkCmdFillBuffer(Command, Reset.first->getCounter()->getHandle(), 0,
                    sizeof(uint32_t), Reset.second);
  }
  CounterResets.clear();
}

//---------------------------------------------------------------
// rendering
//---------------------------------------------------------------

void RSCommandContext::beginRendering() {
  std::array<VkRenderingAttachmentInfo, RS_MAX_RENDER_TARGETS> Colors = {};
  VkRenderingAttachmentInfo Depth = {};
  VkExtent2D Extent = {UINT32_MAX, UINT32_MAX};

  for (uint32_t I = 0; I < Rendering.RtvCount; I++) {
    auto &Color = Colors[I];
    Color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    auto Rtv = Rendering.Rtvs[I];
    if (!Rtv) {
      continue;
    }
    auto Texture = Rtv->getTexture();
    Color.imageView = Rtv->getImageView();
    Color.imageLayout = LocalStates[Texture].Layout;
    Color.loadOp = Rendering.LoadOps[I];
    Color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    Color.clearValue.color = Rendering.ClearColors[I];
    auto MipExtent = getMipExtent(Texture, Rtv->getTextureViewDesc().BaseMip);
    Extent.width = std::min(Extent.width, MipExtent.width);
    Extent.height = std::min(Extent.height, MipExtent.height);
    Rendering.LoadOps[I] = VK_ATTACHMENT_LOAD_OP_LOAD;
  }

  if (Rendering.Dsv) {
    auto Texture = Rendering.Dsv->getTexture();
    Depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    Depth.imageView = Rendering.Dsv->getImageView();
    Depth.imageLayout = LocalStates[Texture].Layout;
    Depth.loadOp = Rendering.DepthLoadOp;
    Depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    Depth.clearValue.depthStencil = {Rendering.ClearDepth, 0};
    auto MipExtent =
        getMipExtent(Texture, Rendering.Dsv->getTextureViewDesc().BaseMip);
    Extent.width = std::min(Extent.width, MipExtent.width);
    Extent.height = std::min(Extent.height, MipExtent.height);
    Rendering.DepthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  }

  VkRenderingInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  Info.renderArea = {{0, 0}, Extent};
  Info.layerCount = 1;
  Info.colorAttachmentCount = Rendering.RtvCount;
  Info.pColorAttachments = Colors.data();
  Info.pDepthAttachment = Rendering.Dsv ? &Depth : nullptr;
  if (Rendering.Dsv && (Rendering.Dsv->getTexture()->getAspect() &
                        VK_IMAGE_ASPECT_STENCIL_BIT)) {
    Info.pStencilAttachment = &Depth;
  }
  vkCmdBeginRendering(Command, &Info);
  Rendering.Begun = true;

  VkRect2D Scissor = {{0, 0}, Extent};
  vkCmdSetScissor(Command, 0, 1, &Scissor);
  VkViewport Flipped = ViewportSet
                           ? Viewport
                           : VkViewport{0.f, 0.f, static_cast<float>(Extent.width),
                                        static_cast<float>(Extent.height), 0.f,
                                        1.f};
  // negative height keeps direct3d clip space and window coordinates
  Flipped.y += Flipped.height;
  Flipped.height = -Flipped.height;
  vkCmdSetViewport(Command, 0, 1, &Flipped);
}

void RSCommandContext::endRendering() {
  if (Rendering.Begun) {
    vkCmdEndRendering(Command);
    Rendering.Begun = false;
  }
}

void RSCommandContext::prepareRendering() {
  if (!Rendering.Begun) {
    for (uint32_t I = 0; I < Rendering.RtvCount; I++) {
      if (Rendering.Rtvs[I]) {
        auto Texture = Rendering.Rtvs[I]->getTexture();
        requireAccess(Texture, rs_access::colorAttachment(Texture));
      }
    }
    if (Rendering.Dsv) {
      requireAccess(Rendering.Dsv->getTexture(), rs_access::depthAttachment());
    }
  }
  flushBarriers();
  if (!Rendering.Begun) {
    beginRendering();
  }
}

void RSCommandContext::flushPendingClears() {
  if (!Rendering.Active || Rendering.Begun) {
    return;
  }
  bool HasClear = Rendering.DepthLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR;
  for (uint32_t I = 0; I < Rendering.RtvCount; I++) {
    HasClear = HasClear || Rendering.LoadOps[I] == VK_ATTACHMENT_LOAD_OP_CLEAR;
  }
  if (HasClear) {
    prepareRendering();
    endRendering();
  }
}

//---------------------------------------------------------------
// commands
//---------------------------------------------------------------

void RSCommandContext::draw(UINT VertexCount, UINT StartVertexLocation) {
  if (!GraphicsPipeline || !Rendering.Active) {
    assert(false && "draw without pipeline or render target");
    return;
  }

  prepareResources(GraphicsPipeline, false);
  if (Rendering.Begun && (!ImageBarriers.empty() || !BufferBarriers.empty())) {
    endRendering();
  }
  prepareRendering();

  if (BoundPipeline != GraphicsPipeline->getHandle()) {
    vkCmdBindPipeline(Command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      GraphicsPipeline->getHandle());
    BoundPipeline = GraphicsPipeline->getHandle();
  }
  vkCmdSetPrimitiveTopology(Command,
                            (Topology != VK_PRIMITIVE_TOPOLOGY_MAX_ENUM)
                                ? Topology
                                : GraphicsPipeline->getTopology());
  bindDescriptors(GraphicsPipeline, false);

  if (VertexBuffer && GraphicsPipeline->getVertexStride()) {
    auto Range = VertexBuffer->getCurrentRange(DynamicAlignment);
    VkDeviceSize Offset = Range.Offset + VertexOffset;
    vkCmdBindVertexBuffers(Command, 0, 1, &Range.Buffer, &Offset);
  }

  vkCmdDraw(Command, VertexCount, 1, StartVertexLocation, 0);
}

void RSCommandContext::drawIndexedInstanced(UINT IndexCountPerInstance,
                                            UINT InstanceCount,
                                            UINT StartIndexLocation,
                                            INT BaseVertexLocation,
                                            UINT StartInstanceLocation) {
  if (!GraphicsPipeline || !Rendering.Active || !IndexBuffer) {
    assert(false && "draw without pipeline, render target or index buffer");
    return;
  }

  prepareResources(GraphicsPipeline, false);
  if (Rendering.Begun && (!ImageBarriers.empty() || !BufferBarriers.empty())) {
    endRendering();
  }
  prepareRendering();

  if (BoundPipeline != GraphicsPipeline->getHandle()) {
    vkCmdBindPipeline(Command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      GraphicsPipeline->getHandle());
    BoundPipeline = GraphicsPipeline->getHandle();
  }
  vkCmdSetPrimitiveTopology(Command,
                            (Topology != VK_PRIMITIVE_TOPOLOGY_MAX_ENUM)
                                ? Topology
                                : GraphicsPipeline->getTopology());
  bindDescriptors(GraphicsPipeline, false);

  if (VertexBuffer && GraphicsPipeline->getVertexStride()) {
    auto Range = VertexBuffer->getCurrentRange(DynamicAlignment);
    VkDeviceSize Offset = Range.Offset + VertexOffset;
    vkCmdBindVertexBuffers(Command, 0, 1, &Range.Buffer, &Offset);
  }
  auto IndexRange = IndexBuffer->getCurrentRange(DynamicAlignment);
  vkCmdBindIndexBuffer(Command, IndexRange.Buffer,
                       IndexRange.Offset + IndexOffset, VK_INDEX_TYPE_UINT32);

  vkCmdDrawIndexed(Command, IndexCountPerInstance, InstanceCount,
                   StartIndexLocation, BaseVertexLocation,
                   StartInstanceLocation);
}

void RSCommandContext::dispatch(UINT GroupCountX,
                                UINT GroupCountY,
                                UINT GroupCountZ) {
  if (!ComputePipeline) {
    assert(false && "dispatch without pipeline");
    return;
  }

  endRendering();
  applyCounterResets();
  prepareResources(ComputePipeline, true);
  flushBarriers();

  if (BoundPipeline != ComputePipeline->getHandle()) {
    vkCmdBindPipeline(Command, VK_PIPELINE_BIND_POINT_COMPUTE,
                      ComputePipeline->getHandle());
    BoundPipeline = ComputePipeline->getHandle();
  }
  bindDescriptors(ComputePipeline, true);

  vkCmdDispatch(Command, GroupCountX, GroupCountY, GroupCountZ);
}

void RSCommandContext::copyTexture(RSTexture *Dst, RSTexture *Src) {
  if (!Dst || !Src) {
    return;
  }
  endRendering();
  requireAccess(Src, rs_access::transferSrc(Src));
  requireAccess(Dst, rs_access::transferDst(Dst));
  flushBarriers();

  const auto &SrcDesc = Src->getDesc();
  const auto &DstDesc = Dst->getDesc();
  uint32_t Mips = std::min(SrcDesc.MipLevels, DstDesc.MipLevels);
  uint32_t Layers = std::min(SrcDesc.ArrayLayers, DstDesc.ArrayLayers);
  std::vector<VkImageCopy> Regions = {};
  for (uint32_t Mip = 0; Mip < Mips; Mip++) {
    VkImageCopy Region = {};
    Region.srcSubresource = {Src->getAspect(), Mip, 0, Layers};
    Region.dstSubresource = {Dst->getAspect(), Mip, 0, Layers};
    auto Extent = getMipExtent(Src, Mip);
    Region.extent = {Extent.width, Extent.height,
                     std::max(1u, SrcDesc.Depth >> Mip)};
    Regions.push_back(Region);
  }
  vkCmdCopyImage(Command, Src->getHandle(), LocalStates[Src].Layout,
                 Dst->getHandle(), LocalStates[Dst].Layout,
                 static_cast<uint32_t>(Regions.size()), Regions.data());
}

void RSCommandContext::copyBuffer(RSBuffer *Dst,
                                  VkDeviceSize DstOffset,
                                  RSBuffer *Src,
                                  VkDeviceSize SrcOffset,
                                  VkDeviceSize Size) {
  if (!Dst || !Src || Dst->isDynamic()) {
    return;
  }
  endRendering();
  if (!Src->isDynamic()) {
    requireAccess(Src, rs_access::transferSrc(Src));
  }
  requireAccess(Dst, rs_access::transferDst(Dst));
  flushBarriers();

  auto SrcRange = Src->getCurrentRange(DynamicAlignment);
  VkBufferCopy Region = {SrcRange.Offset + SrcOffset, DstOffset, Size};
  vkCmdCopyBuffer(Command, SrcRange.Buffer, Dst->getHandle(), 1, &Region);
}

void RSCommandContext::copyStructureCount(RSBuffer *DstBuffer,
                                          UINT DstAlignedByteOffset,
                                          RSUnorderedAccessView *SrcView) {
  if (!SrcView || !SrcView->getCounter()) {
    return;
  }
  applyCounterResets();
  copyBuffer(DstBuffer, DstAlignedByteOffset, SrcView->getCounter(), 0,
             sizeof(uint32_t));
}

void RSCommandContext::generateMips(RSShaderResourceView *Srv) {
  if (!Srv || !Srv->getTexture()) {
    return;
  }
  auto Texture = Srv->getTexture();
  const auto &Desc = Texture->getDesc();
  if (Desc.MipLevels < 2) {
    return;
  }

  endRendering();
  requireAccess(Texture, rs_access::transferDst(Texture));
  flushBarriers();

  VkImageLayout DstLayout = LocalStates[Texture].Layout;
  VkImageLayout SrcLayout = Texture->isStorage()
                                ? VK_IMAGE_LAYOUT_GENERAL
                                : VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

  VkImageMemoryBarrier2 Barrier = {};
  Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  Barrier.image = Texture->getHandle();
  Barrier.subresourceRange = Texture->getFullRange();
  VkDependencyInfo Dependency = {};
  Dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  Dependency.imageMemoryBarrierCount = 1;
  Dependency.pImageMemoryBarriers = &Barrier;

  for (uint32_t Mip = 1; Mip < Desc.MipLevels; Mip++) {
    // previous level becomes the blit source
    Barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    Barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    Barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
    Barrier.oldLayout = DstLayout;
    Barrier.newLayout = SrcLayout;
    Barrier.subresourceRange.baseMipLevel = Mip - 1;
    Barrier.subresourceRange.levelCount = 1;
    vkCmdPipelineBarrier2(Command, &Dependency);

    auto SrcExtent = getMipExtent(Texture, Mip - 1);
    auto DstExtent = getMipExtent(Texture, Mip);
    VkImageBlit Blit = {};
    Blit.srcSubresource = {Texture->getAspect(), Mip - 1, 0, Desc.ArrayLayers};
    Blit.srcOffsets[1] = {static_cast<int32_t>(SrcExtent.width),
                          static_cast<int32_t>(SrcExtent.height), 1};
    Blit.dstSubresource = {Texture->getAspect(), Mip, 0, Desc.ArrayLayers};
    Blit.dstOffsets[1] = {static_cast<int32_t>(DstExtent.width),
                          static_cast<int32_t>(DstExtent.height), 1};
    vkCmdBlitImage(Command, Texture->getHandle(), SrcLayout,
                   Texture->getHandle(), DstLayout, 1, &Blit,
                   VK_FILTER_LINEAR);
  }

  // bring every level back to the tracked layout
  Barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  Barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
  Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  Barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  Barrier.oldLayout = SrcLayout;
  Barrier.newLayout = DstLayout;
  Barrier.subresourceRange.baseMipLevel = 0;
  Barrier.subresourceRange.levelCount = Desc.MipLevels - 1;
  vkCmdPipelineBarrier2(Command, &Dependency);

  auto &State = LocalStates[Texture];
  State.WriteStages = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  State.WriteAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  State.ReadStages = VK_PIPELINE_STAGE_2_NONE;
  State.ReadAccess = VK_ACCESS_2_NONE;
}
