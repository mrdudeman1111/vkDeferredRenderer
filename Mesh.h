#pragma once

#include <Pipeline.h>

#include <assimp/BaseImporter.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/ParsingUtils.h>
#include <vulkan/vulkan_core.h>

struct Header
{
public:
  uint32_t AllocSize;
  uint32_t Offset;
};

struct Texture
{
public:
  VkImage Image;

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
    void ParseNode(aiNode* pNode);

    glm::mat4 GetTransform();

    std::vector<Bone> Children;

    std::string Name;

    Bone* Parent;

    glm::mat4 Transform;
};

class Skeleton
{
  public:
    Bone RootBone;
};

class Mesh
{
  public:
    void UpdateSkeleton();

    glm::mat4 Transform;

    VkDescriptorSet MaterialSet;
    VkDescriptorSet SkeletonSet;

    Skeleton Armature;

    std::vector<Vertex> Vertices;
    std::vector<uint32_t> Indices;
  private:

    Buffer vBuff;
    Buffer iBuff;
};

class MeshComponent
{
  public:
    void LoadMesh(const char* ModelPath);

  private:
    void ParseNode(const aiScene* pScene, aiNode* pNode);
    void ParseMesh(const aiScene* pScene, aiMesh* pMesh);
    void ParseBone(Bone* pBone, aiNode* pNode);

    std::vector<Mesh> Meshes;

    Pipeline* pPipeline;
};


