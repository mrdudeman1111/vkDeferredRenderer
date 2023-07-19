#include <Mesh.h>

#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <cstring>
#include <glm/fwd.hpp>

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

    for(uint32_t x = 0; i < pMesh->mFaces[i].mNumIndices; i++)
    {
      newMesh.Indices.push_back(pMesh->mFaces[i].mIndices[x]);
    }
  }

  if(pMesh->HasBones())
  {
    aiNode* Begin = FindNode(pScene->mRootNode, pMesh->mBones[0]->mName.C_Str());

    newMesh.Armature.RootBone.ParseNode(Begin);
  }
}

void Bone::ParseNode(aiNode* pNode)
{
  MatConvert(pNode->mTransformation, &Transform);

  for(uint32_t i = 0; i < pNode->mNumChildren; i++)
  {
    Children.push_back({});
    Children[i].Parent = this;
    Children[i].ParseNode(pNode->mChildren[i]);
  }
}

void MeshComponent::LoadMesh(const char* ModelPath)
{
  Assimp::Importer Importer;
  const aiScene* Scene = Importer.ReadFile(ModelPath, aiProcess_Triangulate);

  ParseNode(Scene, Scene->mRootNode);
}

