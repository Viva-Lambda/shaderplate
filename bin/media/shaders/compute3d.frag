#version 430 core
out vec4 FragColor;
in vec3 TexCoord;

uniform sampler3D myTexture;

void main() { FragColor = vec4(texture(myTexture, TexCoord)); }
