#include <Renderer.h>
#include <assimp/material.h>
#include <vulkan/vulkan_core.h>

void SortAtt(std::vector<EkFrameBufferAttachment>* Atts, EkFrameBufferAttachment* tmp, uint32_t i)
{
  if(Atts->at(i).Location != i)
  {
    *tmp = Atts->at(Atts->at(i).Location);
    Atts->at(Atts->at(i).Location) = Atts->at(i);
    Atts->at(i) = *tmp;

    if(Atts->at(i).Location != i)
    {
      SortAtt(Atts, tmp, i);
    }
  }
}

void EkSubpass::AddAttachment(uint32_t Location, eAttachmentType AttType)
{
  Attachments[Location] = AttType;
}

void Renderer::CreateRenderPass(EkSubpass* Subpasses, uint32_t SubpassCount)
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


  /*
      // Color
      Descriptions[0].format = SurfaceFormat.format;
      Descriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      Descriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      Descriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
      Descriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      Descriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      Descriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      Descriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

      // Pos
      Descriptions[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
      Descriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      Descriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      Descriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      Descriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      Descriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      Descriptions[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      Descriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;

      // Normal
      Descriptions[2].format = VK_FORMAT_R16G16B16A16_SFLOAT;
      Descriptions[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      Descriptions[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
      Descriptions[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      Descriptions[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
      Descriptions[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      Descriptions[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      Descriptions[2].samples = VK_SAMPLE_COUNT_1_BIT;

      // Depth
      Descriptions[3].format = VK_FORMAT_D16_UNORM;
      Descriptions[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      Descriptions[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      Descriptions[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      Descriptions[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      Descriptions[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
      Descriptions[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
      Descriptions[3].samples = VK_SAMPLE_COUNT_1_BIT;
    // Defining the structure of the framebuffer / Framebuffer specific 
  */

  /*
    AttRefs[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    AttRefs[0].attachment = 0;

    AttRefs[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    AttRefs[1].attachment = 1;

    AttRefs[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    AttRefs[2].attachment = 2;

    pDepth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    pDepth.attachment = 3;
  */


    VkAttachmentDescription Descriptions[ (DepthAttachment == nullptr) ? AttachmentInfo.size() : AttachmentInfo.size() + 1 ]{};
    VkAttachmentReference References[ (DepthAttachment == nullptr) ? AttachmentInfo.size() : AttachmentInfo.size() ]{};

  // Sort
    EkFrameBufferAttachment tmpSort;

    for(uint32_t i = 0; i < AttachmentInfo.size(); i++)
    {
      SortAtt(&AttachmentInfo, &tmpSort, i);
    }
  // Sort

  // Attachments
    for(uint32_t i = 0; i < AttachmentInfo.size(); i++)
    {
      Descriptions[i] = AttachmentInfo[i].FB_Description;
      References[i].layout = AttachmentInfo[i].FB_Layout;
      References[i].attachment = AttachmentInfo[i].Location;
    }

    VkAttachmentReference RefDepth;

    if(DepthAttachment)
    {
      Descriptions[DepthAttachment->Location] = DepthAttachment->FB_Description;
      RefDepth.attachment = AttachmentInfo.size();
      RefDepth.layout = DepthAttachment->FB_Layout;
    }
  // Attachments

  // Subpass Descriptions
    VkSubpassDescription SubpassDescs[SubpassCount]{};

    std::vector<VkAttachmentReference> Color;
    uint32_t ColorIdx = 0;
    std::vector<VkAttachmentReference> Input;
    uint32_t InputIdx = 0;
    std::vector<VkAttachmentReference> Resolve;
    uint32_t ResolveIdx = 0;
    std::vector<uint32_t> Preserve;
    uint32_t PreserveIdx = 0;

    for(uint32_t i = 0; i < SubpassCount; i++)
    {
      SubpassDescs[i].pipelineBindPoint = Subpasses[i].BindPoint;
      Subpasses[i].AddAttachment(0, eColor);

      uint32_t numColor = 0, numInput = 0, numResolve = 0, numPreserve = 0;

      for(uint32_t x = 0; x < ((DepthAttachment == nullptr) ? AttachmentInfo.size() : AttachmentInfo.size() + 1); x++)
      {
        if(Subpasses[i].Attachments[x] == eColor)
        {
          Color.push_back(References[x]);
          numColor++;
        }
        else if(Subpasses[i].Attachments[x] == eInput)
        {
          Input.push_back(References[x]);
          numInput++;
        }
        else if(Subpasses[i].Attachments[x] == eResolve)
        {
          Resolve.push_back(References[x]);
          numResolve++;
        }
        else if(Subpasses[i].Attachments[x] == ePreserve)
        {
          Preserve.push_back(AttachmentInfo[x].Location);
          numPreserve++;
        }
        else if(Subpasses[i].Attachments[x] == eDepth)
        {
          SubpassDescs[i].pDepthStencilAttachment = &RefDepth;
        }
      }

      SubpassDescs[i].colorAttachmentCount = numColor;
      SubpassDescs[i].pColorAttachments = (numColor == 0) ? nullptr : &Color[ColorIdx];
      SubpassDescs[i].inputAttachmentCount = numInput;
      SubpassDescs[i].pInputAttachments = (numInput == 0) ? nullptr : &Input[InputIdx];
      SubpassDescs[i].pResolveAttachments = (numColor == 0 || numResolve == 0) ? nullptr : &Resolve[ResolveIdx];
      SubpassDescs[i].preserveAttachmentCount = numPreserve;
      SubpassDescs[i].pPreserveAttachments = (numPreserve == 0) ? nullptr : &Preserve[PreserveIdx];
      SubpassDescs[i].flags = 0;

      ColorIdx += numColor;
      InputIdx += numInput;
      ResolveIdx += numResolve;
      PreserveIdx += numPreserve;
    }
  // Subpass Descriptions

  // Subpass Dependencies
    std::vector<VkSubpassDependency> Dependencies;

    for(uint32_t i = 0; i < SubpassCount; i++)
    {
      if(Subpasses[i].Dependencies.size() != 0)
      {
        for(uint32_t x = 0; x < Subpasses[i].Dependencies.size(); x++)
        {
          VkSubpassDependency Dep{};

          Dep.srcSubpass = i;
          Dep.dstSubpass = Subpasses[i].Dependencies[x].dstSubpass;
          Dep.srcStageMask = Subpasses[i].Dependencies[x].srcStage;
          Dep.dstStageMask = Subpasses[i].Dependencies[x].dstStage;
          Dep.srcAccessMask = Subpasses[i].Dependencies[x].srcAccess;
          Dep.dstAccessMask = Subpasses[i].Dependencies[x].dstAccess;

          Dependencies.push_back(Dep);
        }
      }
    }
  // Subpass Dependencies

/*
  Subpasses[1] = {};
  Subpasses[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  Subpasses[1].colorAttachmentCount = 3;
  Subpasses[1].pColorAttachments = AttRefs;
  Subpasses[1].pDepthStencilAttachment = &pDepth;
*/

/*
  VkSubpassDependency LightingDependencies{};
  LightingDependencies.srcSubpass = 0;
  LightingDependencies.dstSubpass = 1;
  LightingDependencies.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  LightingDependencies.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  LightingDependencies.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
  LightingDependencies.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
*/
  // the fragment shader from the G-pass needs to finish writing to the Framebuffer before the lighting pass is executed

  VkRenderPassCreateInfo RenderpassInfo{};
  RenderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

  RenderpassInfo.subpassCount = SubpassCount;
  RenderpassInfo.pSubpasses = SubpassDescs;

  RenderpassInfo.dependencyCount = Dependencies.size();
  RenderpassInfo.pDependencies = Dependencies.data();

  RenderpassInfo.attachmentCount = (DepthAttachment == nullptr) ? AttachmentInfo.size() : AttachmentInfo.size() + 1;
  RenderpassInfo.pAttachments = Descriptions;

  if((Res = vkCreateRenderPass(Device, &RenderpassInfo, nullptr, &Renderpass)) != VK_SUCCESS)
  {
    throw std::runtime_error("failed to create renderpass with error: " + std::to_string(Res));
  }

  MakeSceneDescriptorPool();
}

