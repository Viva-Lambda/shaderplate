#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec2 aTangent;

uniform mat4 model;

out vec3 FragPosWS; // output just positions for now
out vec2 TexCoords; // output just positions for now
out vec3 Normal;    // output just positions for now
out vec3 TangentWS; // output just positions for now

void main() {
  FragPosWS = vec3(model * vec4(aPos, 1.0));
  Normal = vec3(model * vec4(aNormal, 1.0));
  TangentWS = vec3(model * vec4(aTangent, 1.0));
  TexCoords = aTexCoord;
  gl_Position = FragPosWS;
}
