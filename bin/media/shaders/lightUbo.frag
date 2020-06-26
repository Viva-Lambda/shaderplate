#version 430 core

out vec4 FragColor;

layout(std140, binding = 1) uniform Light {
  vec4 lightColor;
  float coeff;
};

void main() { FragColor = lightColor * coeff; }
