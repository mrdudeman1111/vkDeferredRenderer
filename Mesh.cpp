#include <Mesh.h>
#include <Renderer.h>

#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cassert>
#include <cstring>

#include <glm/fwd.hpp>
#include <stdexcept>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>

void MatConvert(aiMatrix4x4 aiMat, glm::mat4* OutMatrix)
{
  for(uint32_t i = 0; i < 4; i++)
  {
    (*OutMatrix)[i][0] = aiMat[0][i];
    (*OutMatrix)[i][1] = aiMat[1][i];
    (*OutMatrix)[i][2] = aiMat[2][i];
    (*OutMatrix)[i][3] = aiMat[3][i];
  }
}

aiNode* FindNode(aiNode* pNode, const char* NodeName)
{
  if(strcmp(pNode->mName.C_Str(), NodeName) == 0)
  {
    return pNode;
  }
  else
  {
    for(uint32_t i = 0; i < pNode->mNumChildren; i++)
    {
      if(FindNode(pNode->mChildren[i], NodeName) != nullptr)
      {
        return pNode->mChildren[i];
      }
    }
  }
  return nullptr;
}

void MeshComponent::ParseNode(const aiScene* pScene, aiNode* pNode)
{
  if(pNode->mNumChildren > 0)
  {
    for(uint32_t i = 0; i < pNode->mNumChildren; i++)
    {
      ParseNode(pScene, pNode->mChildren[i]);
    }
  }

  if(pNode->mNumMeshes > 0)
  {
    for(uint32_t i = 0; i < pNode->mNumMeshes; i++)
    {
      // the aiNode::pMeshes is an array of indices, you need to use these indices to access the mesh array contained in the scene structure.
      // this helps save on memory and resources.
      ParseMesh(pScene, pScene->mMeshes[pNode->mMeshes[i]]);
    }
  }
}

void MeshComponent::ParseMesh(const aiScene* pScene, aiMesh* pMesh)
{
  Mesh newMesh;

  for(uint32_t i = 0; i < pMesh->mNumVertices; i++)
  {
    Vertex v;
    v.Position.x = pMesh->mVertices[i].x;
    v.Position.y = pMesh->mVertices[i].y;
    v.Position.z = pMesh->mVertices[i].z;

    v.Normal.x = pMesh->mNormals[i].x;
    v.Normal.y = pMesh->mNormals[i].y;
    v.Normal.z = pMesh->mNormals[i].z;

    newMesh.Vertices.push_back(v);
  }

  for(uint32_t i = 0; i < pMesh->mNumFaces; i++)
  {
    assert(pMesh->mFaces[i].mNumIndices == 3);

    for(uint32_t x = 0; x < pMesh->mFaces[i].mNumIndices; x++)
    {
      newMesh.Indices.push_back(pMesh->mFaces[i].mIndices[x]);
    }
  }

  if(pMesh->HasBones())
  {
    aiNode* Begin = FindNode(pScene->mRootNode, pMesh->mBones[0]->mName.C_Str());

    newMesh.Armature.RootBone.AsignNode(Begin);

    for(uint32_t i = 0; i < pMesh->mNumBones; i++)
    {
      // this will store the offset matrix in our bones now that we've setup the hierarchy
      Bone* Bone = newMesh.Armature.FindBone(pMesh->mBones[i]->mName.C_Str());
      Bone->BoneIdx = i;
      MatConvert(pMesh->mBones[i]->mOffsetMatrix, &Bone->Transform);
    }
  }

  Meshes.push_back(newMesh);
}

void Bone::AsignNode(aiNode* pNode)
{
  //MatConvert(pNode->mTransformation, &Transform);

  Name = pNode->mName.C_Str();

  for(uint32_t i = 0; i < pNode->mNumChildren; i++)
  {
    Children.push_back({});
    Children[i].Parent = this;
    Children[i].AsignNode(pNode->mChildren[i]);
  }
}

void Bone::PackBuffer(glm::mat4* BufferMemory)
{
  BufferMemory[BoneIdx] = GetTransform();
}

glm::mat4 Bone::GetTransform()
{
  // if Parent is nullptr, return Transform, else return The Transform relative to the parent. This places the transform in global space.
  return (Parent == nullptr) ? Transform : Parent->GetTransform() * Transform;
}

Bone* Bone::FindBone(const char* pName)
{
  if(strcmp(pName, Name.c_str()) == 0)
  {
    return this;
  }
  for(uint32_t i = 0; i < Children.size(); i++)
  {
    if(Children[i].FindBone(pName) != nullptr)
    {
      return &Children[i];
    }
  }
  return nullptr;
}

Bone* Skeleton::FindBone(const char* pName)
{
  return RootBone.FindBone(pName);
}

void Skeleton::PackBuffer(glm::mat4* BufferMemory)
{
  RootBone.PackBuffer(BufferMemory);
}

void Mesh::ZeroDescriptor(glm::mat4* Buffer)
{
  for(uint32_t i = 0; i < 100; i++)
  {
    Buffer[i] = glm::mat4(1.f);
  }
}

void Mesh::UpdateSkeleton(Renderer* pRenderer)
{
  if(pPipeline->Mat.HasSkeleton)
  {
    if(sBuff.Allocation.AllocSize == -1)
    {
      pRenderer->CreateBuffer(&sBuff, sizeof(glm::mat4) * 100, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, eBufferMemory);
    }

    Buffer Transfer;
    if(pRenderer->HasTransfer())
    {
      pRenderer->CreateBuffer(&Transfer, sizeof(glm::mat4) * 100, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, eTransferMemory);
    }
    else
    {
      pRenderer->CreateBuffer(&Transfer, sizeof(glm::mat4) * 100, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, eHostMemory);
    }

    void* TransferMemory;
    vkMapMemory(pRenderer->Device, *Transfer.Allocation.Memory, Transfer.Allocation.Offset, Transfer.Allocation.AllocSize, 0, &TransferMemory);
      ZeroDescriptor((glm::mat4*)TransferMemory);
      Armature.PackBuffer((glm::mat4*)TransferMemory);
    vkUnmapMemory(pRenderer->Device, *Transfer.Allocation.Memory);

    VkBufferCopy Copy{};
    Copy.size = sizeof(glm::mat4) * 100;
    // these offsets are relative to the start of the actual buffer, not the allocation.
    Copy.srcOffset = 0; 
    Copy.dstOffset = 0;

    pRenderer->CopyToBuffer(&Transfer, &sBuff, Copy);

    VkDescriptorBufferInfo BufferWrite{};
    BufferWrite.buffer = sBuff.Buffer;
    BufferWrite.offset = 0; // sBuff.Allocation.Offset; it would be the allocation offset if the offset parameter was relative to the allocation, but it's not, it's relative to the start of the buffer
    BufferWrite.range = sBuff.Allocation.AllocSize;

    VkWriteDescriptorSet Write{};
    Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Write.dstSet = MaterialSet;
    Write.dstBinding = 1;
    Write.pBufferInfo = &BufferWrite;
    Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Write.dstArrayElement = 0;
    Write.descriptorCount = 1;

    vkUpdateDescriptorSets(pRenderer->Device, 1, &Write, 0, nullptr);
  }
}

void Mesh::Update(Renderer* pRenderer)
{
  if(vBuff.Allocation.AllocSize == -1)
  {
    pRenderer->CreateBuffer(&vBuff, sizeof(Vertex) * Vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, eHostMemory);
  }
  if(iBuff.Allocation.AllocSize == -1)
  {
    pRenderer->CreateBuffer(&iBuff, sizeof(uint32_t) * Indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, eHostMemory);
  }

  void* VertexMemory;
  vkMapMemory(pRenderer->Device, *vBuff.Allocation.Memory, vBuff.Allocation.Offset, vBuff.Allocation.AllocSize, 0, &VertexMemory);
    memcpy(VertexMemory, Vertices.data(), vBuff.Allocation.AllocSize);
  vkUnmapMemory(pRenderer->Device, *vBuff.Allocation.Memory);

  void* IndexMemory;
  vkMapMemory(pRenderer->Device, *iBuff.Allocation.Memory, iBuff.Allocation.Offset, iBuff.Allocation.AllocSize, 0, &IndexMemory);
    memcpy(IndexMemory, Indices.data(), iBuff.Allocation.AllocSize);
  vkUnmapMemory(pRenderer->Device, *iBuff.Allocation.Memory);
}

void Mesh::Draw(VkCommandBuffer* Buffer, VkDescriptorSet CameraDescriptor)
{
  vkCmdBindPipeline(*Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->Pipe);

  VkDeviceSize Offset = 0;  

  vkCmdBindVertexBuffers(*Buffer, 0, 1, &vBuff.Buffer, &Offset);
  vkCmdBindIndexBuffer(*Buffer, iBuff.Buffer, Offset, VK_INDEX_TYPE_UINT32);

  vkCmdBindDescriptorSets(*Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->PipeLayout, 1, 1, &MaterialSet, 0, 0);
  vkCmdBindDescriptorSets(*Buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pPipeline->PipeLayout, 0, 1, &CameraDescriptor, 0, 0);

  vkCmdDrawIndexed(*Buffer, Indices.size(), 1, 0, 0, 0);
}

void MeshComponent::LoadMesh(Renderer* pRender, Pipeline* pPipeline, const char* ModelPath)
{
  VkResult Res;

  pRenderer = pRender;

  Assimp::Importer Importer;
  const aiScene* Scene = Importer.ReadFile(ModelPath, aiProcess_Triangulate);

  if(Scene == nullptr)
  {
    throw std::runtime_error("Failed to load mesh");
  }

  ParseNode(Scene, Scene->mRootNode);

  for(uint32_t i = 0; i < Meshes.size(); i++)
  {
    Meshes[i].MaterialSet = pPipeline->CreateDescriptor();
    Meshes[i].pPipeline = pPipeline;
  }
}

void MeshComponent::Update()
{
  for(uint32_t i = 0; i < Meshes.size(); i++)
  {
    Meshes[i].Update(pRenderer);
  }
  UpdateSkeletons();
}

void MeshComponent::UpdateSkeletons()
{
  for(uint32_t i = 0; i < Meshes.size(); i++)
  {
    Meshes[i].UpdateSkeleton(pRenderer);
  }
}

void MeshComponent::Draw(VkCommandBuffer* Buffer)
{
  VkDescriptorSet CamDesc = pRenderer->GetCameraDescriptor();
  for(uint32_t i = 0; i < Meshes.size(); i++)
  {
    Meshes[i].Draw(Buffer, CamDesc);
  }
}

