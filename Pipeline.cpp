#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <Pipeline.h>
#include <vulkan/vulkan_core.h>

Pipeline::Pipeline(VkDevice* pDev, VkRenderPass* pRP) : pDevice(pDev), pRenderPass(pRP)
{}

void Pipeline::Init(VkDescriptorSetLayout CameraLayout)
{
  VkResult Res;

  VkPipelineLayoutCreateInfo LayoutInfo{};
  LayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

  VkDescriptorSetLayout DescLay = Mat.GetLayout();
  VkDescriptorSetLayout Layouts[2] = { CameraLayout, DescLay };

  LayoutInfo.setLayoutCount = 2;
  LayoutInfo.pSetLayouts = Layouts;

  if((Res = vkCreatePipelineLayout(*pDevice, &LayoutInfo, nullptr, &PipeLayout)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create pipeline layout with error: " + std::to_string(Res));
  }

  VkPipelineVertexInputStateCreateInfo vInputState{};
  vInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vInputState.vertexBindingDescriptionCount = Recipe.inBindings.size();
  vInputState.pVertexBindingDescriptions = Recipe.inBindings.data();
  vInputState.vertexAttributeDescriptionCount = Recipe.inAttribs.size();
  vInputState.pVertexAttributeDescriptions = Recipe.inAttribs.data();
  vInputState.vertexBindingDescriptionCount = Recipe.inBindings.size();
  vInputState.pVertexBindingDescriptions = Recipe.inBindings.data();

  VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
  InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  InputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineRasterizationStateCreateInfo Raster{};
  Raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  Raster.depthClampEnable = VK_FALSE;
  Raster.rasterizerDiscardEnable = VK_FALSE;
  Raster.polygonMode = VK_POLYGON_MODE_FILL;
  Raster.lineWidth = 1.f;
  Raster.cullMode = VK_CULL_MODE_BACK_BIT;
  Raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  Raster.depthBiasEnable = VK_FALSE;

  VkGraphicsPipelineCreateInfo PipeInfo{};
  PipeInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  PipeInfo.layout = PipeLayout;
  PipeInfo.stageCount = Recipe.StageInfos.size();
  PipeInfo.pStages = Recipe.StageInfos.data();
  PipeInfo.subpass = PassType;
  PipeInfo.renderPass = *pRenderPass;
  PipeInfo.pViewportState = &Recipe.ViewportInfo;
  PipeInfo.pColorBlendState = &Recipe.ColorBlend;
  PipeInfo.pMultisampleState = &Recipe.MultiSample;

  if(Recipe.pDepthStencilInfo.sType == VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO) // if the stype is filled, we have added this structure to the recipe
  {
    PipeInfo.pDepthStencilState = &Recipe.pDepthStencilInfo;
  }

  PipeInfo.pInputAssemblyState = &InputAssembly;
  PipeInfo.pVertexInputState = &vInputState;

  if(Recipe.TesselationState.sType == VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO) // if the stype is filled, we have added this structure to the recipe
  {
    PipeInfo.pTessellationState = &Recipe.TesselationState;
  }
  PipeInfo.pRasterizationState = &Raster;

  if((Res = vkCreateGraphicsPipelines(*pDevice, VK_NULL_HANDLE, 1, &PipeInfo, nullptr, &Pipe)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create pipeline with error: " + std::to_string(Res));
  }
}

void Pipeline::BakeRecipe(uint32_t inWidth, uint32_t inHeight, int X, int Y, eSpace SpaceType, ePassType Type, std::vector<PipelineAttachment> Attachments)
{
  VkResult Res;

  VkDescriptorPoolCreateInfo PoolCI{};
  PoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

  uint32_t SizeCount = 0;
  PoolCI.pPoolSizes = Mat.GetSizes(&SizeCount);
  PoolCI.poolSizeCount = SizeCount;

  PoolCI.maxSets = 2048;

  if((Res = vkCreateDescriptorPool(*pDevice, &PoolCI, nullptr, &Pool)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor pool with error: " + std::to_string(Res));
  }

  Recipe.StageInfos = {{},{}};
  Recipe.StageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  Recipe.StageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  Recipe.StageInfos[0].pName = Mat.VertShader->EntryPoint.data();
  Recipe.StageInfos[0].module = Mat.VertShader->sModule;

  Recipe.StageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  Recipe.StageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  Recipe.StageInfos[1].pName = Mat.FragShader->EntryPoint.data();
  Recipe.StageInfos[1].module = Mat.FragShader->sModule;

  if(Mat.GeomShader)
  {
    Recipe.StageInfos.push_back({});
    Recipe.StageInfos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    Recipe.StageInfos[2].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    Recipe.StageInfos[2].pName = Mat.GeomShader->EntryPoint.data();
    Recipe.StageInfos[2].module = Mat.GeomShader->sModule;
  }

  Recipe.StageInfos.shrink_to_fit();

  if(SpaceType == e3D)
  {
    Vertex V;
    Recipe.inBindings = V.GetBindingDescription();
    Recipe.inAttribs = V.GetAttributeDescription();
  }
  else if(SpaceType == e2D)
  {
    Vertex2D V;
    Recipe.inBindings = V.GetBindingDescription();
    Recipe.inAttribs = V.GetAttributeDescription();
  }
  else
  {
    std::cout << "what space type is this pipeline!?\n";
  }

  Recipe.pDepthStencilInfo = {}; // fill with defaults
  Recipe.pDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  Recipe.pDepthStencilInfo.depthTestEnable = VK_TRUE;
  Recipe.pDepthStencilInfo.depthWriteEnable = VK_TRUE;
  Recipe.pDepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  Recipe.pDepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
  Recipe.pDepthStencilInfo.minDepthBounds = 0.0f;
  Recipe.pDepthStencilInfo.maxDepthBounds = 1.0f;
  Recipe.pDepthStencilInfo.stencilTestEnable = VK_FALSE;

  Recipe.MultiSample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  Recipe.MultiSample.sampleShadingEnable = VK_FALSE;
  Recipe.MultiSample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState* ColorAttachment = new VkPipelineColorBlendAttachmentState[Attachments.size()]{};

  for(uint32_t i = 0; i < Attachments.size(); i++)
  {
    if(Attachments[i].Transparent)
    {
      // this will write over full RGBA.
        ColorAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

      // Fully overwrite areas being BLIT ed to the final image.
        ColorAttachment[i].colorBlendOp = VK_BLEND_OP_SRC_EXT;
        ColorAttachment[i].alphaBlendOp = VK_BLEND_OP_SRC_EXT;

      // Blend areas with only a factor of more than zero. else, it is not effected.
        ColorAttachment[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        ColorAttachment[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;

      // Hard code to prevent the other output buffers from being affected from rendering UI
        ColorAttachment[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        ColorAttachment[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;

      ColorAttachment[i].blendEnable = VK_TRUE;
    }
    else
    {
      ColorAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      ColorAttachment[i].blendEnable = VK_FALSE;
    }
  }

/*
  ColorAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  ColorAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
  ColorAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
  ColorAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  ColorAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  ColorAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;
*/

  // only the color buffer [0] has any weight to our presented image, the rest are just lighting info

  Recipe.ColorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  Recipe.ColorBlend.logicOp = VK_LOGIC_OP_COPY;
  Recipe.ColorBlend.logicOpEnable = VK_FALSE;
  Recipe.ColorBlend.logicOp = VK_LOGIC_OP_COPY;
  Recipe.ColorBlend.attachmentCount = Attachments.size();
  Recipe.ColorBlend.pAttachments = ColorAttachment;

  // this is a constant blend factor, which means that this will be the minimum blend factor for EVERY attachment in the framebuffer,
  // meaning the output color will be a blend of every attachment's corresponding color.
    Recipe.ColorBlend.blendConstants[0] = 0.0f;
    Recipe.ColorBlend.blendConstants[1] = 0.0f;
    Recipe.ColorBlend.blendConstants[2] = 0.0f;
    Recipe.ColorBlend.blendConstants[3] = 0.0f;

  VkRect2D* Scissor = new VkRect2D;
  Scissor->extent.width = inWidth;
  Scissor->extent.height = inHeight;
  Scissor->offset.x = X;
  Scissor->offset.y = Y;

  Recipe.Viewport.width = inWidth;
  Recipe.Viewport.height = inHeight;
  Recipe.Viewport.minDepth = 0.f;
  Recipe.Viewport.maxDepth = 1.f;
  Recipe.Viewport.x = X;
  Recipe.Viewport.y = Y;

  Recipe.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  Recipe.ViewportInfo.scissorCount = 1;
  Recipe.ViewportInfo.pScissors = Scissor;
  Recipe.ViewportInfo.viewportCount = 1;
  Recipe.ViewportInfo.pViewports = &Recipe.Viewport;

  PassType = Type;
}

VkDescriptorSet Pipeline::CreateDescriptor()
{
  VkResult Res;

  VkDescriptorSet Set;

  VkDescriptorSetLayout Lay = Mat.GetLayout();

  VkDescriptorSetAllocateInfo AllocInfo{};
  AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  AllocInfo.descriptorPool = Pool;
  AllocInfo.descriptorSetCount = 1;
  AllocInfo.pSetLayouts = &Lay;

  if((Res = vkAllocateDescriptorSets(*pDevice, &AllocInfo, &Set)) != VK_SUCCESS)
  {
    switch(Res)
    {
      case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        std::cout << "ran out of pool memory when allocating a descriptor set from a pipeline, ran out of DEVICE_MEMORY\n";
        break;
      case VK_ERROR_OUT_OF_POOL_MEMORY:
        std::cout << "ran out of pool memory when allocating a descriptor set from a pipeline, ran out of POOL_MEMORY\n";
        break;
      case VK_ERROR_OUT_OF_HOST_MEMORY:
        std::cout << "ran out of pool memory when allocating a descriptor set from a pipeline, ran out of HOST_MEMORY\n";
        break;
      default:
        throw std::runtime_error("Failed to allocate descriptor set with error: " + std::to_string(Res));
    }
  }

  return Set;
}

void Material::AddResource(VkDescriptorSetLayoutBinding Binding)
{
  VkDescriptorPoolSize Size;
  Size.type = Binding.descriptorType;
  Size.descriptorCount = 1;

  Bindings.push_back(Binding);
}

VkDescriptorPoolSize* Material::GetSizes(uint32_t* pSizeCount)
{
  for(VkDescriptorSetLayoutBinding& Binding : Bindings)
  {
    VkDescriptorPoolSize Size;
    Size.descriptorCount = Binding.descriptorCount;
    Size.type = Binding.descriptorType;

    Sizes.push_back(Size);

    *pSizeCount += Size.descriptorCount;
  }

  return Sizes.data();
}

VkDescriptorSetLayout Material::GetLayout()
{
  VkResult Res;

  VkDescriptorSetLayout Ret;

  VkDescriptorSetLayoutCreateInfo LayoutCI{};
  LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  LayoutCI.bindingCount = Bindings.size();
  LayoutCI.pBindings = Bindings.data();

  if((Res = vkCreateDescriptorSetLayout(*pDevice, &LayoutCI, nullptr, &Ret)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create layout in Material::GetLayout() with error " + std::to_string(Res));
  }

  return Ret;
}

