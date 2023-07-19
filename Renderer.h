#pragma once

#include <cstddef>
#include <glm/fwd.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <vulkan/vulkan_core.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <Mesh.h>

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

enum eMemoryType
{
  eTransferMemory = 1,
  eTextureMemory = 2,
  eBufferMemory = 3,
  eMeshMemory = 4,
  eHostMemory = 5
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

  std::vector<Header*> Allocations;

  VkDeviceSize Available;
  VkDeviceSize Used;
  VkDeviceSize Total;
};

class Camera
{
public:
  Camera(Renderer* R) : pRenderer(R) {}

  union{
    glm::mat4 World;
    glm::mat4 View;
    glm::mat4 Proj;
  } CamDat;

  void Update();
  void PollInputs();

  glm::vec3 Position = glm::vec3(0.f, 0.f, 10.f); // -z is forwards
  glm::vec3 Rotation;
  glm::mat4 CameraMat; // the camera matrix is the inverse of the view matrix, instead of transforming other things to it, it transforms itself relative to other things, therefore moving itself through space

  double LastMousePos[2];

  Buffer CameraBuffer;
  void* BufferMemory;
  VkDescriptorSet CamDescriptor;

  Renderer* pRenderer;

  float Sensitivity;
};

typedef uint32_t EkTexture;
typedef uint32_t EkBuffer;

class TextureStorage
{
public:
  Texture* GetTexture(EkTexture Texture);
private:
  std::map<EkTexture, Texture> TextureMap;
};

class BufferStorage
{
public:
  Buffer* GetBuffer(EkBuffer Buffer);
private:
  std::map<EkBuffer, Buffer> BufferMap;
};

class Renderer
{
  friend Camera;
public:
  CommandDispatch GraphicsDispatch;
  CommandDispatch ComputeDispatch;

  VkDevice Device;

  Renderer() : PrimCamera(this)
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

  // brief: Don't call this before CreateRenderPass()
  Camera* GetCamera();
  bool CreateDevice();

  void CreateCmdBuffers();
  void CreateSwapchain(VkPresentModeKHR PresentMode);
  void CreateRenderPass();
  void CreateFrameBuffers();
  Material CreateMat();
  Shader CreateShader(const char* ShaderPath, const char* EntryPoint);
  Pipeline CreatePipeline(Material Mat);

  void CreateImage(Texture* Image, VkFormat Format, VkImageUsageFlags Usage, eMemoryType Memory = eTextureMemory);
  void CreateImageView(VkImageView* ImageView, VkImage* Image, VkFormat Format, VkImageViewType ViewType, VkImageAspectFlags AspectMask);

private:
  void GetMemoryIndices();
  void MakeSceneDescriptorPool();

  Camera PrimCamera;
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

  VkDescriptorPool ScenePool;

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
};

