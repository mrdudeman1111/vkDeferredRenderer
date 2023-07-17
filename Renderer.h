#include <cstddef>
#include <iostream>
#include <vector>
#include <thread>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

// 4 DescSets at a time

// Parralel matrix computation when computing bone spaces per frmae
class Renderer;
class CommandDispatch;
struct Texture;
struct Buffer;

enum ePoolType
{
  eTypeGraphics,
  eTypeCompute
};

// Linux Implementation
class CommandDispatch
{
  friend Renderer;
public:
  // The Command buffer returned from this function is already recording, when done just submit the buffer and the Renderer will stop the recording and submit it down the queue. (don't submit a command buffer to a queue that it wasn't allocated from, you can also just call vkEndCommandBuffer())
  VkCommandBuffer* GetCommandBuffer(VkDevice* pDevice);

  // This function takes in one VkCommandBuffer* from GetCommandBuffer, ends it, and then submits it.
  bool SubmitCommandBuffer(VkCommandBuffer* CommandBuffer, VkSemaphore* pSemaphores = nullptr, uint32_t SemaphoreCount = 0, VkPipelineStageFlags WaitStage = 0);
private:
  VkQueue Queue;
  VkCommandPool Pool;
  uint8_t PoolFlags;
};

struct MemoryIndices
{
public:
  // Device local memory
  uint32_t VRamHeap = -1;

  // Host local memory
  uint32_t DRamHeap = -1;

  uint32_t TransferHeap = -1;

  uint64_t VRamHeapSizeTotal = -1;
  uint64_t DRamHeapSizeTotal = -1;
  uint64_t TransferSizeTotal = -1;

  uint32_t VRamIndex = -1;
  uint32_t DRamIndex = -1;
  uint32_t TransferIndex = -1;
};

struct MemoryBlock
{
public:
  VkDeviceMemory Memory;

  VkDeviceSize Available;
  VkDeviceSize Used;
  VkDeviceSize Total;
};

struct Texture
{
  public:
  VkImage Image;

  uint32_t MemorySize;
  uint32_t Offset;
};

struct Buffer
{
public:
  VkBuffer Buffer;

  uint32_t MemorySize;
  uint32_t Offset;
};

class Renderer
{
public:
  CommandDispatch GraphicsDispatch;
  CommandDispatch ComputeDispatch;

  VkDevice Device;

  Renderer()
  {

  }
  ~Renderer()
  {

  }

  bool RequestInstLayer(const char* LayerName);
  bool RequestInstExt(const char* ExtName);

  bool RequestDevLayer(const char* LayerName);
  bool RequestDevExt(const char* ExtName);

  void Init(uint32_t inWidth, uint32_t inHeight);
  bool CreateDevice();

  void CreateCmdBuffers();
  void CreateSwapchain(VkPresentModeKHR PresentMode);
  void CreateRenderPass();
  void CreateFrameBuffers();




  void CreateImage(Texture* Image, VkFormat Format, VkImageUsageFlags Usage);
  void CreateImageView(VkImageView* ImageView, VkImage* Image, VkFormat Format, VkImageViewType ViewType, VkImageAspectFlags AspectMask);

  void CreateDescriptorSet(VkDescriptorSetLayout* Layouts, uint32_t LayoutCount);
  void DestroyDescriptorSet(VkDescriptorSet* DescriptorSet);
private:
  void GetMemoryIndices();

  uint32_t Width;
  uint32_t Height;

  std::vector<const char*> InstanceLayers;
  std::vector<const char*> InstanceExtensions;
  VkInstance Instance;

  uint32_t GraphicsFamily;
  uint32_t ComputeFamily;
  VkPhysicalDevice PhysicalDevice;

  std::vector<const char*> DeviceLayers;
  std::vector<const char*> DeviceExtensions;

  // Memory stuff
    MemoryIndices MemoryInfo;

    MemoryBlock TransferMemory;
    MemoryBlock TextureMemory;
    MemoryBlock BufferMemory;
    MemoryBlock MeshMemory;
    MemoryBlock HostMemory;

  VkSurfaceFormatKHR SurfaceFormat;
  VkSurfaceKHR Surface;
  VkSwapchainKHR Swapchain;

  std::vector<VkImage> SwapchainImages; // color attachment for framebuffers
  std::vector<VkImageView> SwapchainImageViews;
  std::vector<Texture> PosImages;
  std::vector<VkImageView> PosImageViews;
  std::vector<Texture> DepthImages;
  std::vector<VkImageView> DepthImageViews;
  std::vector<Texture> NormImages;
  std::vector<VkImageView> NormImageViews;
  std::vector<VkFramebuffer> FrameBuffers;

  VkSubpassDescription PrimPass;
  VkSubpassDescription LightingPass;
  VkRenderPass Renderpass;

  VkFence Fence;
  VkSemaphore Semaphore;

  GLFWwindow* Window;

  std::vector<Texture> Textures;
};

