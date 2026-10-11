//---------------------------------------------------------------
// File: RSDevices.cpp
// Proj: RenderSystem_VK
// Info: 保存并提供与Vulkan直接相关的内容和引用
// Date: 2021.9.13
// Mail: cai_genkan@outlook.com
// Comt: 负责设备、交换链、帧同步、资源创建、上传与延迟销毁
//---------------------------------------------------------------

#include "RSDevices.h"

#include "RSCommandContext.h"
#include "RSRoot_VK.h"

#include <SDL.h>
#include <SDL_vulkan.h>
#include <TextUtility.h>
#include <vma/vk_mem_alloc.h>

#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

namespace {

constexpr VkDeviceSize RING_BLOCK_SIZE = 32ull * 1024ull * 1024ull;
constexpr VkDeviceSize DUMMY_BUFFER_SIZE = 16384;

VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity,
              VkDebugUtilsMessageTypeFlagsEXT Type,
              const VkDebugUtilsMessengerCallbackDataEXT *Data,
              void *UserData) {
  (void)Type;
  (void)UserData;
  const char *Level =
      (Severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR"
                                                                 : "WARNING";
  std::fprintf(stderr, "[[[VULKAN %s]]] : %s\n", Level, Data->pMessage);
  return VK_FALSE;
}

bool hasLayer(const char *Name) {
  uint32_t Count = 0;
  vkEnumerateInstanceLayerProperties(&Count, nullptr);
  std::vector<VkLayerProperties> Layers(Count);
  vkEnumerateInstanceLayerProperties(&Count, Layers.data());
  for (const auto &Layer : Layers) {
    if (!std::strcmp(Layer.layerName, Name)) {
      return true;
    }
  }
  return false;
}

bool shouldEnableValidation() {
  const char *Env = std::getenv("HYC_VK_VALIDATION");
  if (Env) {
    return std::strcmp(Env, "0") != 0;
  }
#ifdef _DEBUG
  return true;
#else
  return false;
#endif // _DEBUG
}

VkDeviceSize alignUp(VkDeviceSize Value, VkDeviceSize Alignment) {
  return (Value + Alignment - 1) / Alignment * Alignment;
}

} // namespace

RSDevices::RSDevices()
    : RenderSystemRoot(nullptr), RenderDeivceConfig({}), Window(nullptr),
      Instance(VK_NULL_HANDLE), DebugMessenger(VK_NULL_HANDLE),
      Surface(VK_NULL_HANDLE), PhysicalDevice(VK_NULL_HANDLE),
      PhysicalDeviceProperties({}), Device(VK_NULL_HANDLE), QueueFamily(0),
      Queue(VK_NULL_HANDLE), Allocator(nullptr),
      PipelineCache(VK_NULL_HANDLE), SwapChain(VK_NULL_HANDLE),
      SwapChainFormat(VK_FORMAT_UNDEFINED), SwapChainExtent({0, 0}),
      SwapChainImages({}), RenderFinishedSemaphores({}), SwapChainDirty(false),
      Frames({}), FrameSerial(1), CompletedSerial(0), FrameBegun(false),
      SubmittedContexts({}), DestroyQueue({}), ShuttingDown(false),
      UploadCommandPool(VK_NULL_HANDLE), HighDynamicTexture(nullptr),
      HighDynamicRtv(nullptr), HighDynamicSrv(nullptr),
      HighDynamicUav(nullptr), BackBufferTexture(nullptr),
      BackBufferRtv(nullptr), DummyViews({}), DummyBuffer(nullptr),
      DefaultSampler(nullptr), ConcurrentCreateSupportFlag(false),
      CommandListSupportFlag(false), ValidationEnabled(false), WndWidth(0),
      WndHeight(0) {}

RSDevices::~RSDevices() {}

bool RSDevices::startUp(RSRoot_VK *RootPtr, SDL_Window *WndHandle) {
  if (!RootPtr || !WndHandle) {
    return false;
  }

  RenderSystemRoot = RootPtr;
  Window = WndHandle;

  {
    using namespace hyc;
    using namespace hyc::text;
    TomlNode ConfigRoot = {};
    TomlNode Node = {};
    std::string ErrorMess = "";
    if (!loadTomlAndParse(ConfigRoot,
                          "./Assets/Configs/render-device-config.toml",
                          ErrorMess)) {
      return false;
    }

    if (!getTomlNode(ConfigRoot, "device.adapter.manual-setting", Node)) {
      return false;
    } else {
      if (getAs<bool>(Node) &&
          getTomlNode(ConfigRoot, "device.adapter.index-designation", Node)) {
        RenderDeivceConfig.ForceAdapterIndex = getAs<uint>(Node);
      }
    }

    if (!getTomlNode(ConfigRoot, "device.single-thread", Node)) {
      return false;
    } else {
      RenderDeivceConfig.ForceSingleThreadFlag = getAs<bool>(Node);
    }
  }

  // the render resolution is fixed, it's scaled to the window on present
  WndWidth = 1280;
  WndHeight = 720;

  if (!createInstance()) {
    return false;
  }
  if (!SDL_Vulkan_CreateSurface(Window, Instance, &Surface)) {
    std::fprintf(stderr, "[[[ERROR]]] : %s\n", SDL_GetError());
    return false;
  }
  if (!selectPhysicalDevice()) {
    return false;
  }
  if (!createLogicalDevice()) {
    return false;
  }
  if (!createAllocator()) {
    return false;
  }
  if (!createFrameResources()) {
    return false;
  }
  if (!createSwapChain()) {
    return false;
  }
  if (!createDummyResources()) {
    return false;
  }
  if (!createHighDynamicTexture(WndWidth, WndHeight)) {
    return false;
  }
  if (!createBackBuffer(WndWidth, WndHeight)) {
    return false;
  }

  // vulkan command buffers can always be recorded on any thread
  ConcurrentCreateSupportFlag = true;
  CommandListSupportFlag = true;

  return true;
}

void RSDevices::cleanAndStop() {
  if (Device) {
    vkDeviceWaitIdle(Device);
  }

  ShuttingDown = true;
  collectGarbage(UINT64_MAX);

  auto SafeRelease = [](auto *&Ptr) {
    if (Ptr) {
      Ptr->Release();
      Ptr = nullptr;
    }
  };
  SafeRelease(BackBufferRtv);
  SafeRelease(BackBufferTexture);
  SafeRelease(HighDynamicUav);
  SafeRelease(HighDynamicSrv);
  SafeRelease(HighDynamicRtv);
  SafeRelease(HighDynamicTexture);
  for (auto &Dummy : DummyViews) {
    Dummy.second->Release();
  }
  DummyViews.clear();
  SafeRelease(DummyBuffer);
  SafeRelease(DefaultSampler);
  collectGarbage(UINT64_MAX);

  destroySwapChain();
  destroyFrameResources();

  if (UploadCommandPool) {
    vkDestroyCommandPool(Device, UploadCommandPool, nullptr);
    UploadCommandPool = VK_NULL_HANDLE;
  }
  if (PipelineCache) {
    vkDestroyPipelineCache(Device, PipelineCache, nullptr);
    PipelineCache = VK_NULL_HANDLE;
  }
  RSVkObject::reportLiveObjects();
  if (Allocator) {
    vmaDestroyAllocator(Allocator);
    Allocator = nullptr;
  }
  if (Device) {
    vkDestroyDevice(Device, nullptr);
    Device = VK_NULL_HANDLE;
  }
  if (Surface) {
    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    Surface = VK_NULL_HANDLE;
  }
  if (DebugMessenger) {
    auto DestroyFunc = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(Instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (DestroyFunc) {
      DestroyFunc(Instance, DebugMessenger, nullptr);
    }
    DebugMessenger = VK_NULL_HANDLE;
  }
  if (Instance) {
    vkDestroyInstance(Instance, nullptr);
    Instance = VK_NULL_HANDLE;
  }
}

//---------------------------------------------------------------
// initialization
//---------------------------------------------------------------

bool RSDevices::createInstance() {
  unsigned int ExtCount = 0;
  if (!SDL_Vulkan_GetInstanceExtensions(Window, &ExtCount, nullptr)) {
    return false;
  }
  std::vector<const char *> Extensions(ExtCount);
  SDL_Vulkan_GetInstanceExtensions(Window, &ExtCount, Extensions.data());

  std::vector<const char *> Layers = {};
  ValidationEnabled = shouldEnableValidation() &&
                      hasLayer("VK_LAYER_KHRONOS_validation");
  if (ValidationEnabled) {
    Layers.push_back("VK_LAYER_KHRONOS_validation");
    Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  VkApplicationInfo AppInfo = {};
  AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  AppInfo.pApplicationName = "HycFrame3D";
  AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  AppInfo.pEngineName = "HycFrame3D";
  AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  AppInfo.apiVersion = VK_API_VERSION_1_3;

  VkDebugUtilsMessengerCreateInfoEXT DebugInfo = {};
  DebugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  DebugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  DebugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  DebugInfo.pfnUserCallback = debugCallback;

  VkInstanceCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  Info.pNext = ValidationEnabled ? &DebugInfo : nullptr;
  Info.pApplicationInfo = &AppInfo;
  Info.enabledLayerCount = static_cast<uint32_t>(Layers.size());
  Info.ppEnabledLayerNames = Layers.data();
  Info.enabledExtensionCount = static_cast<uint32_t>(Extensions.size());
  Info.ppEnabledExtensionNames = Extensions.data();
  FAIL_VK_RETURN(vkCreateInstance(&Info, nullptr, &Instance));

  if (ValidationEnabled) {
    auto CreateFunc = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(Instance, "vkCreateDebugUtilsMessengerEXT"));
    if (CreateFunc) {
      CreateFunc(Instance, &DebugInfo, nullptr, &DebugMessenger);
    }
  }

  return true;
}

bool RSDevices::selectPhysicalDevice() {
  uint32_t Count = 0;
  vkEnumeratePhysicalDevices(Instance, &Count, nullptr);
  std::vector<VkPhysicalDevice> AllDevices(Count);
  vkEnumeratePhysicalDevices(Instance, &Count, AllDevices.data());
  if (!Count) {
    return false;
  }

  std::ofstream File("./Assets/Configs/AutoCreated_AdapterInfoInSystem.txt");
  for (uint32_t I = 0; I < Count; I++) {
    VkPhysicalDeviceProperties Props = {};
    vkGetPhysicalDeviceProperties(AllDevices[I], &Props);
    File << "--------------------\nAdapter Info in Index : " << I << "\n"
         << Props.deviceName << "\n--------------------\n";
  }
  File.close();

  auto IsSuitable = [this](VkPhysicalDevice Candidate,
                           uint32_t &OutFamily) -> bool {
    VkPhysicalDeviceProperties Props = {};
    vkGetPhysicalDeviceProperties(Candidate, &Props);
    if (Props.apiVersion < VK_API_VERSION_1_3) {
      return false;
    }

    VkPhysicalDeviceVulkan13Features Features13 = {};
    Features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    VkPhysicalDeviceFeatures2 Features = {};
    Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    Features.pNext = &Features13;
    vkGetPhysicalDeviceFeatures2(Candidate, &Features);
    if (!Features13.dynamicRendering || !Features13.synchronization2 ||
        !Features13.shaderDemoteToHelperInvocation ||
        !Features.features.geometryShader ||
        !Features.features.shaderStorageImageReadWithoutFormat ||
        !Features.features.shaderStorageImageWriteWithoutFormat) {
      return false;
    }

    uint32_t FamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(Candidate, &FamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> Families(FamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(Candidate, &FamilyCount,
                                             Families.data());
    for (uint32_t I = 0; I < FamilyCount; I++) {
      VkBool32 Present = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(Candidate, I, Surface, &Present);
      if ((Families[I].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
          (Families[I].queueFlags & VK_QUEUE_COMPUTE_BIT) && Present) {
        OutFamily = I;
        return true;
      }
    }
    return false;
  };

  auto TypeScore = [](VkPhysicalDeviceType Type) {
    switch (Type) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      return 4;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return 3;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return 2;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      return 1;
    default:
      return 0;
    }
  };

  uint32_t Family = 0;
  if (RenderDeivceConfig.ForceAdapterIndex < Count &&
      IsSuitable(AllDevices[RenderDeivceConfig.ForceAdapterIndex], Family)) {
    PhysicalDevice = AllDevices[RenderDeivceConfig.ForceAdapterIndex];
    QueueFamily = Family;
  } else {
    int BestScore = -1;
    for (auto Candidate : AllDevices) {
      if (!IsSuitable(Candidate, Family)) {
        continue;
      }
      VkPhysicalDeviceProperties Props = {};
      vkGetPhysicalDeviceProperties(Candidate, &Props);
      int Score = TypeScore(Props.deviceType);
      if (Score > BestScore) {
        BestScore = Score;
        PhysicalDevice = Candidate;
        QueueFamily = Family;
      }
    }
  }

  if (!PhysicalDevice) {
    std::fprintf(stderr, "[[[ERROR]]] : no vulkan 1.3 device with required "
                         "features was found\n");
    return false;
  }

  vkGetPhysicalDeviceProperties(PhysicalDevice, &PhysicalDeviceProperties);
  std::fprintf(stderr, "[[[MESSAGE]]] : render device : %s\n",
               PhysicalDeviceProperties.deviceName);

  return true;
}

bool RSDevices::createLogicalDevice() {
  VkPhysicalDeviceFeatures Supported = {};
  vkGetPhysicalDeviceFeatures(PhysicalDevice, &Supported);

  VkPhysicalDeviceFeatures2 Features = {};
  Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  Features.features.geometryShader = VK_TRUE;
  Features.features.shaderStorageImageReadWithoutFormat = VK_TRUE;
  Features.features.shaderStorageImageWriteWithoutFormat = VK_TRUE;
  Features.features.samplerAnisotropy = Supported.samplerAnisotropy;
  Features.features.depthClamp = Supported.depthClamp;
  Features.features.fillModeNonSolid = Supported.fillModeNonSolid;
  Features.features.independentBlend = Supported.independentBlend;
  Features.features.textureCompressionBC = Supported.textureCompressionBC;
  Features.features.imageCubeArray = Supported.imageCubeArray;

  VkPhysicalDeviceVulkan13Features Features13 = {};
  Features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
  Features13.dynamicRendering = VK_TRUE;
  Features13.synchronization2 = VK_TRUE;
  // hlsl discard is compiled to demote like d3d11 behaves
  Features13.shaderDemoteToHelperInvocation = VK_TRUE;
  Features.pNext = &Features13;

  float Priority = 1.f;
  VkDeviceQueueCreateInfo QueueInfo = {};
  QueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueInfo.queueFamilyIndex = QueueFamily;
  QueueInfo.queueCount = 1;
  QueueInfo.pQueuePriorities = &Priority;

  const char *Extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  VkDeviceCreateInfo Info = {};
  Info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  Info.pNext = &Features;
  Info.queueCreateInfoCount = 1;
  Info.pQueueCreateInfos = &QueueInfo;
  Info.enabledExtensionCount = 1;
  Info.ppEnabledExtensionNames = Extensions;
  FAIL_VK_RETURN(vkCreateDevice(PhysicalDevice, &Info, nullptr, &Device));

  vkGetDeviceQueue(Device, QueueFamily, 0, &Queue);

  VkPipelineCacheCreateInfo CacheInfo = {};
  CacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  FAIL_VK_RETURN(
      vkCreatePipelineCache(Device, &CacheInfo, nullptr, &PipelineCache));

  VkCommandPoolCreateInfo PoolInfo = {};
  PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  PoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                   VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  PoolInfo.queueFamilyIndex = QueueFamily;
  FAIL_VK_RETURN(
      vkCreateCommandPool(Device, &PoolInfo, nullptr, &UploadCommandPool));

  return true;
}

bool RSDevices::createAllocator() {
  VmaAllocatorCreateInfo Info = {};
  Info.vulkanApiVersion = VK_API_VERSION_1_3;
  Info.instance = Instance;
  Info.physicalDevice = PhysicalDevice;
  Info.device = Device;
  FAIL_VK_RETURN(vmaCreateAllocator(&Info, &Allocator));
  return true;
}

bool RSDevices::createFrameResources() {
  for (auto &Frame : Frames) {
    VkFenceCreateInfo FenceInfo = {};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    FAIL_VK_RETURN(vkCreateFence(Device, &FenceInfo, nullptr, &Frame.Fence));

    VkSemaphoreCreateInfo SemInfo = {};
    SemInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    FAIL_VK_RETURN(
        vkCreateSemaphore(Device, &SemInfo, nullptr, &Frame.ImageAcquired));

    VkCommandPoolCreateInfo PoolInfo = {};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    PoolInfo.queueFamilyIndex = QueueFamily;
    FAIL_VK_RETURN(
        vkCreateCommandPool(Device, &PoolInfo, nullptr, &Frame.CommandPool));

    VkCommandBufferAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    AllocInfo.commandPool = Frame.CommandPool;
    AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    AllocInfo.commandBufferCount = 1;
    FAIL_VK_RETURN(
        vkAllocateCommandBuffers(Device, &AllocInfo, &Frame.FinalCommand));
  }
  return true;
}

void RSDevices::destroyFrameResources() {
  for (auto &Frame : Frames) {
    for (auto &Block : Frame.RingBlocks) {
      vmaUnmapMemory(Allocator, Block.Allocation);
      vmaDestroyBuffer(Allocator, Block.Buffer, Block.Allocation);
    }
    Frame.RingBlocks.clear();
    if (Frame.CommandPool) {
      vkDestroyCommandPool(Device, Frame.CommandPool, nullptr);
    }
    if (Frame.ImageAcquired) {
      vkDestroySemaphore(Device, Frame.ImageAcquired, nullptr);
    }
    if (Frame.Fence) {
      vkDestroyFence(Device, Frame.Fence, nullptr);
    }
    Frame = {};
  }
}

bool RSDevices::createSwapChain() {
  int DrawableWidth = 0;
  int DrawableHeight = 0;
  SDL_Vulkan_GetDrawableSize(Window, &DrawableWidth, &DrawableHeight);

  VkSurfaceCapabilitiesKHR Caps = {};
  FAIL_VK_RETURN(
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &Caps));

  VkExtent2D Extent = Caps.currentExtent;
  if (Extent.width == UINT32_MAX) {
    Extent.width = std::clamp(static_cast<uint32_t>(DrawableWidth),
                              Caps.minImageExtent.width,
                              Caps.maxImageExtent.width);
    Extent.height = std::clamp(static_cast<uint32_t>(DrawableHeight),
                               Caps.minImageExtent.height,
                               Caps.maxImageExtent.height);
  }
  if (!Extent.width || !Extent.height) {
    // minimized, try again later
    SwapChainDirty = true;
    return true;
  }

  uint32_t FormatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount,
                                       nullptr);
  std::vector<VkSurfaceFormatKHR> Formats(FormatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &FormatCount,
                                       Formats.data());
  // tone mapping already outputs srgb encoded values, use unorm formats
  VkSurfaceFormatKHR Chosen = Formats.empty() ? VkSurfaceFormatKHR{}
                                              : Formats[0];
  for (const auto &Format : Formats) {
    if ((Format.format == VK_FORMAT_B8G8R8A8_UNORM ||
         Format.format == VK_FORMAT_R8G8B8A8_UNORM) &&
        Format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      Chosen = Format;
      break;
    }
  }

  uint32_t ModeCount = 0;
  vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface,
                                            &ModeCount, nullptr);
  std::vector<VkPresentModeKHR> Modes(ModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface,
                                            &ModeCount, Modes.data());
  // dxgi Present(0, 0) doesn't wait for vertical blank
  VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;
  for (auto Preferred :
       {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR}) {
    if (std::find(Modes.begin(), Modes.end(), Preferred) != Modes.end()) {
      PresentMode = Preferred;
      break;
    }
  }

  uint32_t ImageCount = Caps.minImageCount + 1;
  if (Caps.maxImageCount && ImageCount > Caps.maxImageCount) {
    ImageCount = Caps.maxImageCount;
  }

  VkSwapchainCreateInfoKHR Info = {};
  Info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  Info.surface = Surface;
  Info.minImageCount = ImageCount;
  Info.imageFormat = Chosen.format;
  Info.imageColorSpace = Chosen.colorSpace;
  Info.imageExtent = Extent;
  Info.imageArrayLayers = 1;
  Info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  Info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  Info.preTransform = Caps.currentTransform;
  Info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  Info.presentMode = PresentMode;
  Info.clipped = VK_TRUE;
  FAIL_VK_RETURN(vkCreateSwapchainKHR(Device, &Info, nullptr, &SwapChain));

  SwapChainFormat = Chosen.format;
  SwapChainExtent = Extent;

  uint32_t Count = 0;
  vkGetSwapchainImagesKHR(Device, SwapChain, &Count, nullptr);
  SwapChainImages.resize(Count);
  vkGetSwapchainImagesKHR(Device, SwapChain, &Count, SwapChainImages.data());

  RenderFinishedSemaphores.resize(Count);
  for (auto &Semaphore : RenderFinishedSemaphores) {
    VkSemaphoreCreateInfo SemInfo = {};
    SemInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    FAIL_VK_RETURN(vkCreateSemaphore(Device, &SemInfo, nullptr, &Semaphore));
  }

  SwapChainDirty = false;
  return true;
}

void RSDevices::destroySwapChain() {
  for (auto Semaphore : RenderFinishedSemaphores) {
    vkDestroySemaphore(Device, Semaphore, nullptr);
  }
  RenderFinishedSemaphores.clear();
  SwapChainImages.clear();
  if (SwapChain) {
    vkDestroySwapchainKHR(Device, SwapChain, nullptr);
    SwapChain = VK_NULL_HANDLE;
  }
}

bool RSDevices::createHighDynamicTexture(UINT Width, UINT Height) {
  RS_TEXTURE_DESC Desc = {};
  Desc.Format = VK_FORMAT_R16G16B16A16_SFLOAT;
  Desc.Width = Width;
  Desc.Height = Height;
  Desc.Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
               VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
  HighDynamicTexture = createTexture(Desc);
  FAIL_NULL_RETURN(HighDynamicTexture);
  HighDynamicRtv = createRenderTargetView(HighDynamicTexture);
  FAIL_NULL_RETURN(HighDynamicRtv);
  HighDynamicSrv = createShaderResourceView(HighDynamicTexture);
  FAIL_NULL_RETURN(HighDynamicSrv);
  HighDynamicUav = createUnorderedAccessView(HighDynamicTexture);
  FAIL_NULL_RETURN(HighDynamicUav);
  return true;
}

bool RSDevices::createBackBuffer(UINT Width, UINT Height) {
  RS_TEXTURE_DESC Desc = {};
  Desc.Format = VK_FORMAT_R8G8B8A8_UNORM;
  Desc.Width = Width;
  Desc.Height = Height;
  Desc.Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  BackBufferTexture = createTexture(Desc);
  FAIL_NULL_RETURN(BackBufferTexture);
  BackBufferRtv = createRenderTargetView(BackBufferTexture);
  FAIL_NULL_RETURN(BackBufferRtv);
  return true;
}

bool RSDevices::createDummyResources() {
  RS_BUFFER_DESC BufDesc = {};
  BufDesc.Size = DUMMY_BUFFER_SIZE;
  BufDesc.Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                  VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
                  VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT |
                  VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                  VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  DummyBuffer = createBuffer(BufDesc);
  FAIL_NULL_RETURN(DummyBuffer);

  // direct3d 11 default sampler state
  VkSamplerCreateInfo SamplerInfo = {};
  SamplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  SamplerInfo.magFilter = VK_FILTER_LINEAR;
  SamplerInfo.minFilter = VK_FILTER_LINEAR;
  SamplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  SamplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  SamplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  SamplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  SamplerInfo.minLod = -FLT_MAX;
  SamplerInfo.maxLod = FLT_MAX;
  SamplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  DefaultSampler = createSampler(SamplerInfo);
  FAIL_NULL_RETURN(DefaultSampler);

  return true;
}

//---------------------------------------------------------------
// resource creation
//---------------------------------------------------------------

RSBuffer *RSDevices::createBuffer(const RS_BUFFER_DESC &Desc,
                                  const void *InitData) {
  auto Buffer = new RSBuffer(this, Desc);
  if (!Buffer->create()) {
    delete Buffer;
    return nullptr;
  }

  if (Buffer->isDynamic()) {
    if (InitData) {
      std::memcpy(Buffer->map(), InitData, static_cast<size_t>(Desc.Size));
      Buffer->unmap();
    }
    return Buffer;
  }

  // buffers start zero filled when no data is given
  if (!uploadBuffer(Buffer, InitData, Desc.Size)) {
    delete Buffer;
    return nullptr;
  }

  return Buffer;
}

RSTexture *RSDevices::createTexture(const RS_TEXTURE_DESC &Desc,
                                    const RS_SUBRESOURCE_DATA *InitData) {
  auto Texture = new RSTexture(this, Desc);
  if (!Texture->create()) {
    delete Texture;
    return nullptr;
  }

  bool Result = InitData ? uploadTexture(Texture, InitData)
                         : transitionNewTexture(Texture);
  if (!Result) {
    delete Texture;
    return nullptr;
  }

  return Texture;
}

RSShaderResourceView *
RSDevices::createShaderResourceView(RSTexture *Texture,
                                    const RS_TEXTURE_VIEW_DESC *Desc) {
  if (!Texture) {
    return nullptr;
  }
  RS_TEXTURE_VIEW_DESC ViewDesc = Desc ? *Desc : RS_TEXTURE_VIEW_DESC{};
  if (!Desc) {
    const auto &TexDesc = Texture->getDesc();
    if (TexDesc.CubeCompatible && TexDesc.ArrayLayers == 6) {
      ViewDesc.ViewType = VK_IMAGE_VIEW_TYPE_CUBE;
    } else if (TexDesc.ArrayLayers > 1) {
      ViewDesc.ViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    } else if (TexDesc.Type == VK_IMAGE_TYPE_3D) {
      ViewDesc.ViewType = VK_IMAGE_VIEW_TYPE_3D;
    } else if (TexDesc.Type == VK_IMAGE_TYPE_1D) {
      ViewDesc.ViewType = VK_IMAGE_VIEW_TYPE_1D;
    }
  }
  auto View = new RSShaderResourceView(this);
  if (!View->createTextureView(Texture, ViewDesc, VK_IMAGE_USAGE_SAMPLED_BIT)) {
    delete View;
    return nullptr;
  }
  return View;
}

RSShaderResourceView *
RSDevices::createShaderResourceView(RSBuffer *Buffer,
                                    const RS_BUFFER_VIEW_DESC &Desc) {
  if (!Buffer) {
    return nullptr;
  }
  auto View = new RSShaderResourceView(this);
  if (!View->createBufferView(Buffer, Desc)) {
    delete View;
    return nullptr;
  }
  return View;
}

RSUnorderedAccessView *
RSDevices::createUnorderedAccessView(RSTexture *Texture,
                                     const RS_TEXTURE_VIEW_DESC *Desc) {
  if (!Texture) {
    return nullptr;
  }
  RS_TEXTURE_VIEW_DESC ViewDesc = Desc ? *Desc : RS_TEXTURE_VIEW_DESC{};
  if (!Desc) {
    // uav sees one mip level like D3D11_TEX2D_UAV::MipSlice
    ViewDesc.MipCount = 1;
  }
  auto View = new RSUnorderedAccessView(this);
  if (!View->createTextureView(Texture, ViewDesc, VK_IMAGE_USAGE_STORAGE_BIT)) {
    delete View;
    return nullptr;
  }
  return View;
}

RSUnorderedAccessView *
RSDevices::createUnorderedAccessView(RSBuffer *Buffer,
                                     const RS_BUFFER_VIEW_DESC &Desc) {
  if (!Buffer) {
    return nullptr;
  }
  auto View = new RSUnorderedAccessView(this);
  if (!View->createBufferView(Buffer, Desc) ||
      (Desc.WithCounter && !View->createCounter())) {
    delete View;
    return nullptr;
  }
  return View;
}

RSRenderTargetView *
RSDevices::createRenderTargetView(RSTexture *Texture,
                                  const RS_TEXTURE_VIEW_DESC *Desc) {
  if (!Texture) {
    return nullptr;
  }
  RS_TEXTURE_VIEW_DESC ViewDesc = Desc ? *Desc : RS_TEXTURE_VIEW_DESC{};
  if (!Desc) {
    ViewDesc.MipCount = 1;
    ViewDesc.LayerCount = 1;
  }
  auto View = new RSRenderTargetView(this);
  if (!View->createTextureView(Texture, ViewDesc,
                               VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
    delete View;
    return nullptr;
  }
  return View;
}

RSDepthStencilView *
RSDevices::createDepthStencilView(RSTexture *Texture,
                                  const RS_TEXTURE_VIEW_DESC *Desc) {
  if (!Texture) {
    return nullptr;
  }
  RS_TEXTURE_VIEW_DESC ViewDesc = Desc ? *Desc : RS_TEXTURE_VIEW_DESC{};
  if (!Desc) {
    ViewDesc.MipCount = 1;
    ViewDesc.LayerCount = 1;
  }
  auto View = new RSDepthStencilView(this);
  if (!View->createTextureView(Texture, ViewDesc,
                               VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
    delete View;
    return nullptr;
  }
  return View;
}

RSSampler *RSDevices::createSampler(const VkSamplerCreateInfo &Info) {
  VkSamplerCreateInfo Fixed = Info;
  VkPhysicalDeviceFeatures Features = {};
  vkGetPhysicalDeviceFeatures(PhysicalDevice, &Features);
  if (!Features.samplerAnisotropy) {
    Fixed.anisotropyEnable = VK_FALSE;
  }
  Fixed.maxAnisotropy =
      std::min(Fixed.maxAnisotropy,
               PhysicalDeviceProperties.limits.maxSamplerAnisotropy);
  auto Sampler = new RSSampler(this);
  if (!Sampler->create(Fixed)) {
    delete Sampler;
    return nullptr;
  }
  return Sampler;
}

RSShader *RSDevices::createShader(std::vector<uint32_t> &&Spirv) {
  auto Shader = new RSShader(this);
  if (!Shader->create(std::move(Spirv))) {
    delete Shader;
    return nullptr;
  }
  return Shader;
}

RSPipelineState *
RSDevices::createGraphicsPipeline(const RS_GRAPHICS_PIPELINE_DESC &Desc) {
  auto Pipeline = new RSPipelineState(this);
  if (!Pipeline->createGraphics(Desc)) {
    delete Pipeline;
    return nullptr;
  }
  return Pipeline;
}

RSPipelineState *RSDevices::createComputePipeline(RSShader *ComputeShader) {
  auto Pipeline = new RSPipelineState(this);
  if (!Pipeline->createCompute(ComputeShader)) {
    delete Pipeline;
    return nullptr;
  }
  return Pipeline;
}

//---------------------------------------------------------------
// upload
//---------------------------------------------------------------

VkCommandBuffer RSDevices::beginOneTimeCommand() {
  VkCommandBufferAllocateInfo AllocInfo = {};
  AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  AllocInfo.commandPool = UploadCommandPool;
  AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  AllocInfo.commandBufferCount = 1;
  VkCommandBuffer Cmd = VK_NULL_HANDLE;
  if (vkAllocateCommandBuffers(Device, &AllocInfo, &Cmd) != VK_SUCCESS) {
    return VK_NULL_HANDLE;
  }

  VkCommandBufferBeginInfo BeginInfo = {};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(Cmd, &BeginInfo);
  return Cmd;
}

bool RSDevices::endOneTimeCommand(VkCommandBuffer Cmd) {
  // make everything written by the upload visible to later submissions
  VkMemoryBarrier2 Barrier = {};
  Barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
  Barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  Barrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
  Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  Barrier.dstAccessMask =
      VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
  VkDependencyInfo Dependency = {};
  Dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  Dependency.memoryBarrierCount = 1;
  Dependency.pMemoryBarriers = &Barrier;
  vkCmdPipelineBarrier2(Cmd, &Dependency);
  vkEndCommandBuffer(Cmd);

  VkFenceCreateInfo FenceInfo = {};
  FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  VkFence Fence = VK_NULL_HANDLE;
  vkCreateFence(Device, &FenceInfo, nullptr, &Fence);

  VkCommandBufferSubmitInfo CmdInfo = {};
  CmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  CmdInfo.commandBuffer = Cmd;
  VkSubmitInfo2 Submit = {};
  Submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  Submit.commandBufferInfoCount = 1;
  Submit.pCommandBufferInfos = &CmdInfo;
  bool Result = submitToQueue(Submit, Fence);
  if (Result) {
    vkWaitForFences(Device, 1, &Fence, VK_TRUE, UINT64_MAX);
  }

  vkDestroyFence(Device, Fence, nullptr);
  vkFreeCommandBuffers(Device, UploadCommandPool, 1, &Cmd);
  return Result;
}

bool RSDevices::submitToQueue(const VkSubmitInfo2 &Submit, VkFence Fence) {
  std::lock_guard<std::mutex> Lock(QueueLock);
  return vkQueueSubmit2(Queue, 1, &Submit, Fence) == VK_SUCCESS;
}

bool RSDevices::uploadBuffer(RSBuffer *Buffer,
                             const void *Data,
                             VkDeviceSize Size) {
  std::lock_guard<std::mutex> Lock(UploadLock);

  VkBuffer Staging = VK_NULL_HANDLE;
  VmaAllocation StagingAlloc = nullptr;
  if (Data) {
    VkBufferCreateInfo Info = {};
    Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    Info.size = Size;
    Info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo AllocInfo = {};
    AllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    AllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                      VMA_ALLOCATION_CREATE_MAPPED_BIT;
    VmaAllocationInfo Mapped = {};
    if (vmaCreateBuffer(Allocator, &Info, &AllocInfo, &Staging, &StagingAlloc,
                        &Mapped) != VK_SUCCESS) {
      return false;
    }
    std::memcpy(Mapped.pMappedData, Data, static_cast<size_t>(Size));
    vmaFlushAllocation(Allocator, StagingAlloc, 0, VK_WHOLE_SIZE);
  }

  VkCommandBuffer Cmd = beginOneTimeCommand();
  if (Data) {
    VkBufferCopy Region = {0, 0, Size};
    vkCmdCopyBuffer(Cmd, Staging, Buffer->getHandle(), 1, &Region);
  } else {
    vkCmdFillBuffer(Cmd, Buffer->getHandle(), 0, VK_WHOLE_SIZE, 0);
  }
  bool Result = endOneTimeCommand(Cmd);

  if (Staging) {
    vmaDestroyBuffer(Allocator, Staging, StagingAlloc);
  }
  Buffer->GlobalState = {};

  return Result;
}

bool RSDevices::uploadTexture(RSTexture *Texture,
                              const RS_SUBRESOURCE_DATA *Data) {
  std::lock_guard<std::mutex> Lock(UploadLock);

  const auto &Desc = Texture->getDesc();
  RS_FORMAT_INFO Info = getFormatInfo(Desc.Format);

  // pack every subresource tightly into one staging buffer
  struct REGION {
    uint32_t Layer;
    uint32_t Mip;
    VkDeviceSize Offset;
  };
  std::vector<REGION> Regions = {};
  VkDeviceSize TotalSize = 0;
  for (uint32_t Layer = 0; Layer < Desc.ArrayLayers; Layer++) {
    for (uint32_t Mip = 0; Mip < Desc.MipLevels; Mip++) {
      uint32_t W = std::max(1u, Desc.Width >> Mip);
      uint32_t H = std::max(1u, Desc.Height >> Mip);
      uint32_t D = std::max(1u, Desc.Depth >> Mip);
      uint32_t BlocksX = (W + Info.BlockWidth - 1) / Info.BlockWidth;
      uint32_t BlocksY = (H + Info.BlockHeight - 1) / Info.BlockHeight;
      TotalSize = alignUp(TotalSize, 16);
      Regions.push_back({Layer, Mip, TotalSize});
      TotalSize += static_cast<VkDeviceSize>(BlocksX) * Info.BlockSize *
                   BlocksY * D;
    }
  }

  VkBufferCreateInfo BufInfo = {};
  BufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  BufInfo.size = TotalSize;
  BufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  VmaAllocationCreateInfo AllocInfo = {};
  AllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
  AllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                    VMA_ALLOCATION_CREATE_MAPPED_BIT;
  VkBuffer Staging = VK_NULL_HANDLE;
  VmaAllocation StagingAlloc = nullptr;
  VmaAllocationInfo Mapped = {};
  if (vmaCreateBuffer(Allocator, &BufInfo, &AllocInfo, &Staging, &StagingAlloc,
                      &Mapped) != VK_SUCCESS) {
    return false;
  }

  std::vector<VkBufferImageCopy> Copies = {};
  for (size_t I = 0; I < Regions.size(); I++) {
    const auto &Region = Regions[I];
    const auto &Sub = Data[I];
    uint32_t W = std::max(1u, Desc.Width >> Region.Mip);
    uint32_t H = std::max(1u, Desc.Height >> Region.Mip);
    uint32_t D = std::max(1u, Desc.Depth >> Region.Mip);
    uint32_t BlocksX = (W + Info.BlockWidth - 1) / Info.BlockWidth;
    uint32_t BlocksY = (H + Info.BlockHeight - 1) / Info.BlockHeight;
    size_t RowSize = static_cast<size_t>(BlocksX) * Info.BlockSize;
    size_t RowPitch = Sub.RowPitch ? Sub.RowPitch : RowSize;
    size_t SlicePitch = Sub.SlicePitch ? Sub.SlicePitch : RowPitch * BlocksY;
    auto Dst = static_cast<uint8_t *>(Mapped.pMappedData) + Region.Offset;
    auto Src = static_cast<const uint8_t *>(Sub.Data);
    for (uint32_t Z = 0; Z < D; Z++) {
      for (uint32_t Y = 0; Y < BlocksY; Y++) {
        std::memcpy(Dst + (static_cast<size_t>(Z) * BlocksY + Y) * RowSize,
                    Src + Z * SlicePitch + Y * RowPitch, RowSize);
      }
    }

    VkBufferImageCopy Copy = {};
    Copy.bufferOffset = Region.Offset;
    Copy.imageSubresource = {Texture->getAspect(), Region.Mip, Region.Layer, 1};
    Copy.imageExtent = {W, H, D};
    Copies.push_back(Copy);
  }
  vmaFlushAllocation(Allocator, StagingAlloc, 0, VK_WHOLE_SIZE);

  VkImageLayout Final = Texture->isStorage()
                            ? VK_IMAGE_LAYOUT_GENERAL
                            : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkCommandBuffer Cmd = beginOneTimeCommand();
  VkImageMemoryBarrier2 Barrier = {};
  Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  Barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  Barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  Barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  Barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  Barrier.image = Texture->getHandle();
  Barrier.subresourceRange = Texture->getFullRange();
  VkDependencyInfo Dependency = {};
  Dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  Dependency.imageMemoryBarrierCount = 1;
  Dependency.pImageMemoryBarriers = &Barrier;
  vkCmdPipelineBarrier2(Cmd, &Dependency);

  vkCmdCopyBufferToImage(Cmd, Staging, Texture->getHandle(),
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         static_cast<uint32_t>(Copies.size()), Copies.data());

  Barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  Barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  Barrier.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;
  Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  Barrier.newLayout = Final;
  vkCmdPipelineBarrier2(Cmd, &Dependency);

  bool Result = endOneTimeCommand(Cmd);
  vmaDestroyBuffer(Allocator, Staging, StagingAlloc);

  Texture->GlobalState = {};
  Texture->GlobalState.Layout = Final;

  return Result;
}

bool RSDevices::transitionNewTexture(RSTexture *Texture) {
  // new textures are cleared to zero like the direct3d runtime does
  std::lock_guard<std::mutex> Lock(UploadLock);

  VkCommandBuffer Cmd = beginOneTimeCommand();
  VkImageMemoryBarrier2 Barrier = {};
  Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
  Barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
  Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  Barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
  Barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  Barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
  Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  Barrier.image = Texture->getHandle();
  Barrier.subresourceRange = Texture->getFullRange();
  VkDependencyInfo Dependency = {};
  Dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
  Dependency.imageMemoryBarrierCount = 1;
  Dependency.pImageMemoryBarriers = &Barrier;
  vkCmdPipelineBarrier2(Cmd, &Dependency);

  auto Range = Texture->getFullRange();
  if (Texture->getAspect() & VK_IMAGE_ASPECT_DEPTH_BIT) {
    VkClearDepthStencilValue Value = {0.f, 0};
    vkCmdClearDepthStencilImage(Cmd, Texture->getHandle(),
                                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Value, 1,
                                &Range);
  } else {
    VkClearColorValue Value = {};
    vkCmdClearColorImage(Cmd, Texture->getHandle(),
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &Value, 1,
                         &Range);
  }

  bool Result = endOneTimeCommand(Cmd);
  Texture->GlobalState = {};
  Texture->GlobalState.Layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

  return Result;
}

//---------------------------------------------------------------
// frame
//---------------------------------------------------------------

uint32_t RSDevices::getFrameIndex() const {
  return static_cast<uint32_t>(FrameSerial % RS_FRAME_IN_FLIGHT);
}

void RSDevices::beginFrame() {
  if (FrameBegun) {
    return;
  }

  auto &Frame = Frames[getFrameIndex()];
  vkWaitForFences(Device, 1, &Frame.Fence, VK_TRUE, UINT64_MAX);
  CompletedSerial = std::max(CompletedSerial, Frame.Serial);
  collectGarbage(CompletedSerial);

  {
    std::lock_guard<std::mutex> Lock(RingLock);
    Frame.RingBlockIndex = 0;
    Frame.RingOffset = 0;
  }
  Frame.Serial = FrameSerial;
  SubmittedContexts.clear();
  FrameBegun = true;
}

void RSDevices::submitContext(RSCommandContext *Context) {
  SubmittedContexts.push_back(Context);
}

void RSDevices::recordTransition(VkCommandBuffer Cmd,
                                 RSTrackedResource *Resource,
                                 const RS_TRACKED_STATE &From,
                                 const RS_ACCESS &To,
                                 const VkImageSubresourceRange *Range) {
  // the destination is every later command, contexts rely on this
  VkPipelineStageFlags2 SrcStages = From.WriteStages | From.ReadStages;
  if (Resource->isImage()) {
    auto Texture = static_cast<RSTexture *>(Resource);
    VkImageMemoryBarrier2 Barrier = {};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    Barrier.srcStageMask = SrcStages;
    Barrier.srcAccessMask = From.WriteAccess;
    Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    Barrier.dstAccessMask =
        VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    Barrier.oldLayout = From.Layout;
    Barrier.newLayout = To.Layout;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.image = Texture->getHandle();
    Barrier.subresourceRange = Range ? *Range : Texture->getFullRange();
    VkDependencyInfo Dependency = {};
    Dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    Dependency.imageMemoryBarrierCount = 1;
    Dependency.pImageMemoryBarriers = &Barrier;
    vkCmdPipelineBarrier2(Cmd, &Dependency);
  } else {
    auto Buffer = static_cast<RSBuffer *>(Resource);
    VkBufferMemoryBarrier2 Barrier = {};
    Barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    Barrier.srcStageMask = SrcStages;
    Barrier.srcAccessMask = From.WriteAccess;
    Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    Barrier.dstAccessMask =
        VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.buffer = Buffer->getHandle();
    Barrier.offset = 0;
    Barrier.size = VK_WHOLE_SIZE;
    VkDependencyInfo Dependency = {};
    Dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    Dependency.bufferMemoryBarrierCount = 1;
    Dependency.pBufferMemoryBarriers = &Barrier;
    vkCmdPipelineBarrier2(Cmd, &Dependency);
  }
}

void RSDevices::recordFinalCommand(VkCommandBuffer Cmd, uint32_t ImageIndex) {
  VkCommandBufferBeginInfo BeginInfo = {};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  vkBeginCommandBuffer(Cmd, &BeginInfo);

  if (ImageIndex != UINT32_MAX) {
    auto SrcAccess = rs_access::transferSrc(BackBufferTexture);
    recordTransition(Cmd, BackBufferTexture, BackBufferTexture->GlobalState,
                     SrcAccess);
    BackBufferTexture->GlobalState = {};
    BackBufferTexture->GlobalState.Layout = SrcAccess.Layout;
    BackBufferTexture->GlobalState.ReadStages =
        VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    BackBufferTexture->GlobalState.ReadAccess = VK_ACCESS_2_TRANSFER_READ_BIT;

    VkImage Image = SwapChainImages[ImageIndex];
    VkImageMemoryBarrier2 Barrier = {};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    Barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    Barrier.srcAccessMask = VK_ACCESS_2_NONE;
    Barrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    Barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    Barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    Barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.image = Image;
    Barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    VkDependencyInfo Dependency = {};
    Dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    Dependency.imageMemoryBarrierCount = 1;
    Dependency.pImageMemoryBarriers = &Barrier;
    vkCmdPipelineBarrier2(Cmd, &Dependency);

    // stretch the fixed size back buffer to the window like dxgi does
    VkImageBlit Blit = {};
    Blit.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    Blit.srcOffsets[1] = {static_cast<int32_t>(WndWidth),
                          static_cast<int32_t>(WndHeight), 1};
    Blit.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    Blit.dstOffsets[1] = {static_cast<int32_t>(SwapChainExtent.width),
                          static_cast<int32_t>(SwapChainExtent.height), 1};
    vkCmdBlitImage(Cmd, BackBufferTexture->getHandle(), SrcAccess.Layout, Image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Blit,
                   VK_FILTER_LINEAR);

    Barrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
    Barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
    Barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
    Barrier.dstAccessMask = VK_ACCESS_2_NONE;
    Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    Barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkCmdPipelineBarrier2(Cmd, &Dependency);
  }

  vkEndCommandBuffer(Cmd);
}

void RSDevices::presentSwapChain() {
  beginFrame();
  auto &Frame = Frames[getFrameIndex()];

  if (SwapChainDirty) {
    vkDeviceWaitIdle(Device);
    destroySwapChain();
    createSwapChain();
  }

  uint32_t ImageIndex = UINT32_MAX;
  if (SwapChain) {
    VkResult Result =
        vkAcquireNextImageKHR(Device, SwapChain, UINT64_MAX,
                              Frame.ImageAcquired, VK_NULL_HANDLE, &ImageIndex);
    if (Result == VK_ERROR_OUT_OF_DATE_KHR) {
      SwapChainDirty = true;
      ImageIndex = UINT32_MAX;
    } else if (Result == VK_SUBOPTIMAL_KHR) {
      SwapChainDirty = true;
    } else if (Result != VK_SUCCESS) {
      ImageIndex = UINT32_MAX;
    }
  }

  std::vector<VkCommandBufferSubmitInfo> Commands = {};
  auto PushCommand = [&Commands](VkCommandBuffer Cmd) {
    VkCommandBufferSubmitInfo Info = {};
    Info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    Info.commandBuffer = Cmd;
    Commands.push_back(Info);
  };
  for (auto Context : SubmittedContexts) {
    if (auto Glue = Context->recordGlue()) {
      PushCommand(Glue);
    }
    PushCommand(Context->getCommandBuffer());
  }

  vkResetCommandPool(Device, Frame.CommandPool, 0);
  recordFinalCommand(Frame.FinalCommand, ImageIndex);
  PushCommand(Frame.FinalCommand);

  VkSemaphoreSubmitInfo WaitInfo = {};
  WaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  WaitInfo.semaphore = Frame.ImageAcquired;
  WaitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT;
  VkSemaphoreSubmitInfo SignalInfo = {};
  SignalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
  SignalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
  bool Presenting = ImageIndex != UINT32_MAX;
  if (Presenting) {
    SignalInfo.semaphore = RenderFinishedSemaphores[ImageIndex];
  }

  VkSubmitInfo2 Submit = {};
  Submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
  Submit.waitSemaphoreInfoCount = Presenting ? 1 : 0;
  Submit.pWaitSemaphoreInfos = &WaitInfo;
  Submit.commandBufferInfoCount = static_cast<uint32_t>(Commands.size());
  Submit.pCommandBufferInfos = Commands.data();
  Submit.signalSemaphoreInfoCount = Presenting ? 1 : 0;
  Submit.pSignalSemaphoreInfos = &SignalInfo;

  vkResetFences(Device, 1, &Frame.Fence);
  if (!submitToQueue(Submit, Frame.Fence)) {
    assert(false && "failed to submit frame");
  }

  if (Presenting) {
    VkPresentInfoKHR PresentInfo = {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pWaitSemaphores = &RenderFinishedSemaphores[ImageIndex];
    PresentInfo.swapchainCount = 1;
    PresentInfo.pSwapchains = &SwapChain;
    PresentInfo.pImageIndices = &ImageIndex;
    VkResult Result = VK_SUCCESS;
    {
      std::lock_guard<std::mutex> Lock(QueueLock);
      Result = vkQueuePresentKHR(Queue, &PresentInfo);
    }
    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR) {
      SwapChainDirty = true;
    }
  }

  SubmittedContexts.clear();
  ++FrameSerial;
  FrameBegun = false;
}

void RSDevices::waitIdle() {
  if (Device) {
    vkDeviceWaitIdle(Device);
  }
}

RS_DYNAMIC_ALLOCATION RSDevices::allocateDynamic(VkDeviceSize Size,
                                                 VkDeviceSize Alignment) {
  std::lock_guard<std::mutex> Lock(RingLock);
  auto &Frame = Frames[getFrameIndex()];
  Alignment = std::max<VkDeviceSize>(
      {Alignment, PhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment,
       PhysicalDeviceProperties.limits.minStorageBufferOffsetAlignment, 16});

  while (true) {
    if (Frame.RingBlockIndex < Frame.RingBlocks.size()) {
      auto &Block = Frame.RingBlocks[Frame.RingBlockIndex];
      VkDeviceSize Offset = alignUp(Frame.RingOffset, Alignment);
      if (Offset + Size <= Block.Size) {
        Frame.RingOffset = Offset + Size;
        return {Block.Buffer, Offset, Block.CpuAddress + Offset};
      }
      ++Frame.RingBlockIndex;
      Frame.RingOffset = 0;
      continue;
    }

    FRAME_RESOURCE::RING_BLOCK Block = {};
    Block.Size = std::max(RING_BLOCK_SIZE, Size);
    VkBufferCreateInfo Info = {};
    Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    Info.size = Block.Size;
    Info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                 VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                 VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VmaAllocationCreateInfo AllocInfo = {};
    AllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    AllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    AllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (vmaCreateBuffer(Allocator, &Info, &AllocInfo, &Block.Buffer,
                        &Block.Allocation, nullptr) != VK_SUCCESS) {
      assert(false && "failed to create dynamic ring buffer");
      return {};
    }
    void *Ptr = nullptr;
    vmaMapMemory(Allocator, Block.Allocation, &Ptr);
    Block.CpuAddress = static_cast<uint8_t *>(Ptr);
    Frame.RingBlocks.push_back(Block);
  }
}

//---------------------------------------------------------------
// deferred destruction
//---------------------------------------------------------------

void RSDevices::deferDestroy(RSVkObject *Object) {
  if (ShuttingDown) {
    delete Object;
    return;
  }
  std::lock_guard<std::mutex> Lock(DestroyLock);
  DestroyQueue.push_back({Object, FrameSerial});
}

void RSDevices::collectGarbage(uint64_t SafeSerial) {
  std::vector<RSVkObject *> ToDelete = {};
  {
    std::lock_guard<std::mutex> Lock(DestroyLock);
    auto It = std::stable_partition(
        DestroyQueue.begin(), DestroyQueue.end(),
        [SafeSerial](const std::pair<RSVkObject *, uint64_t> &Entry) {
          return Entry.second > SafeSerial;
        });
    for (auto I = It; I != DestroyQueue.end(); I++) {
      ToDelete.push_back(I->first);
    }
    DestroyQueue.erase(It, DestroyQueue.end());
  }
  // destructors may release children, which come back to the queue
  for (auto Object : ToDelete) {
    delete Object;
  }
  if (ShuttingDown) {
    std::lock_guard<std::mutex> Lock(DestroyLock);
    if (!DestroyQueue.empty()) {
      auto Remain = DestroyQueue;
      DestroyQueue.clear();
      for (auto &Entry : Remain) {
        delete Entry.first;
      }
    }
  }
}

//---------------------------------------------------------------
// dummy resources
//---------------------------------------------------------------

RSResourceView *RSDevices::getDummyView(VkDescriptorType Type,
                                        rs_tool::SPIRV_IMAGE_DIM Dim,
                                        bool Arrayed,
                                        rs_tool::SPIRV_SCALAR_KIND Scalar) {
  uint64_t Key = (static_cast<uint64_t>(Type) << 32) |
                 (static_cast<uint64_t>(Dim) << 16) |
                 (static_cast<uint64_t>(Arrayed) << 8) |
                 static_cast<uint64_t>(Scalar);
  std::lock_guard<std::mutex> Lock(DummyLock);
  auto Found = DummyViews.find(Key);
  if (Found != DummyViews.end()) {
    return Found->second;
  }

  RSResourceView *View = nullptr;
  if (Dim == rs_tool::SPIRV_IMAGE_DIM::BUFFER) {
    RS_BUFFER_VIEW_DESC Desc = {};
    switch (Scalar) {
    case rs_tool::SPIRV_SCALAR_KIND::UINT:
      Desc.Format = VK_FORMAT_R32_UINT;
      break;
    case rs_tool::SPIRV_SCALAR_KIND::SINT:
      Desc.Format = VK_FORMAT_R32_SINT;
      break;
    default:
      Desc.Format = VK_FORMAT_R32G32B32A32_SFLOAT;
      break;
    }
    Desc.NumElements = 1;
    View = createShaderResourceView(DummyBuffer, Desc);
  } else {
    RS_TEXTURE_DESC Desc = {};
    Desc.Usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    switch (Scalar) {
    case rs_tool::SPIRV_SCALAR_KIND::UINT:
      Desc.Format = VK_FORMAT_R32G32B32A32_UINT;
      break;
    case rs_tool::SPIRV_SCALAR_KIND::SINT:
      Desc.Format = VK_FORMAT_R32G32B32A32_SINT;
      break;
    default:
      Desc.Format = VK_FORMAT_R16G16B16A16_SFLOAT;
      break;
    }
    RS_TEXTURE_VIEW_DESC ViewDesc = {};
    switch (Dim) {
    case rs_tool::SPIRV_IMAGE_DIM::TEX1D:
      Desc.Type = VK_IMAGE_TYPE_1D;
      ViewDesc.ViewType =
          Arrayed ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
      break;
    case rs_tool::SPIRV_IMAGE_DIM::TEX3D:
      Desc.Type = VK_IMAGE_TYPE_3D;
      ViewDesc.ViewType = VK_IMAGE_VIEW_TYPE_3D;
      break;
    case rs_tool::SPIRV_IMAGE_DIM::CUBE:
      Desc.ArrayLayers = 6;
      Desc.CubeCompatible = true;
      Desc.Usage = VK_IMAGE_USAGE_SAMPLED_BIT;
      ViewDesc.ViewType =
          Arrayed ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
      break;
    default:
      ViewDesc.ViewType =
          Arrayed ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
      break;
    }
    auto Texture = createTexture(Desc);
    if (Texture) {
      // dummy images are never tracked, keep them in the layout used by
      // their descriptors
      std::lock_guard<std::mutex> UploadGuard(UploadLock);
      VkCommandBuffer Cmd = beginOneTimeCommand();
      recordTransition(Cmd, Texture, Texture->GlobalState,
                       {Texture->isStorage()
                            ? VK_IMAGE_LAYOUT_GENERAL
                            : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
      endOneTimeCommand(Cmd);
      Texture->GlobalState.Layout = Texture->isStorage()
                                        ? VK_IMAGE_LAYOUT_GENERAL
                                        : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      if (Type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
        View = createUnorderedAccessView(Texture, &ViewDesc);
      } else {
        View = createShaderResourceView(Texture, &ViewDesc);
      }
      Texture->Release();
    }
  }

  assert(View && "failed to create dummy view");
  DummyViews.insert({Key, View});
  return View;
}

//---------------------------------------------------------------
// misc
//---------------------------------------------------------------

void RSDevices::copyHighDynamicTexture(RSCommandContext *Context,
                                       RSTexture *DstResource) {
  Context->copyTexture(DstResource, HighDynamicTexture);
}

bool RSDevices::getConcurrentCreateSupport() const {
  return ConcurrentCreateSupportFlag;
}

bool RSDevices::getCommandListSupport() const {
  return CommandListSupportFlag && (!RenderDeivceConfig.ForceSingleThreadFlag);
}

UINT RSDevices::getCurrWndWidth() const { return WndWidth; }

UINT RSDevices::getCurrWndHeight() const { return WndHeight; }
