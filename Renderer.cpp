#include <Renderer.h>

#include <cstdint>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtx/transform.hpp>
#include <ios>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>

#include <variant>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

#include <glm/mat4x4.hpp>
#include <glm/gtx/projection.hpp>
#include <glm/ext/matrix_transform.hpp>

// VkImage
// VkFormat
// VkLayout
// VkDescriptorSetLayout
// VkDescriptorSet

/*
  Transfer Queues!!!!!!!!!!!
*/

// I've chosen a stack allocator for memory management

std::vector<char> ReadFile(const std::string& FileName)
{
  std::ifstream File(FileName, std::ios::ate | std::ios::binary);

  if(!File.is_open())
  {
    throw std::runtime_error("Failed to open file");
  }

  size_t FileSize = File.tellg();
  std::vector<char> Buffer(FileSize);

  File.seekg(0);
  File.read(Buffer.data(), static_cast<std::streamsize>(FileSize));

  File.close();

  return Buffer;
}

void Camera::PollInputs()
{
  CamDat.World = glm::mat4(1.f);
  CamDat.Proj = glm::perspective(glm::radians(45.f), (float)(pRenderer->Width/pRenderer->Height), 0.1f, 1000.f);

  {
    double X, Y;
    glfwGetCursorPos(pRenderer->Window, &X, &Y);

    double DeltaX, DeltaY;

    DeltaX = LastMousePos[0] - X;
    DeltaY = LastMousePos[1] - Y;

    LastMousePos[0] = X;
    LastMousePos[1] = Y;

    Rotation.x += (DeltaX * Sensitivity);
    Rotation.y += (DeltaY * Sensitivity);

    std::clamp(Rotation.y, -90.f, 90.f);

    glm::vec3 Up = glm::vec3(0.f, 1.f, 0.f);
    CameraMat = glm::rotate(CameraMat, Rotation.x, Up);

    glm::vec3 Right = glm::vec3(CameraMat[0][0], CameraMat[1][0], CameraMat[2][0]);
    CameraMat = glm::rotate(CameraMat, Rotation.y, Right);

    glm::vec3 Forward = glm::vec3(CameraMat[0][2], CameraMat[1][2], CameraMat[2][2]);
    Forward *= -1.f; // forward direction is always z*-1
    CameraMat = glm::rotate(CameraMat, Rotation.z, Forward);

    CameraMat = glm::translate(CameraMat, Position);
  }

  CamDat.View = glm::inverse(CameraMat);
}

void Camera::Update()
{
  VkResult Res;

  if(CameraBuffer.Buffer == VK_NULL_HANDLE)
  {
    VkBufferCreateInfo BufferCI{};
    BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCI.size = (sizeof(glm::mat4) * 3);
    BufferCI.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    BufferCI.sharingMode = VK_SHARING_MODE_CONCURRENT;

    if((Res = vkCreateBuffer(pRenderer->Device, &BufferCI, nullptr, &CameraBuffer.Buffer)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create camera uniform buffer with error " + std::to_string(Res));
    }

    VkMemoryRequirements Req;
    vkGetBufferMemoryRequirements(pRenderer->Device, CameraBuffer.Buffer, &Req);

    uint32_t Alignment = 0;

    for(uint32_t i = 0; Alignment < pRenderer->HostMemory.Used; i++)
    {
      Alignment = Req.alignment * i;
    }

    CameraBuffer.Allocation.AllocSize = sizeof(glm::mat4) * 3;
    CameraBuffer.Allocation.Offset = Alignment;

    if((Res = vkBindBufferMemory(pRenderer->Device, CameraBuffer.Buffer, pRenderer->HostMemory.Memory, Alignment)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to bind buffer to memory at " + std::to_string(Alignment) + ", might be out of memory, error: " + std::to_string(Res));
    }

    vkMapMemory(pRenderer->Device, pRenderer->HostMemory.Memory, CameraBuffer.Allocation.Offset, CameraBuffer.Allocation.AllocSize, 0, &BufferMemory);
  }

  PollInputs();

  memcpy(BufferMemory, &CamDat, sizeof(CamDat));

  VkDescriptorBufferInfo BuffInfo{};
  BuffInfo.range = sizeof(glm::vec3) * 3;
  BuffInfo.buffer = CameraBuffer.Buffer;
  // this is an offset relative to the start of the Buffer, not the VkDeviceMemory that we've put the buffer in.
  BuffInfo.offset = 0;

  VkWriteDescriptorSet Write{};
  Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  Write.descriptorCount = 1;
  Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  Write.dstSet = CamDescriptor;
  Write.dstBinding = 0;
  Write.dstArrayElement = 0;
  Write.pBufferInfo = &BuffInfo;

  vkUpdateDescriptorSets(pRenderer->Device, 1, &Write, 0, nullptr);
}

/*        Helpers         */
Buffer* BufferStorage::GetBuffer(EkBuffer Buffer)
{
  if(BufferMap.count(Buffer))
  {
    return &BufferMap[Buffer];
  }

  return nullptr;
}

Texture* TextureStorage::GetTexture(EkTexture Texture)
{
  if(TextureMap.count(Texture))
  {
    return &TextureMap[Texture];
  }

  return nullptr;
}

void Renderer::CreateImage(Texture* Image, VkFormat Format, VkImageUsageFlags Usage, eMemoryType Memory)
{
  VkResult Res;

  VkImageCreateInfo ImageCI{};
  ImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  ImageCI.usage = Usage;
  ImageCI.extent = VkExtent3D{Width, Height, 1};
  ImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
  ImageCI.format = Format;
  ImageCI.imageType = VK_IMAGE_TYPE_2D;
  ImageCI.samples = VK_SAMPLE_COUNT_1_BIT;
  ImageCI.mipLevels = 1;
  ImageCI.arrayLayers = 1;
  ImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  ImageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  if((Res = vkCreateImage(Device, &ImageCI, nullptr, &Image->Image)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image with error: " + std::to_string(Res));
  }

  VkMemoryRequirements MemReq;
  vkGetImageMemoryRequirements(Device, Image->Image, &MemReq);

  MemoryBlock* MemoryTarget;

  switch(Memory)
  {
    case eTextureMemory:
      MemoryTarget = &TextureMemory;
      break;
    case eTransferMemory:
      MemoryTarget = &TransferMemory;
      break;
    case eHostMemory:
      MemoryTarget = &HostMemory;
      break;
    case eMeshMemory:
      MemoryTarget = &MeshMemory;
      break;
    case eBufferMemory:
      MemoryTarget = &BufferMemory;
      break;
    default:
      MemoryTarget = &TextureMemory;
      break;
  }

  uint32_t Alignment = 0;

  for(uint32_t i = 0; Alignment == 0; i++)
  {
    if((MemReq.alignment*i) >= MemoryTarget->Used)
    {
      Alignment = MemReq.alignment*i;
    }
  }

  vkBindImageMemory(Device, Image->Image, TextureMemory.Memory, VkDeviceSize(Alignment));

  Image->Allocation.Offset = Alignment;
  Image->Allocation.AllocSize = MemReq.size;

  TextureMemory.Available -= VkDeviceSize(Alignment);
  TextureMemory.Used += VkDeviceSize(Alignment);
}

void Renderer::CreateImageView(VkImageView* ImageView, VkImage* Image, VkFormat Format, VkImageViewType ViewType, VkImageAspectFlags AspectMask)
{
  VkResult Res;

  VkImageViewCreateInfo ViewCI{};
  ViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  ViewCI.format = Format;
  ViewCI.image = *Image;
  ViewCI.viewType = ViewType;
  ViewCI.components.r = VK_COMPONENT_SWIZZLE_R;
  ViewCI.components.g = VK_COMPONENT_SWIZZLE_G;
  ViewCI.components.b = VK_COMPONENT_SWIZZLE_B;
  ViewCI.components.a = VK_COMPONENT_SWIZZLE_A;
  ViewCI.subresourceRange.aspectMask = AspectMask;
  ViewCI.subresourceRange.baseMipLevel = 0;
  ViewCI.subresourceRange.baseArrayLayer = 0;
  ViewCI.subresourceRange.layerCount = 1;
  ViewCI.subresourceRange.levelCount = 1;

  if((Res = vkCreateImageView(Device, &ViewCI, nullptr, ImageView)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create image view with error: " + std::to_string(Res));
  }
}

void Renderer::GetMemoryIndices()
{
  VkPhysicalDeviceMemoryProperties MemProps;
  vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProps);

  uint64_t BiggestDeviceHeap = 0;
  uint64_t BiggestHostHeap = 0;

  for(uint32_t i = 0; i < MemProps.memoryHeapCount; i++)
  {
    if(MemProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT && MemProps.memoryHeaps[i].size > BiggestDeviceHeap)
    {
      MemoryInfo.VRamHeap = i;
      BiggestDeviceHeap = MemProps.memoryHeaps[i].size;
    }
    if(MemProps.memoryHeaps[i].size > BiggestHostHeap && i != MemoryInfo.VRamHeap)
    {
      MemoryInfo.DRamHeap = i;
      BiggestHostHeap = MemProps.memoryHeaps[i].size;
    }
  }

  MemoryInfo.VRamHeapSizeTotal = BiggestDeviceHeap;
  MemoryInfo.DRamHeapSizeTotal = BiggestHostHeap;

  uint32_t BiggestDeviceType = 0;
  uint32_t BiggestHostType = 0;

  uint32_t BiggestTransferType = 0;

  bool amdMemory = false;

  for(uint32_t i = 0; i < DeviceExtensions.size(); i++)
  {
    if(!strcmp(DeviceExtensions[i], "VK_AMD_device_coherent_memory"))
    {
      amdMemory = true;
      break;
    }
  }

  bool FoundVRam = false;
  bool FoundDRam = false;

  for(uint32_t i = 0; i < MemProps.memoryTypeCount; i++)
  {
    if(MemProps.memoryTypes[i].heapIndex == MemoryInfo.VRamHeap && MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT && !FoundVRam)
    {
      MemoryInfo.VRamIndex = i;
      FoundVRam = true;
    }
    if(MemProps.memoryTypes[i].heapIndex == MemoryInfo.DRamHeap && MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT && !FoundDRam)
    {
      MemoryInfo.DRamIndex = i;
      FoundDRam = true;
    }

    // this will find a Memory heap and type that can be used for transfer. This is most often found on AMD
    // But we can function if we don't find it.
    if(
      MemProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
      && MemProps.memoryHeaps[MemProps.memoryTypes[i].heapIndex].size > BiggestTransferType
      && MemProps.memoryTypes[i].heapIndex != MemoryInfo.DRamHeap
      && MemProps.memoryTypes[i].heapIndex != MemoryInfo.VRamHeap
      && amdMemory)
    {
      MemoryInfo.TransferHeap = MemProps.memoryTypes[i].heapIndex;
      MemoryInfo.TransferSizeTotal = MemProps.memoryHeaps[MemProps.memoryTypes[i].heapIndex].size;
      MemoryInfo.TransferIndex = i;
    }
  }

  return;
}

VkCommandBuffer* CommandDispatch::GetCommandBuffer(VkDevice* pDevice)
{
  VkResult Res;

  VkCommandBuffer* Ret = new VkCommandBuffer();

  VkCommandBufferAllocateInfo AllocInfo;
  AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  AllocInfo.commandPool = Pool;
  AllocInfo.commandBufferCount = 1;

  if((Res = vkAllocateCommandBuffers(*pDevice, &AllocInfo, Ret)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate command buffer with error: " + std::to_string(Res));
  }

  VkCommandBufferBeginInfo BeginInfo;
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(*Ret, &BeginInfo);

  return Ret;
}

bool CommandDispatch::SubmitCommandBuffer(VkCommandBuffer* CommandBuffer, VkSemaphore* pSemaphores, uint32_t SemaphoreCount, VkPipelineStageFlags WaitStage)
{
  VkResult Res;

  vkEndCommandBuffer(*CommandBuffer);

  VkCommandBufferSubmitInfo cmdSubmitInfo{};
  cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmdSubmitInfo.commandBuffer = *CommandBuffer;

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.signalSemaphoreCount = SemaphoreCount;
  SubmitInfo.pSignalSemaphores = pSemaphores;
  SubmitInfo.pWaitDstStageMask = &WaitStage;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = CommandBuffer;

  if((Res = vkQueueSubmit(Queue, 1, &SubmitInfo, nullptr)) != VK_SUCCESS)
  {
    return false;
  }

  return true;
}

bool Renderer::RequestInstLayer(const char* LayerName)
{
  uint32_t LayerCount;
  vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);
  std::vector<VkLayerProperties> LayerProps(LayerCount);
  vkEnumerateInstanceLayerProperties(&LayerCount, LayerProps.data());

  for(uint32_t i = 0; i < LayerCount; i++)
  {
    if(strcmp(LayerName, LayerProps[i].layerName) == 0)
    {
      InstanceLayers.push_back(LayerName);
      return true;
    }
  }

  return false;
}

bool Renderer::RequestInstExt(const char* ExtensionName)
{
  uint32_t ExtCount;
  vkEnumerateInstanceExtensionProperties(nullptr, &ExtCount, nullptr);
  std::vector<VkExtensionProperties> ExtProps(ExtCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &ExtCount, ExtProps.data());

  for(uint32_t i = 0; i < ExtCount; i++)
  {
    if(strcmp(ExtProps[i].extensionName, ExtensionName) == 0)
    { InstanceExtensions.push_back(ExtensionName);
      return true;
    }
  }
  return false;
}

bool Renderer::RequestDevLayer(const char* LayerName)
{
  uint32_t LayerCount;
  vkEnumerateDeviceLayerProperties(PhysicalDevice, &LayerCount, nullptr);
  std::vector<VkLayerProperties> LayerProps(LayerCount);
  vkEnumerateDeviceLayerProperties(PhysicalDevice, &LayerCount, LayerProps.data());

  for(uint32_t i = 0; i < LayerCount; i++)
  {
    if(strcmp(LayerProps[i].layerName, LayerName) == 0)
    {
      DeviceExtensions.push_back(LayerName);
      return true;
    }
  }
  return false;
}

bool Renderer::RequestDevExt(const char* ExtName)
{
  uint32_t ExtCount;
  vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtCount, nullptr);
  std::vector<VkExtensionProperties> ExtProps(ExtCount);
  vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &ExtCount, ExtProps.data());

  for(uint32_t i = 0; i < ExtCount; i++)
  {
    if(strcmp(ExtProps[i].extensionName, ExtName))
    {
      DeviceExtensions.push_back(ExtName);
      return true;
    }
  }
  return false;
}

void Renderer::Init(uint32_t inWidth, uint32_t inHeight)
{
  Width = inWidth;
  Height = inHeight;

  glfwInit();
  uint32_t glfwExtCount;
  const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwExtCount);
  for(uint32_t i = 0; i < glfwExtCount; i++)
  {
    if(!RequestInstExt(glfwExt[i]))
    {
      std::cout << glfwExt[i] << " is not supported\n";
    }
  }

  VkResult Res;

  VkApplicationInfo AppInfo{};
  AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  AppInfo.apiVersion = VK_API_VERSION_1_1;
  AppInfo.pEngineName = "EzGame";
  AppInfo.engineVersion = 1;
  AppInfo.pApplicationName = "VrGame";
  AppInfo.applicationVersion = 1;

  VkInstanceCreateInfo InstCI{};
  InstCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  InstCI.pApplicationInfo = &AppInfo;

  InstCI.enabledLayerCount = InstanceLayers.size();
  InstCI.ppEnabledLayerNames = InstanceLayers.data();

  InstCI.enabledExtensionCount = InstanceExtensions.size();
  InstCI.ppEnabledExtensionNames = InstanceExtensions.data();

  if((Res = vkCreateInstance(&InstCI, nullptr, &Instance)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create instance in vulkan with error: " + std::to_string(Res));
  }

  uint32_t PDevCount;
  vkEnumeratePhysicalDevices(Instance, &PDevCount, nullptr);
  std::vector<VkPhysicalDevice> PDevices(PDevCount);
  vkEnumeratePhysicalDevices(Instance, &PDevCount, PDevices.data());

  for(uint32_t i = 0; i < PDevCount; i++)
  {
    VkPhysicalDeviceProperties PDevProps;
    vkGetPhysicalDeviceProperties(PDevices[i], &PDevProps);

    if(PDevProps.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
    {
      PhysicalDevice = PDevices[i];
      break;
    }
  }

  uint32_t FamPropCount;
  vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &FamPropCount, nullptr);
  std::vector<VkQueueFamilyProperties> FamProps(FamPropCount);
  vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &FamPropCount, FamProps.data());

  std::cout << FamPropCount << " Queue Families found\n";

  bool GraphicsFound = false;
  bool ComputeFound = false;
  for(uint32_t i = 0; i < FamPropCount; i++)
  {
    if(FamProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !GraphicsFound)
    {
      GraphicsFound = true;
      GraphicsFamily = i;
    }
    if(FamProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
    {
      ComputeFound = true;
      ComputeFamily = i;
    }
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  Window = glfwCreateWindow(Width, Height, "Game", NULL, NULL);

  glfwCreateWindowSurface(Instance, Window, nullptr, &Surface);
}

Camera* Renderer::GetCamera()
{
  return &PrimCamera;
}

bool Renderer::CreateDevice()
{
  VkResult Res;

  VkDeviceQueueCreateInfo QueueInfos[2] = { {}, {} };

  QueueInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueInfos[0].queueCount = 1;
  QueueInfos[0].queueFamilyIndex = GraphicsFamily;

  QueueInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueInfos[1].queueCount = 1;
  QueueInfos[1].queueFamilyIndex = ComputeFamily;

  VkDeviceCreateInfo DevInfo{};
  DevInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  DevInfo.enabledLayerCount = DeviceLayers.size();
  DevInfo.ppEnabledLayerNames = DeviceLayers.data();

  DevInfo.enabledExtensionCount = DeviceExtensions.size();
  DevInfo.ppEnabledExtensionNames = DeviceExtensions.data();

  DevInfo.queueCreateInfoCount = 2;
  DevInfo.pQueueCreateInfos = QueueInfos;

  if((Res = vkCreateDevice(PhysicalDevice, &DevInfo, nullptr, &Device)) != VK_SUCCESS)
  {
    return false;
  }

  vkGetDeviceQueue(Device, GraphicsFamily, 0, &GraphicsDispatch.Queue);
  vkGetDeviceQueue(Device, ComputeFamily, 0, &ComputeDispatch.Queue);
  GraphicsDispatch.PoolFlags = eTypeGraphics;
  ComputeDispatch.PoolFlags = eTypeCompute;

  // Get all the memory information from the physical device and choose our preferred heaps and types
  GetMemoryIndices();

  VkMemoryAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  AllocInfo.memoryTypeIndex = MemoryInfo.VRamIndex;
  AllocInfo.allocationSize = std::max((MemoryInfo.VRamHeapSizeTotal/64), (u_long)250000000);

  BufferMemory.Total = AllocInfo.allocationSize;
  BufferMemory.Available = AllocInfo.allocationSize;
  BufferMemory.Used = 0;

  if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &BufferMemory.Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate buffer memory with error: " + std::to_string(Res));
  }

  AllocInfo.allocationSize = std::max(MemoryInfo.VRamHeapSizeTotal/4, (u_long)500000000);

  TextureMemory.Total = AllocInfo.allocationSize;
  TextureMemory.Available = AllocInfo.allocationSize;
  TextureMemory.Used = 0;

  if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &TextureMemory.Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate texture memory with error: " + std::to_string(Res));
  }

  AllocInfo.allocationSize = std::max(MemoryInfo.VRamHeapSizeTotal/16, (u_long)30000000);

  MeshMemory.Total = AllocInfo.allocationSize;
  MeshMemory.Available = AllocInfo.allocationSize;
  MeshMemory.Used = 0;

  if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &MeshMemory.Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate mesh memory with error: " + std::to_string(Res));
  }

  AllocInfo.allocationSize = MemoryInfo.DRamHeapSizeTotal/8;
  AllocInfo.memoryTypeIndex = MemoryInfo.DRamIndex;
  HostMemory.Total = AllocInfo.allocationSize;
  HostMemory.Available = AllocInfo.allocationSize;
  HostMemory.Used = 0;

  if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &HostMemory.Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate host memory with error: " + std::to_string(Res));
  }

  if(MemoryInfo.TransferIndex != -1)
  {
    AllocInfo.allocationSize = MemoryInfo.TransferSizeTotal/4;
    TransferMemory.Total = AllocInfo.allocationSize;
    TransferMemory.Available = AllocInfo.allocationSize;
    TransferMemory.Used = 0;

    if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &TransferMemory.Memory)) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate memory for transfer with error: " + std::to_string(Res));
    }
  }

  VkSemaphoreCreateInfo SemInf{};
  VkFenceCreateInfo FenceInf{};

  SemInf.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  FenceInf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

  if((Res = vkCreateSemaphore(Device, &SemInf, nullptr, &Semaphore)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create semaphore with error: " + std::to_string(Res));
  }
  if((Res = vkCreateFence(Device, &FenceInf, nullptr, &Fence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create fence with error: " + std::to_string(Res));
  }

  return true;
}

void Renderer::CreateCmdBuffers()
{
  VkResult Res;

  VkCommandPoolCreateInfo GraphicsPoolInfo{};
  GraphicsPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  GraphicsPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  GraphicsPoolInfo.queueFamilyIndex = GraphicsFamily;

  VkCommandPoolCreateInfo ComputePoolInfo{};
  ComputePoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  ComputePoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  ComputePoolInfo.queueFamilyIndex = ComputeFamily;

  if((Res = vkCreateCommandPool(Device, &GraphicsPoolInfo, nullptr, &GraphicsDispatch.Pool)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create graphics pool with error: " + std::to_string(Res));
  }
  if((Res = vkCreateCommandPool(Device, &ComputePoolInfo, nullptr, &ComputeDispatch.Pool)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create compute pool with error: " + std::to_string(Res));
  }
}

void Renderer::CreateSwapchain(VkPresentModeKHR PresentMode)
{
  VkResult Res;

  VkSurfaceCapabilitiesKHR SurfCap;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &SurfCap);

  uint32_t SwapchainFormatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SwapchainFormatCount, nullptr);
  std::vector<VkSurfaceFormatKHR> SurfaceFormats(SwapchainFormatCount);
  if((Res = vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SwapchainFormatCount, SurfaceFormats.data())) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to get surface formats");
  }

  SurfaceFormat = SurfaceFormats[0];

  VkExtent2D SwapExtent;
  SwapExtent.width = Width;
  SwapExtent.height = Height;

  VkSwapchainCreateInfoKHR SwapCI{};
  SwapCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  SwapCI.surface = Surface;
  SwapCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  SwapCI.imageExtent = SwapExtent;
  SwapCI.clipped = VK_TRUE;
  SwapCI.presentMode = PresentMode;
  SwapCI.preTransform = SurfCap.currentTransform;
  SwapCI.minImageCount = SurfCap.minImageCount;
  SwapCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  SwapCI.imageColorSpace = SurfaceFormat.colorSpace;
  SwapCI.imageFormat = SurfaceFormat.format;
  SwapCI.imageArrayLayers = 1;
  SwapCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if((Res = vkCreateSwapchainKHR(Device, &SwapCI, nullptr, &Swapchain)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create swapchain with error: " + std::to_string(Res));
  }

  uint32_t ImageCount;
  vkGetSwapchainImagesKHR(Device, Swapchain, &ImageCount, nullptr);
  SwapchainImages.resize(ImageCount);
  vkGetSwapchainImagesKHR(Device, Swapchain, &ImageCount, SwapchainImages.data());
}

void Renderer::MakeSceneDescriptorPool()
{
  VkResult Res;

  // Camera
  VkDescriptorPoolSize Size;
  Size.descriptorCount = 1;
  Size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

  VkDescriptorPoolCreateInfo PoolCI{};
  PoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  PoolCI.poolSizeCount = 1;
  PoolCI.pPoolSizes = &Size;
  PoolCI.maxSets = 3;

  if((Res = vkCreateDescriptorPool(Device, &PoolCI, nullptr, &ScenePool)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor pool with error " + std::to_string(Res));
  }

  VkDescriptorSetLayoutBinding Binding{};
  Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  Binding.descriptorCount = 1;
  Binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  Binding.binding = 0;

  VkDescriptorSetLayoutCreateInfo LayoutCI{};
  LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  LayoutCI.bindingCount = 1;
  LayoutCI.pBindings = &Binding;

  VkDescriptorSetLayout Layout;

  if((Res = vkCreateDescriptorSetLayout(Device, &LayoutCI, nullptr, &Layout)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor set layout with error: " + std::to_string(Res));
  }

  VkDescriptorSetAllocateInfo AllocInf{};
  AllocInf.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  AllocInf.descriptorPool = ScenePool;
  AllocInf.descriptorSetCount = 1;
  AllocInf.pSetLayouts = &Layout;

  if((Res = vkAllocateDescriptorSets(Device, &AllocInf, &PrimCamera.CamDescriptor)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate descriptor with error: " + std::to_string(Res));
  }
}

void Renderer::CreateRenderPass()
{
  /*
     this is a difficult function, you need to know what attachments you want to have in your framebuffers ahead of time.
    in this case, we need a color buffer(SwapchainImage), a normal buffer, and a depthbuffer.
    on top of knowing what buffers(images) we want in our framebuffers, we need to know what format to use.
    the color buffer's format is already known, we define it in the VkSwapchainCreateInfoKHR structure when we created our swapchain.
    but the normal and depth we have to figure out.
    the depth buffer only does one thing, it tells us how far away a surface is from the screen, Because of this, we can just use one channel, we will use a depth format.
    and the normal buffer tells use where a face is pointing, because we are making a *3D* Renderer, we need to store 3 values (XYZ), so we will use an RGB format.
  */

  VkResult Res;

  VkAttachmentReference AttRefs[3]; // Color, pos, norm
  VkAttachmentReference pDepth;
  VkAttachmentDescription Descriptions[4] = {{}, {}, {}, {}}; // Color, Pos, Depth, Normal

  Descriptions[0].format = SurfaceFormat.format;
  Descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  Descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  Descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
  Descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  Descriptions[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  Descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  Descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

  Descriptions[2].format = VK_FORMAT_D16_UNORM;
  Descriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  Descriptions[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  Descriptions[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  Descriptions[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  Descriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  Descriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  Descriptions[2].samples = VK_SAMPLE_COUNT_1_BIT;

  Descriptions[3].format = VK_FORMAT_R16G16B16A16_SFLOAT;
  Descriptions[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  Descriptions[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  Descriptions[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  Descriptions[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  Descriptions[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  Descriptions[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  Descriptions[3].samples = VK_SAMPLE_COUNT_1_BIT;

  Descriptions[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
  Descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  Descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  Descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  Descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  Descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  Descriptions[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  Descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;

  AttRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  AttRefs[0].attachment = 0;

  AttRefs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  AttRefs[1].attachment = 1;

  pDepth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  pDepth.attachment = 2;

  AttRefs[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  AttRefs[2].attachment = 3;

  VkSubpassDescription Subpasses[2];

  Subpasses[0] = {}; // fill with 0/defaults
  Subpasses[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  Subpasses[0].colorAttachmentCount = 3;
  Subpasses[0].pColorAttachments = AttRefs;
  Subpasses[0].pDepthStencilAttachment = &pDepth;

  Subpasses[1] = {};
  Subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  Subpasses[1].colorAttachmentCount = 3;
  Subpasses[1].pColorAttachments = AttRefs;
  Subpasses[1].pDepthStencilAttachment = &pDepth;

  VkSubpassDependency LightingDependencies{};
  LightingDependencies.srcSubpass = 0;
  LightingDependencies.dstSubpass = 1;
  LightingDependencies.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  LightingDependencies.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  LightingDependencies.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  LightingDependencies.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
  // the fragment shader from the G-pass needs to finish writing to the Framebuffer before the lighting pass is executed

  VkRenderPassCreateInfo RenderpassInfo{};
  RenderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  RenderpassInfo.subpassCount = 2;
  RenderpassInfo.pSubpasses = Subpasses;
  RenderpassInfo.attachmentCount = 4;
  RenderpassInfo.pAttachments = Descriptions;
  RenderpassInfo.dependencyCount = 1;
  RenderpassInfo.pDependencies = &LightingDependencies;

  if((Res = vkCreateRenderPass(Device, &RenderpassInfo, nullptr, &Renderpass)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create renderpass with error: " + std::to_string(Res));
  }

  MakeSceneDescriptorPool();
}

void Renderer::CreateFrameBuffers()
{
  VkResult Res;

  SwapchainImageViews.resize(SwapchainImages.size());
  DepthImages.resize(SwapchainImages.size()); DepthImageViews.resize(SwapchainImages.size());
  NormImages.resize(SwapchainImages.size()); NormImageViews.resize(SwapchainImages.size());
  PosImages.resize(SwapchainImages.size()); PosImageViews.resize(SwapchainImageViews.size());

  FrameBuffers.resize(SwapchainImages.size());

  for(uint32_t i = 0; i < SwapchainImages.size(); i++)
  {
    CreateImage(&DepthImages[i], VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    CreateImage(&NormImages[i], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    CreateImage(&PosImages[i], VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);

    CreateImageView(&SwapchainImageViews[i], &SwapchainImages[i], SurfaceFormat.format, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
    CreateImageView(&PosImageViews[i], &PosImages[i].Image, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
    CreateImageView(&DepthImageViews[i], &DepthImages[i].Image, VK_FORMAT_D16_UNORM, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
    CreateImageView(&NormImageViews[i], &NormImages[i].Image, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);

    VkImageView Atts[4] = { SwapchainImageViews[i], PosImageViews[i], DepthImageViews[i], NormImageViews[i] };

    VkFramebufferCreateInfo FrameBufferCI{};
    FrameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FrameBufferCI.width = Width;
    FrameBufferCI.height = Height;
    FrameBufferCI.renderPass = Renderpass;
    FrameBufferCI.attachmentCount = 4;
    FrameBufferCI.pAttachments = Atts;
    FrameBufferCI.layers = 1;

    if((Res = vkCreateFramebuffer(Device, &FrameBufferCI, nullptr, &FrameBuffers[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create framebuffer with error: " + std::to_string(Res));
    }
  }
}

Material Renderer::CreateMat()
{
  Material Ret;
  Ret.pDevice = &Device;

  return Ret;
}

Shader Renderer::CreateShader(const char* ShaderPath, const char* EntryPoint)
{
  VkResult Res;

  Shader Ret;
  Ret.EntryPoint = EntryPoint;

  std::vector<char> ShaderCode = ReadFile(ShaderPath);

  VkShaderModuleCreateInfo ShaderCI{};
  ShaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  ShaderCI.pCode = reinterpret_cast<const uint32_t*>(ShaderCode.data());
  ShaderCI.codeSize = ShaderCode.size();

  if((Res = vkCreateShaderModule(Device, &ShaderCI, nullptr, &Ret.sModule)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create the shader module with error: " + std::to_string(Res));
  }

  return Ret;
}

Pipeline Renderer::CreatePipeline(Material Mat)
{
  Pipeline Ret(&Device);

  Ret.Mat = Mat;

  Ret.BakeRecipe(Width, Height, &Renderpass, ePassType::ePrimary);
  Ret.Init();

  return Ret;
}

