#version 460

layout(std140, set = 0, binding = 0) uniform CameraBuffer
{
  mat4 Model;
  mat4 View;
  mat4 Proj;
} MVP;

layout(std140, set = 1, binding = 1) uniform MeshBuffer
{
  mat4[100] Skeletal;
} Mb;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inCoord;

layout(location = 3) in ivec4 BoneIndices;
layout(location = 4) in vec4 BoneWeights;

// if we used the other buffers in the G-Buffer we would use layout(location = AttachmentIDX) out AttachmentFRMT Name
// but only in the fragment, we can write here, but then primitive assembly would fill in the faces and stuff.
layout(location = 0) out vec3 outColor;

void main()
{
  vec4 Column0 = vec4(1.0, 0.0, 0.0, 0.0);
  vec4 Column1 = vec4(0.0, 1.0, 0.0, 0.0);
  vec4 Column2 = vec4(0.0, 0.0, 1.0, 0.0);
  vec4 Column3 = vec4(0.0, 0.0, 0.0, 1.0);

  mat4 BoneTransform = mat4(Column0, Column1, Column2, Column3);

  if(BoneIndices[0] != -1)
  {
    BoneTransform += Mb.Skeletal[BoneIndices[0]] * BoneWeights[0];
  }

  if(BoneIndices[1] != -1)
  {
    BoneTransform += Mb.Skeletal[BoneIndices[1]] * BoneWeights[1];
  }

  if(BoneIndices[2] != -1)
  {
    BoneTransform += Mb.Skeletal[BoneIndices[2]] * BoneWeights[2];
  }

  if(BoneIndices[3] != -1)
  {
    BoneTransform += Mb.Skeletal[BoneIndices[3]] * BoneWeights[3];
  }


  gl_Position = MVP.Proj * MVP.View * MVP.Model * BoneTransform * vec4(inPos, 1.f);
  outColor = vec3(1.f, 1.f, 1.f);
}
