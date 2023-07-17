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

  Renderer.Init(1280, 720);

  Renderer.RequestDevExt("VK_KHR_swapchain");
  Renderer.CreateDevice();

  Renderer.CreateCmdBuffers();
  Renderer.CreateSwapchain(VK_PRESENT_MODE_IMMEDIATE_KHR);
  Renderer.CreateRenderPass();
  Renderer.CreateFrameBuffers();

  std::cout << "Success\n";
}
