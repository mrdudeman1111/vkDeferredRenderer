#version 460

layout(binding = 0) uniform CameraBuffer
{
  mat4 Model;
  mat4 View;
  mat4 Proj;
} MVP;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec2 inCoord;

layout(location = 0) out vec3 outColor;

void main()
{
  gl_Position = MVP.Proj * MVP.View * MVP.Model * vec4(inPos, 1.f);
  outColor = vec3(1.f, 1.f, 1.f);
}
