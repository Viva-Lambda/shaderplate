#version 430 core

out vec4 FragColor;

layout(std430, binding = 1) readonly buffer Light {
  vec4 lightColor;
  float coeff[3];
};

void main() { FragColor = lightColor * coeff[1]; }
