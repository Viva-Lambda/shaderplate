#version 430
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

out vec3 FragPosVS;
out vec4 FragPosCS;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

void main() {
  FragPos = vec3(model * vec4(aPos, 1.0));
  FragPosVS = vec3(view * vec4(FragPos, 1.0));
  vec4 f_cs = projection * vec4(FragPosVS, 1.0);
  f_cs /= f_cs.w;
  FragPosCS = f_cs;
  Normal = vec3(model * vec4(aNormal, 1.0));
  TexCoord = aTexCoord;

  // classic gl pos
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
