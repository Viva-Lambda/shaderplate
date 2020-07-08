#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

out vec3 FragPosVS;
out vec2 TexCoord;
out vec3 Normal;

void main() {
  FragPosVS = vec3(view * model * vec4(aPos, 1.0));
  Normal = vec3(model * vec4(aNormal, 1.0));
  TexCoord = aTexCoord;

  // classic gl pos
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
