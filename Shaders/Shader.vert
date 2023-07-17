#version 440

layout(binding = 0) uniform SceneBuffer
{
  mat4 Model;
  mat4 View;
  mat4 Proj;
} MVP;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inCoord;

void main()
{

}
