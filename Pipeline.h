#pragma once

#include <cstddef>
#include <glm/fwd.hpp>
#include <iostream>
#include <vector>
#include <vulkan/vulkan_core.h>

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
    glm::vec2 uvCoord = {0.f, 0.f};
    glm::ivec4 BoneIDs = {-1, -1, -1, -1};
    glm::fvec4 BoneWeights{0.f, 0.f, 0.f, 0.f};

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
    std::vector<VkVertexInputAttributeDescription> Ret(5);
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

    Ret[3].binding = 0;
    Ret[3].format = VK_FORMAT_R8G8B8A8_SINT;
    Ret[3].offset = offsetof(Vertex, BoneIDs);
    Ret[3].location = 3;

    Ret[4].binding = 0;
    Ret[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    Ret[4].location = 4;
    Ret[4].offset = offsetof(Vertex, BoneWeights);

    return Ret;
  }
};

class PipelineRecipe
{
public:
  PipelineRecipe() : ViewportInfo{}, MultiSample{}, ColorBlend{}, Viewport{}
  {
  }

  std::vector<VkPipelineShaderStageCreateInfo> StageInfos;

  VkPipelineViewportStateCreateInfo ViewportInfo;
  VkPipelineMultisampleStateCreateInfo MultiSample;
  VkPipelineColorBlendStateCreateInfo ColorBlend;
  VkPipelineTessellationStateCreateInfo TesselationState;
  VkPipelineDepthStencilStateCreateInfo pDepthStencilInfo;

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

  Shader *VertShader = nullptr, *FragShader = nullptr, *GeomShader = nullptr;

  void AddResource(VkDescriptorSetLayoutBinding Binding);

  VkDescriptorPoolSize* GetSizes(uint32_t* pSizeCount);
  VkDescriptorSetLayout GetLayout();

  bool HasSkeleton = false;

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
    Pipeline(VkDevice* pDev, VkRenderPass* pRP);

    PipelineRecipe Recipe;
    VkPipelineLayout PipeLayout;
    VkPipeline Pipe;

    void Init(VkDescriptorSetLayout CameraLayout);
    void BakeRecipe(uint32_t Width, uint32_t Height, ePassType Type);
    VkDescriptorSet CreateDescriptor();

    Material Mat;
  private:
    VkDevice* pDevice;

    VkDescriptorPool Pool;
    ePassType PassType;
    VkRenderPass* pRenderPass;
};

class MaterialInstance
{
  public:
};

