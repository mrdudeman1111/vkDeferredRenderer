#include <GLFW/glfw3.h>
#include <iostream>
#include <Renderer.h>

int main()
{
  Renderer R;

  R.RequestInstLayer("VK_LAYER_KHRONOS_validation");
  R.RequestInstExt("VK_KHR_surface");

  R.Init();

  R.RequestDevExt("VK_KHR_swapchain");
  R.CreateDevice();

  std::cout << "Success\n";
}
