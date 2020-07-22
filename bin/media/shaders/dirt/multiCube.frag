#version 430
layout(location = 0) out vec4 cubeFaceView;

in vec2 TexCoord;
in vec3 Normal;
in vec4 FragPosVS;

void main() { cubeFaceView = FragPosVS; }
