#include <iostream>
#include <vector>
#include <thread>

#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

// Parralel matrix computation when computing bone spaces per frmae
class Renderer;
class CommandDispatch;

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
  // The Command buffed returned from this function is already recording, when done just submit the buffer and the Renderer will stop the recording and submit it down the queue. (don't submit a command buffer to a queue that it wasn't allocated from)
  VkCommandBuffer* GetCommandBuffer(VkDevice* pDevice);

  // This function takes in one VkCommandBuffer* from GetCommandBuffer, ends it, and then submits it.
  bool SubmitCommandBuffer(VkCommandBuffer* CommandBuffer, VkSemaphore* pSemaphores = nullptr, uint32_t SemaphoreCount = 0, VkPipelineStageFlags WaitStage = 0);
private:
  VkQueue Queue;
  VkCommandPool Pool;
  uint8_t PoolFlags;
};

class Renderer
{
public:
  CommandDispatch GraphicsDispatch;
  CommandDispatch ComputeDispatch;

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

  void Init();
  bool CreateDevice();

  void CreateCmdBuffers();
  void CreateSwapchain(VkPresentModeKHR PresentMode);
  void CreateAttachments();
  void CreateRenderPass();
  void CreateFrameBuffers();
private:
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
  VkDevice Device;

  VkSurfaceFormatKHR SurfaceFormat;
  VkSurfaceKHR Surface;
  VkSwapchainKHR Swapchain;
  std::vector<VkImage> SwapchainImages; // color attachment for framebuffers
  std::vector<VkFramebuffer> FrameBuffers;

  VkSubpassDescription PrimPass;
  VkSubpassDescription LightingPass;
  VkRenderPass Renderpass;

  GLFWwindow* Window;
};

