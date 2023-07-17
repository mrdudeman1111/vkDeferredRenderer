#include <iostream>
#include <string>
#include <vector>

#include <Pipeline.h>
#include <vulkan/vulkan_core.h>

void Pipeline::Init()
{
  VkResult Res;

  VkPipelineLayoutCreateInfo LayoutInfo{};
  LayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  LayoutInfo.setLayoutCount = Recipe.DescriptorLayouts.size();
  LayoutInfo.pSetLayouts = Recipe.DescriptorLayouts.data();

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
  if(Recipe.pDepthStencilInfo)
  {
    PipeInfo.pDepthStencilState = Recipe.pDepthStencilInfo;
  }

  PipeInfo.pInputAssemblyState = &InputAssembly;
  PipeInfo.pVertexInputState = &vInputState;

  if(Recipe.TesselationState)
  {
    PipeInfo.pTessellationState = Recipe.TesselationState;
  }
  PipeInfo.pRasterizationState = &Raster;

  if((Res = vkCreateGraphicsPipelines(*pDevice, VK_NULL_HANDLE, 1, &PipeInfo, nullptr, &Pipeline)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create pipeline with error: " + std::to_string(Res));
  }
}

void Pipeline::BakeRecipe(uint32_t inWidth, uint32_t inHeight, VkRenderPass* pRP, ePassType Type)
{
  VkResult Res;

  pRenderPass = pRP;

  VkDescriptorPoolCreateInfo PoolCI{};
  PoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

  uint32_t SizeCount = 0;
  PoolCI.pPoolSizes = Mat.GetSizes(2048, &SizeCount);
  PoolCI.poolSizeCount = SizeCount;

  PoolCI.maxSets = 2048;

  if((Res = vkCreateDescriptorPool(*pDevice, &PoolCI, nullptr, &Pool)) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create descriptor pool with error: " + std::to_string(Res));
  }

  Recipe.StageInfos = {{},{},{}};
  Recipe.StageInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  Recipe.StageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  Recipe.StageInfos[0].pName = Mat.VertShader->EntryPoint;
  Recipe.StageInfos[0].module = Mat.VertShader->sModule;

  Recipe.StageInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  Recipe.StageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  Recipe.StageInfos[1].pName = Mat.FragShader->EntryPoint;
  Recipe.StageInfos[1].module = Mat.FragShader->sModule;
  if(Mat.GeomShader)
  {
    Recipe.StageInfos[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    Recipe.StageInfos[2].stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    Recipe.StageInfos[2].pName = Mat.GeomShader->EntryPoint;
    Recipe.StageInfos[2].module = Mat.GeomShader->sModule;
  }

  Recipe.StageInfos.shrink_to_fit();

  Vertex V;
  Recipe.inBindings = V.GetBindingDescription();
  Recipe.inAttribs = V.GetAttributeDescription();

  Recipe.pDepthStencilInfo = new VkPipelineDepthStencilStateCreateInfo();
  Recipe.pDepthStencilInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  Recipe.pDepthStencilInfo->depthTestEnable = VK_TRUE;
  Recipe.pDepthStencilInfo->depthWriteEnable = VK_TRUE;
  Recipe.pDepthStencilInfo->depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  Recipe.pDepthStencilInfo->depthBoundsTestEnable = VK_FALSE;
  Recipe.pDepthStencilInfo->minDepthBounds = 0.0f;
  Recipe.pDepthStencilInfo->maxDepthBounds = 1.0f;
  Recipe.pDepthStencilInfo->stencilTestEnable = VK_FALSE;

  Recipe.MultiSample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  Recipe.MultiSample.sampleShadingEnable = VK_FALSE;
  Recipe.MultiSample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState ColorAttachment;
  ColorAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  ColorAttachment.blendEnable = VK_FALSE;

  Recipe.ColorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  Recipe.ColorBlend.logicOp = VK_LOGIC_OP_COPY;
  Recipe.ColorBlend.logicOpEnable = VK_FALSE;
  Recipe.ColorBlend.logicOp = VK_LOGIC_OP_COPY;
  Recipe.ColorBlend.attachmentCount = 1;
  Recipe.ColorBlend.pAttachments = &ColorAttachment;
  Recipe.ColorBlend.blendConstants[0] = 0.0f;
  Recipe.ColorBlend.blendConstants[1] = 0.0f;
  Recipe.ColorBlend.blendConstants[2] = 0.0f;
  Recipe.ColorBlend.blendConstants[3] = 0.0f;

  VkRect2D Scissor;
  Scissor.extent.width = inWidth;
  Scissor.extent.height = inWidth;
  Scissor.offset.x = 0;
  Scissor.offset.y = 0;

  Recipe.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  Recipe.ViewportInfo.scissorCount = 1;
  Recipe.ViewportInfo.pScissors = &Scissor;
  Recipe.ViewportInfo.viewportCount = 1;
  Recipe.ViewportInfo.pViewports = &Recipe.Viewport;

  PassType = Type;
}

void Material::AddTexture(Texture* pTex, VkDescriptorSetLayoutBinding Binding, VkDescriptorType Type)
{
  pTextures.push_back(pTex);

  VkDescriptorPoolSize Size;
  Size.type = Type;
  Size.descriptorCount = 1;

  LayoutCI.bindingCount++;

  Bindings.push_back(Binding);
}

void Material::AddBuffer(Buffer* pBuff, VkDescriptorSetLayoutBinding Binding, VkDescriptorType Type)
{
  pBuffers.push_back(pBuff);

  VkDescriptorPoolSize Size;
  Size.type = Type;
  Size.descriptorCount = 1;

  LayoutCI.bindingCount++;

  Bindings.push_back(Binding);
}

VkDescriptorPoolSize* Material::GetSizes(uint32_t MaxSets, uint32_t* pSizeCount)
{
  for(VkDescriptorPoolSize& Size : Sizes)
  {
    Size.descriptorCount *= MaxSets;
    pSizeCount += Size.descriptorCount;
  }

  return Sizes.data();
}

