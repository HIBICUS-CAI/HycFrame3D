//---------------------------------------------------------------
// File: RSDevices.h
// Proj: RenderSystem_VK
// Info: 保存并提供与Vulkan直接相关的内容和引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 负责设备、交换链、帧同步、资源创建、上传与延迟销毁
//---------------------------------------------------------------

#pragma once

#include "RSCommon.h"

#include "RSVkObjects.h"

#include <array>
#include <mutex>
#include <unordered_map>
#include <vector>

typedef struct VmaAllocator_T *VmaAllocator;
struct SDL_Window;

constexpr uint32_t RS_FRAME_IN_FLIGHT = 2;

struct RENDER_DEVICE_CONFIG {
  UINT ForceAdapterIndex = static_cast<UINT>(-1);
  bool ForceSingleThreadFlag = false;
};

struct RS_DYNAMIC_ALLOCATION {
  VkBuffer Buffer = VK_NULL_HANDLE;
  VkDeviceSize Offset = 0;
  void *CpuAddress = nullptr;
};

class RSDevices {
private:
  class RSRoot_DX11 *RenderSystemRoot;

  RENDER_DEVICE_CONFIG RenderDeivceConfig;

  SDL_Window *Window;
  VkInstance Instance;
  VkDebugUtilsMessengerEXT DebugMessenger;
  VkSurfaceKHR Surface;
  VkPhysicalDevice PhysicalDevice;
  VkPhysicalDeviceProperties PhysicalDeviceProperties;
  VkDevice Device;
  uint32_t QueueFamily;
  VkQueue Queue;
  std::mutex QueueLock;
  VmaAllocator Allocator;
  VkPipelineCache PipelineCache;

  // swap chain
  VkSwapchainKHR SwapChain;
  VkFormat SwapChainFormat;
  VkExtent2D SwapChainExtent;
  std::vector<VkImage> SwapChainImages;
  std::vector<VkSemaphore> RenderFinishedSemaphores;
  bool SwapChainDirty;

  // per frame in flight
  struct FRAME_RESOURCE {
    VkFence Fence = VK_NULL_HANDLE;
    VkSemaphore ImageAcquired = VK_NULL_HANDLE;
    VkCommandPool CommandPool = VK_NULL_HANDLE;
    VkCommandBuffer FinalCommand = VK_NULL_HANDLE;
    uint64_t Serial = 0;
    // ring buffer for dynamic buffers
    struct RING_BLOCK {
      VkBuffer Buffer = VK_NULL_HANDLE;
      VmaAllocation Allocation = nullptr;
      uint8_t *CpuAddress = nullptr;
      VkDeviceSize Size = 0;
    };
    std::vector<RING_BLOCK> RingBlocks = {};
    size_t RingBlockIndex = 0;
    VkDeviceSize RingOffset = 0;
  };
  std::array<FRAME_RESOURCE, RS_FRAME_IN_FLIGHT> Frames;
  std::mutex RingLock;
  uint64_t FrameSerial;
  uint64_t CompletedSerial;
  bool FrameBegun;
  std::vector<class RSCommandContext *> SubmittedContexts;

  // deferred destruction
  std::mutex DestroyLock;
  std::vector<std::pair<RSVkObject *, uint64_t>> DestroyQueue;
  bool ShuttingDown;

  // one time submit for uploading initial data
  std::mutex UploadLock;
  VkCommandPool UploadCommandPool;

  // render targets owned by device
  RSTexture *HighDynamicTexture;
  RSRenderTargetView *HighDynamicRtv;
  RSShaderResourceView *HighDynamicSrv;
  RSUnorderedAccessView *HighDynamicUav;
  // scene is always rendered into this 1280x720 image, it is scaled
  // into the real swap chain image on presenting like dxgi does
  RSTexture *BackBufferTexture;
  RSRenderTargetView *BackBufferRtv;

  // resources bound to empty slots, reading them returns zero like
  // a null view in direct3d 11
  std::mutex DummyLock;
  std::unordered_map<uint64_t, RSResourceView *> DummyViews;
  RSBuffer *DummyBuffer;
  RSSampler *DefaultSampler;

  bool ConcurrentCreateSupportFlag;
  bool CommandListSupportFlag;
  bool ValidationEnabled;

  UINT WndWidth;
  UINT WndHeight;

public:
  RSDevices();
  ~RSDevices();

  bool startUp(class RSRoot_DX11 *RootPtr, SDL_Window *WndHandle);
  void cleanAndStop();

  VkInstance getInstance() const { return Instance; }
  VkPhysicalDevice getPhysicalDevice() const { return PhysicalDevice; }
  const VkPhysicalDeviceProperties &getPhysicalDeviceProperties() const {
    return PhysicalDeviceProperties;
  }
  VkDevice getDevice() const { return Device; }
  VmaAllocator getAllocator() const { return Allocator; }
  VkPipelineCache getPipelineCache() const { return PipelineCache; }
  uint32_t getQueueFamily() const { return QueueFamily; }

  //-------------------------------------------------------------
  // resource creation, thread safe
  //-------------------------------------------------------------
  RSBuffer *createBuffer(const RS_BUFFER_DESC &Desc,
                         const void *InitData = nullptr);
  RSTexture *createTexture(const RS_TEXTURE_DESC &Desc,
                           const RS_SUBRESOURCE_DATA *InitData = nullptr);
  RSShaderResourceView *
  createShaderResourceView(RSTexture *Texture,
                           const RS_TEXTURE_VIEW_DESC *Desc = nullptr);
  RSShaderResourceView *createShaderResourceView(RSBuffer *Buffer,
                                                 const RS_BUFFER_VIEW_DESC &Desc);
  RSUnorderedAccessView *
  createUnorderedAccessView(RSTexture *Texture,
                            const RS_TEXTURE_VIEW_DESC *Desc = nullptr);
  RSUnorderedAccessView *
  createUnorderedAccessView(RSBuffer *Buffer, const RS_BUFFER_VIEW_DESC &Desc);
  RSRenderTargetView *
  createRenderTargetView(RSTexture *Texture,
                         const RS_TEXTURE_VIEW_DESC *Desc = nullptr);
  RSDepthStencilView *
  createDepthStencilView(RSTexture *Texture,
                         const RS_TEXTURE_VIEW_DESC *Desc = nullptr);
  RSSampler *createSampler(const VkSamplerCreateInfo &Info);
  RSShader *createShader(std::vector<uint32_t> &&Spirv);
  RSPipelineState *createGraphicsPipeline(const RS_GRAPHICS_PIPELINE_DESC &Desc);
  RSPipelineState *createComputePipeline(RSShader *ComputeShader);

  //-------------------------------------------------------------
  // render targets
  //-------------------------------------------------------------
  RSRenderTargetView *getHighDynamicRtv() const { return HighDynamicRtv; }
  RSRenderTargetView *getSwapChainRtv() const { return BackBufferRtv; }
  RSShaderResourceView *getHighDynamicSrv() const { return HighDynamicSrv; }
  RSUnorderedAccessView *getHighDynamicUav() const { return HighDynamicUav; }
  RSTexture *getHighDynamicTexture() const { return HighDynamicTexture; }

  void copyHighDynamicTexture(class RSCommandContext *Context,
                              RSTexture *DstResource);

  bool getConcurrentCreateSupport() const;
  bool getCommandListSupport() const;
  UINT getCurrWndWidth() const;
  UINT getCurrWndHeight() const;

  //-------------------------------------------------------------
  // frame
  //-------------------------------------------------------------
  void beginFrame();
  uint32_t getFrameIndex() const;
  uint64_t getFrameSerial() const { return FrameSerial; }
  // command contexts are submitted in the order of this call
  void submitContext(class RSCommandContext *Context);
  void presentSwapChain();
  void waitIdle();

  RS_DYNAMIC_ALLOCATION allocateDynamic(VkDeviceSize Size,
                                        VkDeviceSize Alignment);

  void deferDestroy(RSVkObject *Object);

  //-------------------------------------------------------------
  // helpers used by objects and contexts
  //-------------------------------------------------------------
  RSResourceView *getDummyView(VkDescriptorType Type,
                               rs_tool::SPIRV_IMAGE_DIM Dim,
                               bool Arrayed,
                               rs_tool::SPIRV_SCALAR_KIND Scalar);
  RSBuffer *getDummyBuffer() const { return DummyBuffer; }
  RSSampler *getDefaultSampler() const { return DefaultSampler; }

  // records barriers that move a resource from its global state to the
  // first access of a command context
  static void recordTransition(VkCommandBuffer Cmd,
                               RSTrackedResource *Resource,
                               const RS_TRACKED_STATE &From,
                               const RS_ACCESS &To,
                               const VkImageSubresourceRange *Range = nullptr);

private:
  bool createInstance();
  bool selectPhysicalDevice();
  bool createLogicalDevice();
  bool createAllocator();
  bool createFrameResources();
  bool createSwapChain();
  void destroySwapChain();
  bool createHighDynamicTexture(UINT Width, UINT Height);
  bool createBackBuffer(UINT Width, UINT Height);
  bool createDummyResources();
  void destroyFrameResources();
  void collectGarbage(uint64_t SafeSerial);

  bool uploadBuffer(RSBuffer *Buffer, const void *Data, VkDeviceSize Size);
  bool uploadTexture(RSTexture *Texture, const RS_SUBRESOURCE_DATA *Data);
  bool transitionNewTexture(RSTexture *Texture);
  VkCommandBuffer beginOneTimeCommand();
  bool endOneTimeCommand(VkCommandBuffer Cmd);
  bool submitToQueue(const VkSubmitInfo2 &Submit, VkFence Fence);

  void recordFinalCommand(VkCommandBuffer Cmd, uint32_t ImageIndex);
};
