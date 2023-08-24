#include <Renderer.h>
#include <vulkan/vulkan_core.h>

void Renderer::AddFrameBufferAttachment(EkFrameBufferAttachment* Attachment, eAttachmentType Type)
{
  if(Type == eDepth)
  {
    DepthAttachment = Attachment;
  }
  else
  {
    AttachmentInfo.push_back(*Attachment);
  }
}

void Renderer::CreateFrameBuffers()
{
  VkResult Res;

  uint32_t AttachmentCount = (DepthAttachment == nullptr) ? SwapchainImages.size()*AttachmentInfo.size() : (SwapchainImages.size()*AttachmentInfo.size()) + SwapchainImages.size();

  AttachmentViews.resize(AttachmentCount);
  Attachments.resize(AttachmentCount);
  FrameBuffers.resize(SwapchainImages.size());

  RenderDone.resize(SwapchainImages.size());
  RenderReady.resize(SwapchainImages.size());
  RenderSubmit.resize(SwapchainImages.size());

  VkSemaphoreCreateInfo SemCI{};
  SemCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo FenceCI{};
  FenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  FenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT; // this flag means the fence will be in signaled state at creation.

  uint32_t AttachmentIdx = 0;

  for(uint32_t i = 0; i < SwapchainImages.size(); i++)
  {
    // + 1 to account for depth
    AttachmentIdx = i * (AttachmentInfo.size() + ((DepthAttachment == nullptr) ? 0 : 1));

    for(uint32_t x = 0; x < AttachmentInfo.size(); x++)
    {
      if(x != 0)
      {
        CreateImage(&Attachments[AttachmentIdx+x], AttachmentInfo[x].IMG_Format, AttachmentInfo[x].IMG_Usage);
        CreateImageView(&AttachmentViews[AttachmentIdx+x], &Attachments[AttachmentIdx+x].Image, AttachmentInfo[x].IMG_Format, AttachmentInfo[x].IMG_ViewType, AttachmentInfo[x].IMG_Aspect);
      }
      else
      {
        CreateImageView(&AttachmentViews[AttachmentIdx], &SwapchainImages[i], RenderFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
      }
    }

    if(DepthAttachment != nullptr)
    {
      CreateImage(&Attachments[AttachmentIdx + AttachmentInfo.size()], DepthAttachment->IMG_Format, DepthAttachment->IMG_Usage);
      CreateImageView(&AttachmentViews[AttachmentIdx + AttachmentInfo.size()], &Attachments[AttachmentIdx + AttachmentInfo.size()].Image, DepthAttachment->IMG_Format, DepthAttachment->IMG_ViewType, DepthAttachment->IMG_Aspect);
    }

    VkFramebufferCreateInfo FrameBufferCI{};
    FrameBufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FrameBufferCI.width = Width;
    FrameBufferCI.height = Height;
    FrameBufferCI.renderPass = Renderpass;
    FrameBufferCI.attachmentCount = AttachmentInfo.size() + ((DepthAttachment == nullptr) ? 0 : 1);
    FrameBufferCI.pAttachments = &AttachmentViews[AttachmentIdx];
    FrameBufferCI.layers = 1;

    if((Res = vkCreateFramebuffer(Device, &FrameBufferCI, nullptr, &FrameBuffers[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create framebuffer with error: " + std::to_string(Res));
    }

    if((Res = vkCreateSemaphore(Device, &SemCI, nullptr, &RenderSubmit[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create syncing structure for framebuffer at index " + std::to_string(i) + " with error " + std::to_string(Res));
    }

    if((Res = vkCreateSemaphore(Device, &SemCI, nullptr, &RenderReady[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create syncing structure for framebuffer at index " + std::to_string(i) + " with error " + std::to_string(Res));
    }

    if((Res = vkCreateFence(Device, &FenceCI, nullptr, &RenderDone[i])) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create syncing structure for framebuffer at index " + std::to_string(i) + " with error " + std::to_string(Res));
    }
  }
}


