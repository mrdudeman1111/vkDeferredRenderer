#pragma once

#include <cstddef>
#include <iostream>
#include <vector>

#define GLFW_FORCE_RADIANS
#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

#include <map>

class Material;
class Renderer;

enum ePassType
{
  ePrimary = 0,
  eLighting = 1
};

class Vertex
{
public:
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 uvCoord;

  std::vector<VkVertexInputBindingDescription> GetBindingDescription()
  {
    std::vector<VkVertexInputBindingDescription> Ret(1);

    Ret[0].stride = sizeof(Vertex);
    Ret[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    Ret[0].binding = 0;

    return Ret;
  }

  std::vector<VkVertexInputAttributeDescription> GetAttributeDescription()
  {
    std::vector<VkVertexInputAttributeDescription> Ret(3);
    Ret[0].binding = 0;
    Ret[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    Ret[0].offset = offsetof(Vertex, Position);
    Ret[0].location = 0;

    Ret[1].binding = 0;
    Ret[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    Ret[1].offset = offsetof(Vertex, Normal);
    Ret[1].location = 1;

    Ret[2].binding = 0;
    Ret[2].format = VK_FORMAT_R32G32_SFLOAT;
    Ret[2].offset = offsetof(Vertex, uvCoord);
    Ret[2].location = 2;

    return Ret;
  }
};

class PipelineRecipe
{
public:
  std::vector<VkPipelineShaderStageCreateInfo> StageInfos;

  VkPipelineViewportStateCreateInfo ViewportInfo;
  VkPipelineMultisampleStateCreateInfo MultiSample;
  VkPipelineColorBlendStateCreateInfo ColorBlend;
  VkPipelineTessellationStateCreateInfo* TesselationState;
  VkPipelineDepthStencilStateCreateInfo* pDepthStencilInfo;

  VkViewport Viewport;

  std::vector<VkVertexInputAttributeDescription> inAttribs;
  std::vector<VkVertexInputBindingDescription> inBindings;
};

class Shader
{
public:
  VkShaderModule sModule;
  const char* EntryPoint;
};

class Material
{
  friend Renderer;
public:
  Material(){}
  Material(VkDevice* pDev) : pDevice(pDev) {}

  Shader* VertShader, *FragShader, *GeomShader;

  void AddResource(VkDescriptorSetLayoutBinding Binding, VkDescriptorType Type);

  VkDescriptorPoolSize* GetSizes(uint32_t MaxSets, uint32_t* pSizeCount);
  VkDescriptorSetLayout GetLayout();

private:
  VkDevice* pDevice;

  std::vector<VkDescriptorSetLayoutBinding> Bindings;

  std::vector<VkDescriptorPoolSize> Sizes;
};

class Renderer;

class Pipeline
{
  friend Renderer;
public:
  Pipeline(VkDevice* pDev);

  PipelineRecipe Recipe;
  VkPipelineLayout PipeLayout;
  VkPipeline Pipe;

  void Init();
  void BakeRecipe(uint32_t Width, uint32_t Height, VkRenderPass* pRP, ePassType Type);

private:
  VkDevice* pDevice;
  Material Mat;

  VkDescriptorPool Pool;
  ePassType PassType;
  VkRenderPass* pRenderPass;
};

class DescriptorSet
{
public:
  ~DescriptorSet()
  {
    vkFreeDescriptorSets(*pDevice, *pPool, 1, &Set);
  }

private:
  VkDescriptorSet Set;

  VkDevice* pDevice;
  VkDescriptorPool* pPool;
};

class MaterialInstance
{};

