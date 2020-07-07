#version 330 core

out vec4 FragColor;

uniform vec3 lightColor;
in mat3 TBN;
in vec3 FragPosInView;
in vec2 TexCoord;

void main() { FragColor = vec4(lightColor, 1.0); }
