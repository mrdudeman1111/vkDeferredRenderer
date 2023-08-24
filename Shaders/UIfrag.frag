#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNorm;

void main()
{
  outColor = vec4(1.f, 1.f, 1.f, 1.f);
  outNorm = vec4(0.f, 0.f, 0.f, 0.f);
}

