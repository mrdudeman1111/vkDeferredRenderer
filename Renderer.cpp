#include "Mesh.h"
#include "Pipeline.h"
#include "Widget.h"
#include <GLFW/glfw3.h>
#include <Renderer.h>

#include <assimp/types.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/transform.hpp>

#include <iostream>
#include <cstring>
#include <string>

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

UI* PrimaryUI;

void ClickCallback(GLFWwindow* pWindow, int Button, int Action, int Mods)
{
  double Pos[2];
  glfwGetCursorPos(pWindow, &Pos[0], &Pos[1]);
  PrimaryUI->ClickEvent(Button, Action, Pos);
}

void UI::ClickEvent(int Button, int Action, double MousePos[2])
{
  for(uint32_t i = 0; i < Panes.size(); i++)
  {
    Panes[i].ClickEvent(MousePos);
  }
}

void UI::AddPane(Renderer* pRender)
{
  Buffer vBuff, iBuff;

  pRender->CreateBuffer(&vBuff, sizeof(Vertex2D) * 100, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, eBufferMemory);
  pRender->CreateBuffer(&iBuff, sizeof(uint32_t) * 150, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, eBufferMemory);

  EkWidget::EkUIPane Pane(vBuff, iBuff, &pRender->Device);

  Panes.push_back(Pane);
}

// Todo: Read Sync Creation

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

void PrintMat(const char* Name, glm::mat4* Matrix)
{
  std::cout << "-------------" << Name << "-------------\n";
  for(uint32_t i = 0; i < 4; i++)
  {
    for(uint32_t x = 0; x < 4; x++)
    {
      std::cout << (*Matrix)[x][i] << ' ';
    }
    std::cout << '\n';
  }
}



void Camera::PollInputs(float DeltaTime)
{
  Matrices.World = glm::mat4(1.f);
  Matrices.Proj = glm::perspective(glm::radians(90.f), ((float)pRenderer->Width/(float)pRenderer->Height), 0.1f, 1000.f);
  Matrices.Proj[1][1] *= -1;

    double X, Y;
    glfwGetCursorPos(pRenderer->Window, &X, &Y);

    double DeltaX, DeltaY;

    DeltaX = LastMousePos[0] - X;
    DeltaY = LastMousePos[1] - Y;

    LastMousePos[0] = X;
    LastMousePos[1] = Y;

    Rotation.y += (DeltaX * Sensitivity);
    Rotation.x += (DeltaY * Sensitivity);
  /*
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_UP) == GLFW_PRESS)
    {
      Rotation.x += 1*Sensitivity;
    }
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
      Rotation.x -= 1*Sensitivity;
    }
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
      Rotation.y -= 1*Sensitivity;
    }
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
      Rotation.y += 1*Sensitivity;
    }
  */

    glm::vec3 MoveUp = glm::normalize(glm::vec3(Matrices.View[0][1], Matrices.View[1][1], Matrices.View[2][1]));
    glm::vec3 Right = glm::normalize(glm::vec3(Matrices.View[0][0], Matrices.View[1][0], Matrices.View[2][0]));
    glm::vec3 Forward = glm::normalize(glm::vec3(Matrices.View[0][2], Matrices.View[1][2], Matrices.View[2][2]));

    float ShiftAffect = 1.f;

    if(glfwGetKey(pRenderer->Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
      ShiftAffect = 0.5f;
    }

    if(glfwGetKey(pRenderer->Window, GLFW_KEY_W) == GLFW_PRESS)
    {
      Position -= Forward*MoveSensitivity*ShiftAffect;
    }
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_S) == GLFW_PRESS)
    {
      Position += Forward*MoveSensitivity*ShiftAffect;
    }
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_A) == GLFW_PRESS)
    {
      Position -= Right*MoveSensitivity*ShiftAffect;
    }
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_D) == GLFW_PRESS)
    {
      Position += Right*MoveSensitivity*ShiftAffect;
    }
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_E) == GLFW_PRESS)
    {
      Position += MoveUp*MoveSensitivity*ShiftAffect;
    }
    if(glfwGetKey(pRenderer->Window, GLFW_KEY_Q) == GLFW_PRESS)
    {
      Position -= MoveUp*MoveSensitivity*ShiftAffect;
    }

    Rotation.x = std::clamp(Rotation.x, -90.f, 90.f);

    CameraMat = glm::mat4(1.f);

    CameraMat = glm::translate(CameraMat, Position);

    glm::vec3 Up = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 LookRight = glm::vec3(CameraMat[0][0], CameraMat[1][0], CameraMat[2][0]);
    glm::vec3 LookForward = glm::vec3(CameraMat[0][2], CameraMat[1][2], CameraMat[2][2]);

    CameraMat = glm::rotate(CameraMat, glm::radians(Rotation.y), Up);

    CameraMat = glm::rotate(CameraMat, glm::radians(Rotation.x), LookRight);

    CameraMat = glm::rotate(CameraMat, glm::radians(Rotation.z), LookForward);


  if(glfwGetKey(pRenderer->Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    Focus = (Focus) ? false : true;

    if(Focus == true) glfwSetInputMode(pRenderer->Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    else glfwSetInputMode(pRenderer->Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  Matrices.View = glm::inverse(CameraMat);
}

void Camera::Update(float DeltaTime)
{
  VkResult Res;

  glfwPollEvents();

  if(CameraBuffer.Allocation.AllocSize == -1)
  {
    pRenderer->CreateBuffer(&CameraBuffer, sizeof(glm::mat4) * 3, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, eHostMemory);
  }

  PollInputs(DeltaTime);

  void* BufferMemory;
  vkMapMemory(pRenderer->Device, *CameraBuffer.Allocation.Memory, CameraBuffer.Allocation.Offset, CameraBuffer.Allocation.AllocSize, 0, &BufferMemory);

  memcpy(BufferMemory, &Matrices, sizeof(Matrices));

  /*
    memcpy(BufferMemory, &World, sizeof(World));
    memcpy(BufferMemory+1, &View, sizeof(View));
    memcpy(BufferMemory+2, &Proj, sizeof(Proj));
  */

  // PrintMat("World", BufferMemory);
  // PrintMat("View", BufferMemory+1);
  // PrintMat("Proj", BufferMemory+2);

  vkUnmapMemory(pRenderer->Device, pRenderer->HostMemory.Memory);

  VkDescriptorBufferInfo BuffInfo{};
  BuffInfo.range = sizeof(glm::mat4) * 3;
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

bool Renderer::HasTransfer()
{
  return (MemoryInfo.TransferIndex == -1) ? true : false;
}

void Renderer::CopyToBuffer(Buffer* Src, Buffer* Dst, VkBufferCopy CopyInfo)
{
  VkCommandBuffer* Buffer;

  /*
   * if the source is on DRAM and the destination is on VRAM, we use the transferqueue to quickly cross the PCIE bus
   * if the source is on VRAM and the destination is on VRAM, we use the computequeue to quickly cross across VRAM
  */

  // in this case we are performing VRAM->VRAM copy, so no PCIE bus crossing. This means we will want to use anything other than the transfer queue, preferably the compute queue

  uint32_t VRAMflags = (eTransferMemory | eBufferMemory | eTextureMemory | eMeshMemory);

  if(Src->Allocation.MemoryFlags & VRAMflags && Dst->Allocation.MemoryFlags & VRAMflags)
  {
    Buffer = ComputeDispatch.GetCommandBuffer(&Device);
  }

  // in this case, we are crossing the PCIE bus on the motherboard, for these transfers we will make use of the Transfer queue available on nearly all GPUs
  else if(Src->Allocation.MemoryFlags & eHostMemory && Dst->Allocation.MemoryFlags & VRAMflags)
  {
    Buffer = TransferDispatch.GetCommandBuffer(&Device);
  }

  vkCmdCopyBuffer(*Buffer, Src->Buffer, Dst->Buffer, 1, &CopyInfo);

  if(Src->Allocation.MemoryFlags & VRAMflags && Dst->Allocation.MemoryFlags & VRAMflags)
  {
    ComputeDispatch.SubmitCommandBuffer(Buffer);
  }
  else if(Src->Allocation.MemoryFlags & eHostMemory && Dst->Allocation.MemoryFlags & VRAMflags)
  {
    TransferDispatch.SubmitCommandBuffer(Buffer);
  }
}

void Renderer::CopyToImage(Buffer* Src, Texture* Dst, VkBufferImageCopy CopyInfo)
{
  VkCommandBuffer* Buffer;

  if(Src->Allocation.MemoryFlags & eTransferMemory)
  {
    Buffer = ComputeDispatch.GetCommandBuffer(&Device);
  }
  else if(Src->Allocation.MemoryFlags & eHostMemory && Dst->Allocation.MemoryFlags)
  {
    Buffer = TransferDispatch.GetCommandBuffer(&Device);
  }

  vkCmdCopyBufferToImage(*Buffer, Src->Buffer, Dst->Image, Dst->Layout, 1, &CopyInfo);

  if(Src->Allocation.MemoryFlags & eTransferMemory)
  {
    ComputeDispatch.SubmitCommandBuffer(Buffer);
  }
  else if(Src->Allocation.MemoryFlags & eHostMemory && Dst->Allocation.MemoryFlags)
  {
    TransferDispatch.SubmitCommandBuffer(Buffer);
  }
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
  Image->Allocation.Memory = &MemoryTarget->Memory;
  Image->Allocation.MemoryFlags = MemoryTarget->MemType;

  Image->Format = Format;
  Image->Layout = VK_IMAGE_LAYOUT_UNDEFINED;

  MemoryTarget->Available -= VkDeviceSize(Alignment);
  MemoryTarget->Used += VkDeviceSize(Alignment);
  MemoryTarget->Allocations.push_back(&Image->Allocation);
}

void Renderer::CreateBuffer(Buffer* Buffer, uint32_t Size, VkBufferUsageFlags Usage, eMemoryType Memory)
{
  VkResult Res;

  VkBufferCreateInfo BufferCI{};
  BufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  BufferCI.size = Size;
  BufferCI.usage = Usage;
  BufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if((Res = vkCreateBuffer(Device, &BufferCI, nullptr, &Buffer->Buffer)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate buffer, is the memory full? error : " + std::to_string(Res));
  }

  VkMemoryRequirements Req;
  vkGetBufferMemoryRequirements(Device, Buffer->Buffer, &Req);

  uint32_t Alignment = 0;

  MemoryBlock* TargetMemory;

  switch(Memory)
  {
    case eBufferMemory:
      TargetMemory = &BufferMemory;
      Buffer->Allocation.MemoryFlags = eBufferMemory;
      break;
    case eTextureMemory:
      TargetMemory = &TextureMemory;
      Buffer->Allocation.MemoryFlags = eTextureMemory;
      break;
    case eTransferMemory:
      TargetMemory = &TransferMemory;
      Buffer->Allocation.MemoryFlags = eTransferMemory;
      break;
    case eMeshMemory:
      TargetMemory = &MeshMemory;
      Buffer->Allocation.MemoryFlags = eMeshMemory;
      break;
    case eHostMemory:
      TargetMemory = &HostMemory;
      Buffer->Allocation.MemoryFlags = eMeshMemory;
      break;
    default:
      TargetMemory = &BufferMemory;
      break;
  }

  for(uint32_t i = 0; Alignment < TargetMemory->Used; i++)
  {
    Alignment = i * Req.alignment;
  }

  vkBindBufferMemory(Device, Buffer->Buffer, TargetMemory->Memory, Alignment);

  Buffer->Allocation.Offset = Alignment;
  Buffer->Allocation.AllocSize = Size;
  Buffer->Allocation.Memory = &TargetMemory->Memory;

  TargetMemory->Used += Buffer->Allocation.AllocSize;
  TargetMemory->Available -= Buffer->Allocation.AllocSize;
  TargetMemory->Allocations.push_back(&Buffer->Allocation);
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

float Renderer::GetDeltaTime()
{
  auto Time = std::chrono::steady_clock::now();
  auto Res = (Time - PreviousTime);
  PreviousTime = Time;
  return Res.count();
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
      && MemProps.memoryTypes[i].heapIndex != MemoryInfo.VRamHeap)
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

  VkCommandBufferAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  AllocInfo.commandPool = Pool;
  AllocInfo.commandBufferCount = 1;

  if((Res = vkAllocateCommandBuffers(*pDevice, &AllocInfo, Ret)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate command buffer with error: " + std::to_string(Res));
  }

  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  BeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(*Ret, &BeginInfo);

  return Ret;
}

VkCommandBuffer* CommandDispatch::GetRenderBuffer(VkDevice* pDevice)
{
  VkResult Res;

  VkCommandBuffer* Ret = new VkCommandBuffer();

  VkCommandBufferAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  AllocInfo.commandPool = Pool;
  AllocInfo.commandBufferCount = 1;

  if((Res = vkAllocateCommandBuffers(*pDevice, &AllocInfo, Ret)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate render buffer with error: "  + std::to_string(Res));
  }

  return Ret;
}

bool CommandDispatch::SubmitCommandBuffer(VkCommandBuffer* CommandBuffer, VkFence* Fence, VkSemaphore* pSemaphores, uint32_t SemaphoreCount, VkPipelineStageFlags WaitStage)
{
  VkResult Res;

  vkEndCommandBuffer(*CommandBuffer);

  VkCommandBufferSubmitInfo cmdSubmitInfo{};
  cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmdSubmitInfo.commandBuffer = *CommandBuffer;

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  SubmitInfo.signalSemaphoreCount = SemaphoreCount;
  SubmitInfo.pSignalSemaphores = (SemaphoreCount == 0) ? nullptr : pSemaphores;
  SubmitInfo.pWaitDstStageMask = &WaitStage;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = CommandBuffer;

  if((Res = vkQueueSubmit(Queue, 1, &SubmitInfo, (Fence != nullptr) ? *Fence : nullptr)) != VK_SUCCESS)
  {
    return false;
  }

  return true;
}

void CommandDispatch::SubmitRenderBuffer(VkCommandBuffer* CommandBuffer, VkFence* Fence, VkSemaphore* pWaitSemaphores, uint32_t WaitSemaphoreCount, VkSemaphore* pSignalSemaphores, uint32_t SignalSemaphoreCount, VkPipelineStageFlags WaitStage)
{
  VkResult Res;

  VkCommandBufferSubmitInfo cmdSubmitInfo{};
  cmdSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
  cmdSubmitInfo.commandBuffer = *CommandBuffer;

  VkSubmitInfo SubmitInfo{};
  SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  SubmitInfo.waitSemaphoreCount = WaitSemaphoreCount;
  SubmitInfo.pWaitSemaphores = (WaitSemaphoreCount == 0) ? nullptr : pWaitSemaphores;

  SubmitInfo.signalSemaphoreCount = SignalSemaphoreCount;
  SubmitInfo.pSignalSemaphores = (SignalSemaphoreCount == 0) ? nullptr : pSignalSemaphores;

  SubmitInfo.pWaitDstStageMask = &WaitStage;
  SubmitInfo.commandBufferCount = 1;
  SubmitInfo.pCommandBuffers = CommandBuffer;

  if((Res = vkQueueSubmit(Queue, 1, &SubmitInfo, (Fence == nullptr) ? nullptr : *Fence)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to submit render buffer with error: " + std::to_string(Res));
  }

  return;
}

void DescriptorPool::AddResourceBinding(VkDescriptorSetLayoutBinding Binding)
{
  Bindings.push_back(Binding);
}

void DescriptorPool::Build()
{
  VkResult Res;

  std::vector<VkDescriptorPoolSize> Sizes;

  for(uint32_t i = 0; i < Bindings.size(); i++)
  {
    VkDescriptorPoolSize Size{};
    Size.type = Bindings[i].descriptorType;
    Size.descriptorCount = Bindings[i].descriptorCount * numSets;

    Sizes.push_back(Size);
  }

  VkDescriptorPoolCreateInfo PoolCI{};
  PoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  PoolCI.maxSets = numSets;
  PoolCI.poolSizeCount = Sizes.size();
  PoolCI.pPoolSizes = Sizes.data();

  if((Res = vkCreateDescriptorPool(*pDevice, &PoolCI, nullptr, &Pool)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor pool with error: " + std::to_string(Res));
  }

  VkDescriptorSetLayoutCreateInfo LayCI{};
  LayCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  LayCI.bindingCount = Bindings.size();
  LayCI.pBindings = Bindings.data();

  if((Res = vkCreateDescriptorSetLayout(*pDevice, &LayCI, nullptr, &Layout)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor set layout with error: " + std::to_string(Res));
  }
}

VkDescriptorSet DescriptorPool::MakeDescriptor()
{
  VkResult Res;

  VkDescriptorSet Ret;

  if(Layout == VK_NULL_HANDLE)
  {
    VkDescriptorSetLayoutCreateInfo LayCI{};
    LayCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayCI.bindingCount = Bindings.size();
    LayCI.pBindings = Bindings.data();

    if((Res = vkCreateDescriptorSetLayout(*pDevice, &LayCI, nullptr, &Layout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor set layout with error: " + std::to_string(Res));
    }
  }

  VkDescriptorSetAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  AllocInfo.descriptorPool = Pool;
  AllocInfo.descriptorSetCount = 1;
  AllocInfo.pSetLayouts = &Layout;

  if((Res = vkAllocateDescriptorSets(*pDevice, &AllocInfo, &Ret)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to allocate descriptor set with error: " + std::to_string(Res));
  }

  return Ret;
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
    {
      InstanceExtensions.push_back(ExtensionName);
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
  bool TransferFound = false;
  for(uint32_t i = 0; i < FamPropCount; i++)
  {
    std::cout << string_VkQueueFlags(FamProps[i].queueFlags) << '\n';
    if(FamProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !GraphicsFound && FamProps[i].queueCount > 0)
    {
      std::cout << "Found graphics queue\n";
      FamProps[i].queueCount--;
      GraphicsFound = true;
      GraphicsFamily = i;
    }
    if(FamProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !ComputeFound && FamProps[i].queueCount > 0)
    {
      std::cout << "Found compute queue\n";
      FamProps[i].queueCount--;
      ComputeFound = true;
      ComputeFamily = i;
    }
    if(FamProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT && !TransferFound && FamProps[i].queueCount > 0)
    {
      std::cout << "Found transfer queue\n";
      FamProps[i].queueCount--;
      TransferFound = true;
      TransferFamily = i;
    }
  }

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  Window = glfwCreateWindow(Width, Height, "Game", NULL, NULL);

  glfwCreateWindowSurface(Instance, Window, nullptr, &Surface);

  glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

  PrimaryUI = new UI();
}

Camera* Renderer::GetCamera()
{
  return &PrimCamera;
}

VkDescriptorSet Renderer::GetCameraDescriptor()
{
  return PrimCamera.CamDescriptor;
}

bool Renderer::CreateDevice()
{
  VkResult Res;

  VkDeviceQueueCreateInfo QueueInfos[3] = { {}, {}, {} };

  QueueInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueInfos[0].queueCount = 1;
  QueueInfos[0].queueFamilyIndex = GraphicsFamily;

  QueueInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueInfos[1].queueCount = 1;
  QueueInfos[1].queueFamilyIndex = ComputeFamily;

  QueueInfos[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  QueueInfos[2].queueCount = 1;
  QueueInfos[2].queueFamilyIndex = TransferFamily;

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
  vkGetDeviceQueue(Device, (HasTransfer()) ? TransferFamily : ComputeFamily, 0, &TransferDispatch.Queue);

  GraphicsDispatch.PoolFlags = eTypeGraphics;
  ComputeDispatch.PoolFlags = eTypeCompute;
  TransferDispatch.PoolFlags = eTypeTransfer;

  // Get all the memory information from the physical device and choose our preferred heaps and types
  GetMemoryIndices();

  VkMemoryAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  AllocInfo.memoryTypeIndex = MemoryInfo.VRamIndex;
  AllocInfo.allocationSize = std::max((MemoryInfo.VRamHeapSizeTotal/64), (u_long)250000000);

  BufferMemory.Total = AllocInfo.allocationSize;
  BufferMemory.Available = AllocInfo.allocationSize;
  BufferMemory.Used = 0;
  BufferMemory.MemType = eBufferMemory;

  if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &BufferMemory.Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate buffer memory with error: " + std::to_string(Res));
  }

  AllocInfo.allocationSize = std::max(MemoryInfo.VRamHeapSizeTotal/4, (u_long)500000000);

  TextureMemory.Total = AllocInfo.allocationSize;
  TextureMemory.Available = AllocInfo.allocationSize;
  TextureMemory.Used = 0;
  TextureMemory.MemType = eTextureMemory;

  if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &TextureMemory.Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate texture memory with error: " + std::to_string(Res));
  }

  AllocInfo.allocationSize = std::max(MemoryInfo.VRamHeapSizeTotal/16, (u_long)30000000);

  MeshMemory.Total = AllocInfo.allocationSize;
  MeshMemory.Available = AllocInfo.allocationSize;
  MeshMemory.Used = 0;
  MeshMemory.MemType = eMeshMemory;

  if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &MeshMemory.Memory)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to allocate mesh memory with error: " + std::to_string(Res));
  }

  AllocInfo.allocationSize = MemoryInfo.DRamHeapSizeTotal/8;
  AllocInfo.memoryTypeIndex = MemoryInfo.DRamIndex;

  HostMemory.Total = AllocInfo.allocationSize;
  HostMemory.Available = AllocInfo.allocationSize;
  HostMemory.Used = 0;
  HostMemory.MemType = eHostMemory;

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
    TransferMemory.MemType = eTransferMemory;

    if((Res = vkAllocateMemory(Device, &AllocInfo, nullptr, &TransferMemory.Memory)) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to allocate memory for transfer with error: " + std::to_string(Res));
    }
  }

  CreateCmdBuffers();

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
  RenderFormat = SurfaceFormat.format;

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

  // Swapchain image
    EkFrameBufferAttachment ColorAttachment;
    ColorAttachment.IMG_Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    ColorAttachment.IMG_ViewType = VK_IMAGE_VIEW_TYPE_2D;
    ColorAttachment.IMG_Format = RenderFormat;
    ColorAttachment.IMG_Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ColorAttachment.FB_Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    ColorAttachment.FB_Description.format = RenderFormat;
    ColorAttachment.FB_Description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.FB_Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.FB_Description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.FB_Description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    ColorAttachment.FB_Description.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachment.FB_Description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.FB_Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachment.FB_Description.flags = 0;

    ColorAttachment.Location = 0;

  // UI Image
    EkFrameBufferAttachment UIAttachment;
    UIAttachment.IMG_Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    UIAttachment.IMG_ViewType = VK_IMAGE_VIEW_TYPE_2D;
    UIAttachment.IMG_Format = RenderFormat; // so it will be easier to blend with the swapchain image.
    UIAttachment.IMG_Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    UIAttachment.FB_Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    UIAttachment.FB_Description.format = RenderFormat;
    UIAttachment.FB_Description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    UIAttachment.FB_Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    UIAttachment.FB_Description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    UIAttachment.FB_Description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    UIAttachment.FB_Description.samples = VK_SAMPLE_COUNT_1_BIT;
    UIAttachment.FB_Description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    UIAttachment.FB_Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    UIAttachment.FB_Description.flags = 0;

    UIAttachment.Location = 1;

  AddFrameBufferAttachment(&ColorAttachment, eColor);
  AddFrameBufferAttachment(&UIAttachment, eColor);
}

void Renderer::MakeSceneDescriptorPool()
{
  VkResult Res;

  // Camera
    VkDescriptorSetLayoutBinding Binding{};
    Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Binding.descriptorCount = 1;
    Binding.binding = 0;
    Binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    ScenePool.AddResourceBinding(Binding);

    ScenePool.Build();

    PrimCamera.CamDescriptor = ScenePool.MakeDescriptor();
    PrimCamera.Layout = *ScenePool.GetLayout();
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

Pipeline Renderer::CreatePipeline(Material Mat, eSpace SpaceType, uint32_t Width, uint32_t Height, int X, int Y)
{
  Pipeline Ret(&Device, &Renderpass);

  Ret.Mat = Mat;

  std::vector<PipelineAttachment> PipeAtts(AttachmentInfo.size());

  for(uint32_t i = 0; i < AttachmentInfo.size(); i++)
  {
    PipeAtts[i].Transparent = AttachmentInfo[i].Transparency;
  }

  Ret.BakeRecipe(Width, Height, X, Y, SpaceType, ePassType::ePrimary, PipeAtts);
  Ret.Init(PrimCamera.Layout);

  return Ret;
}

void Renderer::CreateUIPane()
{
  if(UIPipe.Pipe != VK_NULL_HANDLE)
  {
    PrimaryUI->AddPane(this);
  }
  else
  {
    Material UIMat = CreateMat();

    Shader Vert = CreateShader(ShaderDir"UIVert.spv", "main");
    Shader Frag = CreateShader(ShaderDir"UIFrag.spv", "main");

    UIMat.VertShader = &Vert;
    UIMat.FragShader = &Frag;

    std::vector<PipelineAttachment> PipeAtts(1);
    PipeAtts[0].Transparent = true;

    UIPipe.BakeRecipe(1280, 720, 0, 0, e2D, ePrimary, PipeAtts);
    UIPipe.Init(PrimCamera.Layout);
  }
}

void Renderer::WaitOnLastFrame()
{
  // that huge weird conditional checks if the FrameIndex-1 is less than zero, if it is we need to reach around to the front of the array.
  vkWaitForFences(Device, 1, &RenderDone[0], VK_TRUE, UINT64_MAX);

  if(UIPipe.Pipe != VK_NULL_HANDLE)
  {
    vkWaitForFences(Device, 1, &RenderDone[1], VK_TRUE, UINT64_MAX);
    vkResetFences(Device, 1, &RenderDone[1]);
  }

  vkResetFences(Device, 1, &RenderDone[0]);
}

void Renderer::BeginRenderPass(VkCommandBuffer* pBuff, VkRect2D RenderArea)
{
  vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, RenderReady[0], VK_NULL_HANDLE, &ImageIndex);

  VkClearColorValue Color{};

  Color.int32[0] = 0;
  Color.int32[1] = 0;
  Color.int32[2] = 0;

  Color.uint32[0] = 0;
  Color.uint32[1] = 0;
  Color.uint32[2] = 0;

  Color.float32[0] = 0.f;
  Color.float32[1] = 0.f;
  Color.float32[2] = 0.f;

  VkClearValue Clear[4] = { {}, {}, {}, {} };
  Clear[0].color = Color;

  Clear[1].color = Color;
  Clear[2].color = Color;

  Clear[3].depthStencil.depth = 1.f;


  VkRenderPassBeginInfo BeginInf{};
  BeginInf.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  BeginInf.renderPass = Renderpass;
  BeginInf.clearValueCount = 4;
  BeginInf.pClearValues = Clear;
  BeginInf.renderArea = RenderArea;
  BeginInf.framebuffer = FrameBuffers[ImageIndex];

  vkCmdBeginRenderPass(*pBuff, &BeginInf, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::EndRenderPass(VkCommandBuffer* pBuff)
{
  vkCmdEndRenderPass(*pBuff);
}

// will wait on RenderSubmit
void Renderer::Present()
{
  if(UIPipe.Pipe != VK_NULL_HANDLE)
  {
    RenderUI();
  }

  VkPresentInfoKHR PresentInfo{};
  PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  PresentInfo.swapchainCount = 1;
  PresentInfo.pSwapchains = &Swapchain;
  PresentInfo.pImageIndices = &ImageIndex;
  PresentInfo.waitSemaphoreCount = 1;
  PresentInfo.pWaitSemaphores = &RenderSubmit[0]; // wait on the submitted render command buffer to finish executing before we present the frame buffer

  vkQueuePresentKHR(GraphicsDispatch.Queue, &PresentInfo);

  FrameIndex = (FrameIndex + 1) % FrameBuffers.size();
}

void Renderer::RenderUI()
{
  if(cmdUIBuffer == nullptr)
  {
    cmdUIBuffer = GraphicsDispatch.GetRenderBuffer(&Device);
  }

  vkResetCommandBuffer(*cmdUIBuffer, 0);

  VkCommandBufferBeginInfo BeginInfo{};
  BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  vkBeginCommandBuffer(*cmdUIBuffer, &BeginInfo);

  vkCmdBindPipeline(*cmdUIBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, UIPipe.Pipe);

  for(uint32_t i = 0; i < PrimaryUI->Panes.size(); i++)
  {
    PrimaryUI->Panes[i].Draw(cmdUIBuffer, Width, Height);
  }

  vkCmdEndRenderPass(*cmdUIBuffer);

  GraphicsDispatch.SubmitRenderBuffer(cmdUIBuffer, &RenderDone[1]);
}

