/*
#include <map>
#include <set>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Ek
{
  class DescriptorPool;
  class DescriptorSetLayoutRecipe;

  struct ShaderBinding
  {
  public:
    uint32_t Binding;
    uint32_t BindingCount;
    VkDescriptorType Type;
    VkShaderStageFlags ShaderStage;
  };

  class DescriptorLayout
  {
    friend DescriptorPool;
    friend DescriptorSetLayoutRecipe;
  public:
    VkDescriptorSetLayout Layout;
    std::vector<ShaderBinding> Bindings;
  };

  class DescriptorLayoutRecipe
  {
  public:
    DescriptorLayoutRecipe(VkDevice* Device) : pDevice(Device)
    {
      LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      LayoutInfo.bindingCount = 0;
    }

    // This adds a slot for the resource, It's your job to hold on to the resource till this descriptor is updated. And make sure you hold on to that descriptor set.
    void AddResource(ShaderBinding ResourceBinding);

    DescriptorLayout Build();
  private:
    VkDevice* pDevice;

    VkDescriptorSetLayoutCreateInfo LayoutInfo{};

    // bindings have everything we need for pool sizes.
    std::vector<ShaderBinding> Bindings;
  };

  class DescriptorPool
  {
  public:
    DescriptorPool(VkDevice* Device) : pDevice(Device)
    {}

    void AddLayout(DescriptorLayout Layout);

    void UpdatePool();

    void UpdateDescriptor();
  private:
    VkDevice* pDevice;

    VkDescriptorPool Pool;

    std::vector<VkDescriptorSetLayoutBinding> SetBindings;
    std::vector<VkDescriptorSet> Sets;

    std::vector<DescriptorLayout> Layouts;
    std::vector<VkDescriptorPoolSize> Sizes;
  };
}
*/
