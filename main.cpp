#include "Mesh.h"
#include "Pipeline.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>
#include <Renderer.h>
#include <vulkan/vulkan_core.h>


// idea: seperate memory blocks, so have "slots" for different locations where textures and buffers can be stored within memory blocks.
/*
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

  Renderer.CreateSwapchain(VK_PRESENT_MODE_IMMEDIATE_KHR);
  Renderer.CreateRenderPass();
  Renderer.CreateFrameBuffers();

  Material Mat = Renderer.CreateMat();

  Mat.HasSkeleton = true;

  Shader Vert = Renderer.CreateShader(ShaderDir"3Dvert.spv", "main");
  Shader Frag = Renderer.CreateShader(ShaderDir"3Dfrag.spv", "main");

  Mat.VertShader = &Vert;
  Mat.FragShader = &Frag;

  VkDescriptorSetLayoutBinding SkeletalBinding{};
  SkeletalBinding.binding = 1;
  SkeletalBinding.descriptorCount = 1;
  SkeletalBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  SkeletalBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  Mat.AddResource(SkeletalBinding);

  Pipeline MainPipe = Renderer.CreatePipeline(Mat, e3D, 1280, 720, 0, 0);

  MeshComponent Cube;
  Cube.LoadMesh(&Renderer, &MainPipe, ModelDir"Cube.dae");
  Cube.pPipeline = &MainPipe;

  VkCommandBuffer* RenderBuffer = Renderer.GraphicsDispatch.GetRenderBuffer(&Renderer.Device);

  Camera* Cam = Renderer.GetCamera();

  VkRect2D RenderArea{};
  RenderArea.extent = {1280, 720};
  RenderArea.offset = {0, 0};

  while(!Renderer.ShouldClose())
  {
    VkCommandBufferBeginInfo BeginInf{};
    BeginInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    Renderer.WaitOnLastFrame();
    float DeltaTime = Renderer.GetDeltaTime();

    vkResetCommandBuffer(*RenderBuffer, 0);

    Cam->Update(DeltaTime);

    Cube.Update();

    // idea: Have the renderer handle the recording of render buffer, then have user submit meshes, etc
    vkBeginCommandBuffer(*RenderBuffer, &BeginInf);
      Renderer.BeginRenderPass(RenderBuffer, RenderArea);
        Cube.Draw(RenderBuffer);
      Renderer.EndRenderPass(RenderBuffer);
    vkEndCommandBuffer(*RenderBuffer);

    Renderer.GraphicsDispatch.SubmitRenderBuffer(RenderBuffer, &Renderer.RenderDone[0], &Renderer.RenderReady[0], 1, &Renderer.RenderSubmit[0], 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    Renderer.Present();
  }

  std::cout << "Success\n";
}
*/

int main()
{
  Renderer Render;

  Render.RequestInstLayer("VK_LAYER_KHRONOS_validation");

  Render.Init(1280, 720);

  Render.RequestDevExt(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  Render.CreateDevice();

  Render.CreateSwapchain(VK_PRESENT_MODE_MAILBOX_KHR);

  EkFrameBufferAttachment DepthAttachment;
  DepthAttachment.IMG_Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
  DepthAttachment.IMG_ViewType = VK_IMAGE_VIEW_TYPE_2D;
  DepthAttachment.IMG_Format = VK_FORMAT_D16_UNORM;
  DepthAttachment.IMG_Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  DepthAttachment.FB_Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  DepthAttachment.FB_Description.format = VK_FORMAT_D16_UNORM;
  DepthAttachment.FB_Description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  DepthAttachment.FB_Description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  DepthAttachment.FB_Description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  DepthAttachment.FB_Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
  DepthAttachment.FB_Description.samples = VK_SAMPLE_COUNT_1_BIT;
  DepthAttachment.FB_Description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  DepthAttachment.FB_Description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  DepthAttachment.FB_Description.flags = 0;

  DepthAttachment.Location = 3;

  EkFrameBufferAttachment NormAttachment;
  NormAttachment.IMG_Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
  NormAttachment.IMG_ViewType = VK_IMAGE_VIEW_TYPE_2D;
  NormAttachment.IMG_Format = VK_FORMAT_R8G8B8A8_UNORM;
  NormAttachment.IMG_Usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  NormAttachment.FB_Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  NormAttachment.FB_Description.format = VK_FORMAT_R8G8B8A8_UNORM;
  NormAttachment.FB_Description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  NormAttachment.FB_Description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  NormAttachment.FB_Description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  NormAttachment.FB_Description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  NormAttachment.FB_Description.samples = VK_SAMPLE_COUNT_1_BIT;
  NormAttachment.FB_Description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  NormAttachment.FB_Description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  NormAttachment.FB_Description.flags = 0;

  NormAttachment.Location = 2;

  Render.AddFrameBufferAttachment(&DepthAttachment, eDepth);
  Render.AddFrameBufferAttachment(&NormAttachment, eColor);

  // Renderpass
    EkSubpass Subpasses[2];

    Subpasses[0].BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpasses[0].AddAttachment(2, eColor);
    Subpasses[0].AddAttachment(3, eDepth);

    Subpasses[1].BindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpasses[1].AddAttachment(1, eColor); // Second attachment is the UI Color attachment, the first is the Swapchain Image;


    Render.CreateRenderPass(Subpasses, 2);
  // Renderpass

  Render.CreateFrameBuffers();

  Render.CreateUIPane();

  Material pMaterial = Render.CreateMat();

  Shader vShader = Render.CreateShader(ShaderDir"3Dvert.spv", "main");
  Shader fShader = Render.CreateShader(ShaderDir"3Dfrag.spv", "main");

  VkDescriptorSetLayoutBinding CameraBinding{};
  CameraBinding.binding = 0;
  CameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  CameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  CameraBinding.descriptorCount = 1;

  pMaterial.AddResource(CameraBinding);

  pMaterial.VertShader = &vShader;
  pMaterial.FragShader = &fShader;
  pMaterial.HasSkeleton = false;

  Pipeline Pipe3D = Render.CreatePipeline(pMaterial, e3D, 1280, 720, 0, 0);

  MeshComponent Cube;
  Cube.LoadMesh(&Render, &Pipe3D, ModelDir"Cube.dae");

  VkCommandBuffer* RenderBuffer = Render.GraphicsDispatch.GetRenderBuffer(&Render.Device);

  Camera* PrimaryCam = Render.GetCamera();
  float DeltaTime;

  VkRect2D RenderArea;
  RenderArea.extent = {1280, 720};
  RenderArea.offset = {0, 0};

  VkDeviceSize Offset = 0;

  while(!Render.ShouldClose())
  {
    VkCommandBufferBeginInfo BeginInf{};
    BeginInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    glfwPollEvents();

    Render.WaitOnLastFrame();
    DeltaTime = Render.GetDeltaTime();

    PrimaryCam->Update(DeltaTime);

    Cube.Update();

    vkResetCommandBuffer(*RenderBuffer, 0);

    // idea: Have the renderer handle the recording of render buffer, then have user submit meshes, etc
    vkBeginCommandBuffer(*RenderBuffer, &BeginInf);
      Render.BeginRenderPass(RenderBuffer, RenderArea);

      Render.EndRenderPass(RenderBuffer);
    vkEndCommandBuffer(*RenderBuffer);

    VkFence Fences[2] = {Render.RenderDone[0], Render.RenderDone[1]};

    Render.GraphicsDispatch.SubmitRenderBuffer(RenderBuffer, Fences, &Render.RenderReady[0], 1, &Render.RenderSubmit[0], 1, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    Render.Present();
  }
}

