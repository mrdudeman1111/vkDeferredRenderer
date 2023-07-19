#include <GLFW/glfw3.h>
#include <iostream>
#include <Renderer.h>
#include <vulkan/vulkan_core.h>


// idea: seperate memory blocks, so have "slots" for different locations where textures and buffers can be stored within memory blocks.

int main()
{
  Renderer Renderer;

  Renderer.RequestInstLayer("VK_LAYER_KHRONOS_validation");

  Renderer.RequestInstExt("VK_KHR_surface");
  Renderer.RequestInstExt("VK_EXT_debug_report");

  Renderer.Init(1280, 720);

  Renderer.RequestDevExt("VK_KHR_swapchain");
  Renderer.CreateDevice();

  Renderer.CreateCmdBuffers();
  Renderer.CreateSwapchain(VK_PRESENT_MODE_IMMEDIATE_KHR);
  Renderer.CreateRenderPass();
  Renderer.CreateFrameBuffers();

  Material Mat = Renderer.CreateMat();

  Shader Vert = Renderer.CreateShader(ShaderDir"Vert.spv", "main");
  Shader Frag = Renderer.CreateShader(ShaderDir"Frag.spv", "main");

  Mat.VertShader = &Vert;
  Mat.FragShader = &Frag;

  VkDescriptorSetLayoutBinding CameraBinding;
  CameraBinding.binding = 0;
  CameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  CameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  CameraBinding.descriptorCount = 1;

  Mat.AddResource(CameraBinding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

  std::cout << "Success\n";
}
