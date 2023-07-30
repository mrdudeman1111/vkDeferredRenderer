#pragma once

#include <Pipeline.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <vulkan/vulkan_core.h>

class MeshComponent;
class Renderer;

struct Header
{
public:
  uint32_t AllocSize = -1;
  uint32_t Offset = -1;

  VkDeviceMemory* Memory;
  uint8_t MemoryFlags;
};

struct Texture
{
public:
  VkImage Image;
  VkFormat Format;
  VkImageLayout Layout;

  Header Allocation;
};

struct Buffer
{
public:
  VkBuffer Buffer;

  Header Allocation;
};

struct VertexWeight
{
  public:
    // Index of the affected vertex
    uint32_t VertexId;
    // normalized value of vertex weight
    float Weight;
};

class Bone
{
  public:
    void AsignNode(aiNode* pNode);
    Bone* FindBone(const char* pName);
    void PackBuffer(float* BufferMemory);

    glm::mat4 GetTransform();

    std::vector<Bone> Children;

    std::string Name;

    Bone* Parent = nullptr;

    glm::mat4 Transform;

    uint32_t BoneIdx = -1;
};

class Skeleton
{
  public:
    Bone RootBone;
    Bone* FindBone(const char* pName);
    void PackBuffer(float* BufferMemory);
    void Cleanup();
};

class Mesh
{
    friend MeshComponent;
  public:
    void Update(Renderer* pRenderer);
    void Draw(VkCommandBuffer* Buffer, VkDescriptorSet CameraDescriptor);

    glm::mat4 Transform;

    VkDescriptorSet MaterialSet;

    Skeleton Armature;

    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;

    Pipeline* pPipeline;
  private:
    void UpdateSkeleton(Renderer* pRenderer);
    void ZeroDescriptor(glm::mat4* Buffer);

    Buffer vBuff;
    Buffer iBuff;
    Buffer sBuff;
};

class MeshComponent
{
  public:
    void LoadMesh(Renderer* pRender, Pipeline* pPipeline, const char* ModelPath);
    void Update();
    void UpdateSkeletons();
    void Draw(VkCommandBuffer* Buffer);

    Pipeline* pPipeline;

  private:
    void ParseNode(const aiScene* pScene, aiNode* pNode);
    void ParseMesh(const aiScene* pScene, aiMesh* pMesh);
    void ParseBone(Bone* pBone, aiNode* pNode);

    Renderer* pRenderer;

    std::vector<Mesh> Meshes;
};


