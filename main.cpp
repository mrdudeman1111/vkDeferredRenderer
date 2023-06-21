#include <GLFW/glfw3.h>
#include <iostream>
#include <Renderer.h>
#include <vulkan/vulkan_core.h>

int main()
{
  Renderer R;

  R.RequestInstLayer("VK_LAYER_KHRONOS_validation");
  R.RequestInstExt("VK_KHR_surface");

  R.Init(1280, 720);

  R.RequestDevExt("VK_KHR_swapchain");
  R.CreateDevice();

  //R.CreateCmdBuffers();
  //R.CreateSwapchain(VK_PRESENT_MODE_IMMEDIATE_KHR);
  //R.CreateRenderPass();
  //R.CreateFrameBuffers();

  std::cout << "Success\n";
}
