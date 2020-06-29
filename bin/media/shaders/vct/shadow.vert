#version 430

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTan;
layout(location = 4) in vec3 aBiTan;

uniform mat4 model;

uniform mat4 lightSpaceMat;

void main() { gl_Position = lightSpaceMat * model * vec4(aPos, 1); }
