//---------------------------------------------------------------
// File: RSCommandContext.h
// Proj: RenderSystem_VK
// Info: 录制Vulkan指令并自动处理资源屏障与描述符集
// Date: 2026.10.11
// Comt: 资源绑定方式与D3D11的DeviceContext保持一致，
//       每个topic拥有一个context，可在不同线程并行录制
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include "RSShaderCompile.h"
#include "RSVkObjects.h"

#include <array>
#include <unordered_map>
#include <vector>

constexpr uint32_t RS_COMMAND_CONTEXT_FRAMES = 2;

enum class RS_SHADER_STAGE { VERTEX, GEOMETRY, PIXEL, COMPUTE, SIZE };

class RSCommandContext {
private:
  class RSDevices *Devices;

  struct FRAME_DATA {
    VkCommandPool Pool = VK_NULL_HANDLE;
    VkCommandBuffer Command = VK_NULL_HANDLE;
    // barriers from the global resource state to the first accesses of
    // this context, recorded when the context gets submitted
    VkCommandBuffer Glue = VK_NULL_HANDLE;
    std::vector<VkDescriptorPool> DescriptorPools = {};
    size_t DescriptorPoolIndex = 0;
  };
  std::array<FRAME_DATA, RS_COMMAND_CONTEXT_FRAMES> Frames;
  uint32_t FrameIndex;
  VkCommandBuffer Command;
  bool Recording;

  struct SLOT_TABLE {
    std::array<RSBuffer *, RS_MAX_CBV_SLOT> Cbv = {};
    std::array<RSShaderResourceView *, RS_MAX_SRV_SLOT> Srv = {};
    std::array<RSUnorderedAccessView *, RS_MAX_UAV_SLOT> Uav = {};
    std::array<RSSampler *, RS_MAX_SAMPLER_SLOT> Sampler = {};
  };
  std::array<SLOT_TABLE, static_cast<size_t>(RS_SHADER_STAGE::SIZE)> Slots;
  std::vector<std::pair<RSUnorderedAccessView *, uint32_t>> CounterResets;

  RSPipelineState *GraphicsPipeline;
  RSPipelineState *ComputePipeline;
  VkPipeline BoundPipeline;
  VkPrimitiveTopology Topology;
  RSBuffer *VertexBuffer;
  VkDeviceSize VertexOffset;
  RSBuffer *IndexBuffer;
  VkDeviceSize IndexOffset;
  VkViewport Viewport;
  bool ViewportSet;

  struct RENDERING_STATE {
    std::array<RSRenderTargetView *, RS_MAX_RENDER_TARGETS> Rtvs = {};
    std::array<VkAttachmentLoadOp, RS_MAX_RENDER_TARGETS> LoadOps = {};
    std::array<VkClearColorValue, RS_MAX_RENDER_TARGETS> ClearColors = {};
    uint32_t RtvCount = 0;
    RSDepthStencilView *Dsv = nullptr;
    VkAttachmentLoadOp DepthLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    float ClearDepth = 1.f;
    // render targets have been set
    bool Active = false;
    // vkCmdBeginRendering has been recorded
    bool Begun = false;
  } Rendering;

  struct ENTRY_REQUIREMENT {
    RSTrackedResource *Resource = nullptr;
    RS_ACCESS Access = {};
  };
  std::unordered_map<RSTrackedResource *, RS_TRACKED_STATE> LocalStates;
  std::vector<ENTRY_REQUIREMENT> EntryRequirements;
  std::vector<VkImageMemoryBarrier2> ImageBarriers;
  std::vector<VkBufferMemoryBarrier2> BufferBarriers;

  VkDeviceSize DynamicAlignment;

public:
  RSCommandContext();
  ~RSCommandContext();

  bool startUp(class RSDevices *DevicesPtr);
  void cleanAndStop();

  //-------------------------------------------------------------
  // recording, used by pipeline and devices
  //-------------------------------------------------------------
  void beginRecording();
  void endRecording();
  bool isRecording() const { return Recording; }
  VkCommandBuffer getCommandBuffer() const { return Command; }
  // records the glue command buffer and commits the final states of
  // all resources used by this context into their global states
  VkCommandBuffer recordGlue();

  //-------------------------------------------------------------
  // output merger
  //-------------------------------------------------------------
  void setRenderTargets(UINT NumViews,
                        RSRenderTargetView *const *Rtvs,
                        RSDepthStencilView *Dsv);
  void clearRenderTargetView(RSRenderTargetView *Rtv, const float Color[4]);
  void clearDepthStencilView(RSDepthStencilView *Dsv, float Depth);
  void setViewport(const VkViewport &ViewPort);

  //-------------------------------------------------------------
  // pipeline and input assembler
  //-------------------------------------------------------------
  void setPipelineState(RSPipelineState *Pipeline);
  void setPrimitiveTopology(VkPrimitiveTopology TopologyType);
  void setVertexBuffer(RSBuffer *Buffer, VkDeviceSize Offset = 0);
  void setIndexBuffer(RSBuffer *Buffer, VkDeviceSize Offset = 0);

  //-------------------------------------------------------------
  // resource binding, slots of each stage are independent
  //-------------------------------------------------------------
  void setConstantBuffers(RS_SHADER_STAGE Stage,
                          UINT StartSlot,
                          UINT NumBuffers,
                          RSBuffer *const *Buffers);
  void setShaderResources(RS_SHADER_STAGE Stage,
                          UINT StartSlot,
                          UINT NumViews,
                          RSShaderResourceView *const *Views);
  // InitialCounts follows CSSetUnorderedAccessViews, -1 keeps the counter
  void setUnorderedAccessViews(RS_SHADER_STAGE Stage,
                               UINT StartSlot,
                               UINT NumViews,
                               RSUnorderedAccessView *const *Views,
                               const UINT *InitialCounts = nullptr);
  void setSamplers(RS_SHADER_STAGE Stage,
                   UINT StartSlot,
                   UINT NumSamplers,
                   RSSampler *const *Samplers);

  //-------------------------------------------------------------
  // commands
  //-------------------------------------------------------------
  void draw(UINT VertexCount, UINT StartVertexLocation);
  void drawIndexedInstanced(UINT IndexCountPerInstance,
                            UINT InstanceCount,
                            UINT StartIndexLocation,
                            INT BaseVertexLocation,
                            UINT StartInstanceLocation);
  void dispatch(UINT GroupCountX, UINT GroupCountY, UINT GroupCountZ);

  void copyTexture(RSTexture *Dst, RSTexture *Src);
  void copyBuffer(RSBuffer *Dst,
                  VkDeviceSize DstOffset,
                  RSBuffer *Src,
                  VkDeviceSize SrcOffset,
                  VkDeviceSize Size);
  void copyStructureCount(RSBuffer *DstBuffer,
                          UINT DstAlignedByteOffset,
                          RSUnorderedAccessView *SrcView);
  void generateMips(RSShaderResourceView *Srv);

private:
  //-------------------------------------------------------------
  // barriers
  //-------------------------------------------------------------
  void requireAccess(RSTrackedResource *Resource, const RS_ACCESS &Access);
  void flushBarriers();
  RS_ACCESS getShaderAccess(RSTrackedResource *Resource,
                            VkDescriptorType Type,
                            bool Write,
                            VkPipelineStageFlags2 Stage) const;
  bool isRenderTarget(RSTrackedResource *Resource) const;

  //-------------------------------------------------------------
  // rendering
  //-------------------------------------------------------------
  void prepareRendering();
  void beginRendering();
  void endRendering();
  void flushPendingClears();

  //-------------------------------------------------------------
  // descriptors
  //-------------------------------------------------------------
  void prepareResources(RSPipelineState *Pipeline, bool Compute);
  void bindDescriptors(RSPipelineState *Pipeline, bool Compute);
  VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout Layout);
  void applyCounterResets();
};

// common accesses
namespace rs_access {

RS_ACCESS colorAttachment(const RSTexture *Texture);
RS_ACCESS depthAttachment();
RS_ACCESS transferSrc(const RSTrackedResource *Resource);
RS_ACCESS transferDst(const RSTrackedResource *Resource);
RS_ACCESS present();

} // namespace rs_access
