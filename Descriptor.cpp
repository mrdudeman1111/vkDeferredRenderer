/*
#include <Descriptor.h>
#include <complex>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan_core.h>

#include <set>

namespace Ek
{
  void DescriptorLayoutRecipe::AddResource(ShaderBinding ResourceBinding)
  {
    LayoutInfo.bindingCount++;
    Bindings.push_back(ResourceBinding);
  }

  DescriptorLayout DescriptorLayoutRecipe::Build()
  {
    VkResult Res;
    DescriptorLayout Ret;
    Ret.Bindings = Bindings;

    VkDescriptorSetLayoutCreateInfo LayoutCI{};
    LayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    LayoutCI.bindingCount = Bindings.size();

    VkDescriptorSetLayoutBinding tmpBindings[Bindings.size()];

    for(uint32_t i = 0; i < Bindings.size(); i++)
    {
      tmpBindings[i].binding = Bindings[i].Binding;
      tmpBindings[i].stageFlags = Bindings[i].ShaderStage;
      tmpBindings[i].descriptorType = Bindings[i].Type;
      tmpBindings[i].descriptorCount = 1;
    }

    if((Res = vkCreateDescriptorSetLayout(*pDevice, &LayoutCI, nullptr, &Ret.Layout)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor layout with error: " + std::to_string(Res));
    }

    return Ret;
  }

  void DescriptorPool::AddLayout(DescriptorLayout Layout)
  {
    Layouts.push_back(Layout);

    for(uint32_t i = 0; i < Layout.Bindings.size(); i++)
    {
      VkDescriptorPoolSize Size;
      Size.type = Layout.Bindings[i].descriptorType;
      Size.descriptorCount = 1;
      Sizes.push_back(Size);
    }

    SetBindings.insert(SetBindings.end(), Layout.Bindings.begin(), Layout.Bindings.end());
  }

  void DescriptorPool::UpdatePool()
  {
    VkResult Res;

    if(Pool != VK_NULL_HANDLE)
    {
      vkDestroyDescriptorPool(*pDevice, Pool, nullptr);
    }

    VkDescriptorPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.maxSets = Layouts.size();
    PoolInfo.poolSizeCount = Sizes.size();
    PoolInfo.pPoolSizes = Sizes.data();

    if((Res = vkCreateDescriptorPool(*pDevice, &PoolInfo, nullptr, &Pool)) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to create descriptor pool with error: " + std::to_string(Res));
    }

    VkDescriptorSetAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocInfo.descriptorPool = Pool;
    AllocInfo.descriptorSetCount = Layouts.size();

    std::vector<VkDescriptorSetLayout> pLayouts;
    for(uint32_t i = 0; i < Layouts.size(); i++)
    {
      pLayouts.push_back(Layouts[i].Layout);
    }

    AllocInfo.pSetLayouts = pLayouts.data();

    if((Res = vkAllocateDescriptorSets(*pDevice, &AllocInfo, Sets.data())) != VK_SUCCESS)
    {
      throw std::runtime_error("Failed to allocate descriptor sets with error: " + std::to_string(Res));
    }
  }

  void DescriptorPool::UpdateDescriptor(uint32_t Binding)
  {
    VkWriteDescriptorSet Write;
    Write.
  }
}
*/
