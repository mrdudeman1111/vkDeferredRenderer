#include "Mesh.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <Renderer.h>
#include <vulkan/vulkan_core.h>


// idea: seperate memory blocks, so have "slots" for different locations where textures and buffers can be stored within memory blocks.



int main()
{
  Renderer Renderer;

  Renderer.RequestInstLayer("VK_LAYER_KHRONOS_validation");

  Renderer.RequestInstExt(VK_KHR_SURFACE_EXTENSION_NAME);
  Renderer.RequestInstExt(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  Renderer.RequestInstExt(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME);

  Renderer.Init(1280, 720);

  Renderer.RequestDevExt("VK_KHR_swapchain");
  Renderer.CreateDevice();

  Renderer.CreateCmdBuffers();
  Renderer.CreateSwapchain(VK_PRESENT_MODE_IMMEDIATE_KHR);
  Renderer.CreateRenderPass();
  Renderer.CreateFrameBuffers();

  Material Mat = Renderer.CreateMat();

  Mat.HasSkeleton = true;

  Shader Vert = Renderer.CreateShader(ShaderDir"Vert.spv", "main");
  Shader Frag = Renderer.CreateShader(ShaderDir"Frag.spv", "main");

  Mat.VertShader = &Vert;
  Mat.FragShader = &Frag;

  VkDescriptorSetLayoutBinding SkeletalBinding{};
  SkeletalBinding.binding = 1;
  SkeletalBinding.descriptorCount = 1;
  SkeletalBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  SkeletalBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  Mat.AddResource(SkeletalBinding);

  Pipeline MainPipe = Renderer.CreatePipeline(Mat);

  MeshComponent Cube;
  Cube.LoadMesh(&Renderer, &MainPipe, ModelDir"Cube.dae");
  Cube.pPipeline = &MainPipe;

  VkCommandBuffer* RenderBuffer = Renderer.GraphicsDispatch.GetRenderBuffer(&Renderer.Device);

  Camera* Cam = Renderer.GetCamera();

  while(!Renderer.ShouldClose())
  {
    VkCommandBufferBeginInfo BeginInf{};
    BeginInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    Renderer.WaitOnLastFrame();

    vkResetCommandBuffer(*RenderBuffer, 0);

    Cam->Update();

    Cube.Update();

    // idea: Have the renderer handle the recording of render buffer, then have user submit meshes, etc
    vkBeginCommandBuffer(*RenderBuffer, &BeginInf);
      Renderer.BeginRenderPass(RenderBuffer);
        Cube.Draw(RenderBuffer);
      Renderer.EndRenderPass(RenderBuffer);
    vkEndCommandBuffer(*RenderBuffer);

    Renderer.GraphicsDispatch.SubmitRenderBuffer(RenderBuffer, &Renderer.RenderDone[0], &Renderer.RenderReady[0], 1, &Renderer.RenderSubmit[0], 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    Renderer.Present();
  }

  std::cout << "Success\n";
}

