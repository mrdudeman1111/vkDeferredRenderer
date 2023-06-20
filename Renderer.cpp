#include <Renderer.h>

#include <cstring>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan_core.h>

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

void Renderer::Init()
{
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
  Window = glfwCreateWindow(1280, 720, "Game", NULL, NULL);

  glfwCreateWindowSurface(Instance, Window, nullptr, &Surface);
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
  vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &SwapchainFormatCount, SurfaceFormats.data());

  for(uint32_t i = 0; i < SwapchainFormatCount; i++)
  {
    if(SurfaceFormats[i].format == VK_FORMAT_R8G8B8_SRGB)
    {
      SurfaceFormat = SurfaceFormats[i];
    }
  }

  VkExtent2D SwapExtent;
  SwapExtent.width = 1280;
  SwapExtent.height = 720;

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

void Renderer::CreateRenderPass()
{
  /*
     this is a difficult function, you need to know what attachments you want to have in your framebuffers ahead of time.
    in this case, we need a color buffer(SwapchainImage), a normal buffer, and a depthbuffer.
    on top of knowing what buffers(images) we want in our framebuffers, we need to know what format to use.
    the color buffer's format is already known, we define it in the VkSwapchainCreateInfoKHR structure when we created our swapchain.
    but the normal and depth we have to figure out.
    the depth buffer only does one thing, it tells us how far away a surface is from the screen, Because of this, we can just use one channel, we will use a depth format.
    and the normal buffer tells use where a face is pointing, because we are making a *3D* Renderer, we need to store 3 values, so we will use an RGB format.
  */

  VkAttachmentDescription ColorDesc{};
  VkAttachmentDescription DepthDesc{};
  VkAttachmentDescription NormDesc{};

  VkAttachmentReference pColor, pDepth, pNorm;

  ColorDesc.format = SurfaceFormat.format;
  ColorDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  ColorDesc.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
  ColorDesc.samples = VK_SAMPLE_COUNT_1_BIT;
  ColorDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  ColorDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  ColorDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_NONE;
  ColorDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

  DepthDesc.format = VK_FORMAT_D16_UNORM;
  DepthDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  DepthDesc.storeOp = VK_ATTACHMENT_STORE_OP_NONE;
}

